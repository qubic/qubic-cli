#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "structs.h"
#include "connection.h"
#include "wallet_utils.h"
#include "logger.h"
#include "key_utils.h"
#include "contracts.h"
#include "asset_utils.h"
#include "qrwa.h"
#include "k12_and_key_utils.h"

// ─── Helpers ───────────────────────────────────────────────────

static const char* payoutTypeName(uint8_t t)
{
    switch (t)
    {
        case QRWA_PAYOUT_TYPE_QMINE_HOLDER:   return "QMINE-Holder";
        case QRWA_PAYOUT_TYPE_QMINE_DEV:      return "QMINE-Dev";
        case QRWA_PAYOUT_TYPE_QRWA_HOLDER:    return "qRWA-Holder";
        case QRWA_PAYOUT_TYPE_DEDICATED_QRWA: return "Dedicated-qRWA";
        case QRWA_PAYOUT_TYPE_POOL_D_QRWA:    return "PoolD-qRWA";
        default:                               return "Unknown";
    }
}

static bool isZeroKey(const uint8_t key[32])
{
    for (int i = 0; i < 32; i++)
        if (key[i] != 0) return false;
    return true;
}

static void printIdentity(const char* label, const uint8_t pubkey[32])
{
    char id[128] = {};
    getIdentityFromPublicKey(pubkey, id, false);
    LOG("%s%s\n", label, id);
}

// Compute a deterministic payout hash from the entry data.
// Hash = K12(recipient[32] || amount[8] || tick[4] || epoch[2] || payoutType[1] || pool[1])
// This mirrors what the contract would compute with qpi.K12().
static void computePayoutHash(const QRWAPayoutEntry& e, uint8_t pool, char* hashOut)
{
    uint8_t buf[48]; // 32 + 8 + 4 + 2 + 1 + 1 = 48
    memcpy(buf,      e.recipient,  32);
    memcpy(buf + 32, &e.amount,     8);
    memcpy(buf + 40, &e.tick,       4);
    memcpy(buf + 44, &e.epoch,      2);
    buf[46] = e.payoutType;
    buf[47] = pool;

    uint8_t digest[32] = {};
    KangarooTwelve(buf, sizeof(buf), digest, 32);
    getTxHashFromDigest(digest, hashOut);
}

// Print paginated payout results (entries already ordered newest-first by contract).
// epochFilter: -1 = show all, >=0 = only show entries matching that epoch
template <typename OutputT>
static uint32_t printPayoutPage(const OutputT& result, const char* poolLabel, int epochFilter)
{
    const auto& payouts = result.payouts;
    uint16_t count = result.returnedCount;

    // If no raw entries on this page, nothing to show
    bool hasAnyEntry = false;
    for (uint16_t i = 0; i < count; i++)
    {
        if (payouts[i].amount != 0 || !isZeroKey(payouts[i].recipient))
        { hasAnyEntry = true; break; }
    }
    if (!hasAnyEntry)
        return 0;

    uint64_t totalAmount = 0;
    uint32_t entryCount = 0;
    uint64_t typeTotal[5] = {};
    uint32_t typeCount[5] = {};

    for (uint16_t i = 0; i < count; i++)
    {
        if (payouts[i].amount == 0 && isZeroKey(payouts[i].recipient))
            continue;
        if (epochFilter >= 0 && payouts[i].epoch != (uint16_t)epochFilter)
            continue;
        entryCount++;
        totalAmount += payouts[i].amount;
        uint8_t t = payouts[i].payoutType;
        if (t < 5)
        {
            typeTotal[t] += payouts[i].amount;
            typeCount[t]++;
        }
    }

    if (entryCount == 0)
        return 0;

    if (epochFilter >= 0)
        LOG("\n═══ %s Payouts — Epoch %d (page %u/%u) ═══════════════\n",
            poolLabel, epochFilter, (unsigned)result.page + 1, (unsigned)result.totalPages);
    else
        LOG("\n═══ %s Payouts (page %u/%u) ══════════════════════════\n",
            poolLabel, (unsigned)result.page + 1, (unsigned)result.totalPages);
    LOG("  Entries: %u   Total QU: %llu\n",
        entryCount, (unsigned long long)totalAmount);
    LOG("  Ring buffer: %llu slots, nextIdx=%u\n",
        (unsigned long long)QRWA_PAYOUT_RING_SIZE, (unsigned)result.nextIdx);

    for (int t = 0; t < 5; t++)
    {
        if (typeCount[t] == 0) continue;
        LOG("  Type %d (%s): %u entries, %llu QU\n",
            t, payoutTypeName(t), typeCount[t], (unsigned long long)typeTotal[t]);
    }

    LOG("\n  Payouts (newest first):\n");
    LOG("  %-60s %16s %12s %12s %10s %6s %s\n", "Recipient", "Amount", "QMINE", "qRWA", "Tick", "Epoch", "Type");
    LOG("  %-60s %16s %12s %12s %10s %6s %s\n",
        "------------------------------------------------------------",
        "----------------", "------------", "------------", "----------", "------", "----------------");

    for (uint16_t i = 0; i < count; i++)
    {
        const auto& e = payouts[i];
        if (e.amount == 0 && isZeroKey(e.recipient))
            continue;
        if (epochFilter >= 0 && e.epoch != (uint16_t)epochFilter)
            continue;

        char identity[128] = {};
        getIdentityFromPublicKey(e.recipient, identity, false);

        LOG("  %-60s %16llu %12llu %12llu %10u %6u %s\n",
            identity,
            (unsigned long long)e.amount,
            (unsigned long long)e.qmineHolding,
            (unsigned long long)e.qrwaHolding,
            e.tick,
            (unsigned)e.epoch,
            payoutTypeName(e.payoutType));
    }

    return entryCount;
}

// ─── qrwaPayout — one command per pool ─────────────────────────

void qrwaPayout(const char* nodeIp, int nodePort, QRWAPoolType pool, int epochFilter)
{
    // Always show totals first
    QRWAGetTotalDistributed_output totals = {};
    if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_TOTAL_DISTRIBUTED, nullptr, 0, &totals, sizeof(totals)))
    {
        LOG("totalPoolADistributed (Qubic Mining): %llu QU\n", (unsigned long long)totals.totalPoolADistributed);
        LOG("totalPoolBDistributed (SC Assets):     %llu QU\n", (unsigned long long)totals.totalPoolBDistributed);
        LOG("totalPoolCDistributed (BTC Mining):    %llu QU\n", (unsigned long long)totals.totalPoolCDistributed);
        LOG("totalPoolDDistributed (MLM Water):     %llu QU\n", (unsigned long long)totals.totalPoolDDistributed);
    }
    else
    {
        LOG("ERROR: Could not query GetTotalDistributed (fn 6)\n");
    }

    if (epochFilter >= 0)
        LOG("Filtering by epoch: %d\n", epochFilter);

    // Each pool command shows exactly one ring buffer:
    //   pool_a → fn 11 (Pool A payouts, after gov fees)
    //   pool_b → fn 13 (Pool B payouts, no gov fees)
    //   pool_c → fn 14 (Pool C payouts, dedicated)
    struct RingQuery { uint8_t fnId; const char* label; };
    RingQuery queries[1] = {};

    switch (pool)
    {
        case QRWA_POOL_A:
            queries[0] = { QRWA_GET_PAYOUTS_QMINE, "Pool A — Qubic Mining (fn 11)" };
            break;
        case QRWA_POOL_B:
            queries[0] = { QRWA_GET_PAYOUTS_QRWA,  "Pool B — SC Assets Revenue (fn 13)" };
            break;
        case QRWA_POOL_C:
            queries[0] = { QRWA_GET_PAYOUTS_DEDICATED, "Pool C — BTC Mining (fn 14)" };
            break;
        case QRWA_POOL_D:
            queries[0] = { QRWA_GET_PAYOUTS_POOL_D, "Pool D — MLM Water (fn 16)" };
            break;
    }

    for (uint16_t page = 0; ; page++)
    {
        QRWAGetPayouts_input pageInput = {};
        pageInput.page = page;

        bool ok = false;
        uint32_t found = 0;

        if (queries[0].fnId == QRWA_GET_PAYOUTS_QMINE)
        {
            QRWAGetPayoutsQmine_output result = {};
            ok = runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                    queries[0].fnId, &pageInput, sizeof(pageInput), &result, sizeof(result));
            if (ok)
            {
                found = printPayoutPage(result, queries[0].label, epochFilter);
                if (page + 1 >= result.totalPages) break;
            }
        }
        else if (queries[0].fnId == QRWA_GET_PAYOUTS_QRWA)
        {
            QRWAGetPayoutsQrwa_output result = {};
            ok = runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                    queries[0].fnId, &pageInput, sizeof(pageInput), &result, sizeof(result));
            if (ok)
            {
                found = printPayoutPage(result, queries[0].label, epochFilter);
                if (page + 1 >= result.totalPages) break;
            }
        }
        else if (queries[0].fnId == QRWA_GET_PAYOUTS_DEDICATED)
        {
            QRWAGetPayoutsDedicated_output result = {};
            ok = runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                    queries[0].fnId, &pageInput, sizeof(pageInput), &result, sizeof(result));
            if (ok)
            {
                found = printPayoutPage(result, queries[0].label, epochFilter);
                if (page + 1 >= result.totalPages) break;
            }
        }
        else if (queries[0].fnId == QRWA_GET_PAYOUTS_POOL_D)
        {
            QRWAGetPayoutsPoolD_output result = {};
            ok = runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                    queries[0].fnId, &pageInput, sizeof(pageInput), &result, sizeof(result));
            if (ok)
            {
                found = printPayoutPage(result, queries[0].label, epochFilter);
                if (page + 1 >= result.totalPages) break;
            }
        }

        if (!ok)
        {
            LOG("  (fn %u: keine Eintraege / Ring-Buffer leer)\n", (unsigned)queries[0].fnId);
            break;
        }
        if (epochFilter < 0) break;       // no filter: first page only
        if (found == 0) break;             // no more matching entries
    }

    // For Pool B: show SC Dividend Tracker (fn 15) — revenue sources
    if (pool == QRWA_POOL_B)
    {
        QRWAGetScDividendTracking_output scTracker = {};
        if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                QRWA_GET_SC_DIVIDEND_TRACKING, nullptr, 0, &scTracker, sizeof(scTracker))
            && scTracker.count > 0)
        {
            LOG("\n═══ SC Dividend Sources (fn 15) — Revenue an Pool B ═══════════════\n");
            LOG("  Tracked SC assets: %llu\n\n", (unsigned long long)scTracker.count);
            LOG("  %-60s %8s %20s\n", "SC Contract", "Name", "Cumulative QU");
            LOG("  %-60s %8s %20s\n",
                "------------------------------------------------------------",
                "--------", "--------------------");
            for (uint64_t i = 0; i < scTracker.count && i < QRWA_MAX_ASSETS; i++)
            {
                char scIdentity[128] = {};
                getIdentityFromPublicKey(scTracker.scContractIds[i], scIdentity, false);
                // Extract contract index from first 8 bytes of the public key
                uint64_t idx = 0;
                memcpy(&idx, scTracker.scContractIds[i], sizeof(uint64_t));
                const char* name = getContractName((uint32_t)idx);
                if (!name) name = "?";
                LOG("  %-60s %8s %20llu\n",
                    scIdentity, name,
                    (unsigned long long)scTracker.cumulativeDividends[i]);
            }
        }
    }
}

// ─── qrwaPayoutAddress — payouts for a specific address ────────

void qrwaPayoutAddress(const char* nodeIp, int nodePort, const char* identity, int epochFilter)
{
    uint8_t targetKey[32] = {};
    getPublicKeyFromIdentity(identity, targetKey);

    char verifiedIdentity[128] = {};
    getIdentityFromPublicKey(targetKey, verifiedIdentity, false);

    LOG("qRWA Payouts for: %s\n", verifiedIdentity);
    if (epochFilter >= 0)
        LOG("Filtering by epoch: %d\n", epochFilter);

    struct PoolQuery {
        uint8_t fnId;
        const char* label;
    };
    PoolQuery pools[4] = {
        { QRWA_GET_PAYOUTS_QMINE,     "Pool A (Qubic Mining)" },
        { QRWA_GET_PAYOUTS_QRWA,      "Pool B (SC Assets)" },
        { QRWA_GET_PAYOUTS_DEDICATED,  "Pool C (BTC Mining)" },
        { QRWA_GET_PAYOUTS_POOL_D,    "Pool D (MLM Water)" },
    };

    uint32_t grandTotal = 0;
    uint64_t grandAmount = 0;

    for (int p = 0; p < 4; p++)
    {
        uint32_t poolEntries = 0;
        uint64_t poolAmount = 0;
        bool headerPrinted = false;

        for (uint16_t page = 0; ; page++)
        {
            QRWAGetPayouts_input pageInput = {};
            pageInput.page = page;

            // All three output structs have identical layout
            QRWAGetPayoutsQmine_output result = {};
            bool ok = false;

            if (pools[p].fnId == QRWA_GET_PAYOUTS_QMINE)
            {
                ok = runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                        pools[p].fnId, &pageInput, sizeof(pageInput), &result, sizeof(result));
            }
            else if (pools[p].fnId == QRWA_GET_PAYOUTS_QRWA)
            {
                QRWAGetPayoutsQrwa_output r = {};
                ok = runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                        pools[p].fnId, &pageInput, sizeof(pageInput), &r, sizeof(r));
                if (ok) memcpy(&result, &r, sizeof(result));
            }
            else if (pools[p].fnId == QRWA_GET_PAYOUTS_DEDICATED)
            {
                QRWAGetPayoutsDedicated_output r = {};
                ok = runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                        pools[p].fnId, &pageInput, sizeof(pageInput), &r, sizeof(r));
                if (ok) memcpy(&result, &r, sizeof(result));
            }
            else
            {
                QRWAGetPayoutsPoolD_output r = {};
                ok = runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
                        pools[p].fnId, &pageInput, sizeof(pageInput), &r, sizeof(r));
                if (ok) memcpy(&result, &r, sizeof(result));
            }

            if (!ok) break;

            for (uint16_t i = 0; i < result.returnedCount; i++)
            {
                const auto& e = result.payouts[i];
                if (e.amount == 0 && isZeroKey(e.recipient))
                    continue;
                if (memcmp(e.recipient, targetKey, 32) != 0)
                    continue;
                if (epochFilter >= 0 && e.epoch != (uint16_t)epochFilter)
                    continue;

                if (!headerPrinted)
                {
                    LOG("\n═══ %s ═══════════════════════════════════\n", pools[p].label);
                    LOG("  %16s %12s %12s %10s %6s %-16s %s\n",
                        "Amount", "QMINE", "qRWA", "Tick", "Epoch", "Type", "PayoutHash");
                    LOG("  %16s %12s %12s %10s %6s %-16s %s\n",
                        "----------------", "------------", "------------",
                        "----------", "------", "----------------",
                        "------------------------------------------------------------");
                    headerPrinted = true;
                }

                char payoutHash[64] = {};
                computePayoutHash(e, (uint8_t)p, payoutHash);

                LOG("  %16llu %12llu %12llu %10u %6u %-16s %s\n",
                    (unsigned long long)e.amount,
                    (unsigned long long)e.qmineHolding,
                    (unsigned long long)e.qrwaHolding,
                    e.tick,
                    (unsigned)e.epoch,
                    payoutTypeName(e.payoutType),
                    payoutHash);

                poolEntries++;
                poolAmount += e.amount;
            }

            if (page + 1 >= result.totalPages) break;
        }

        if (poolEntries > 0)
        {
            LOG("  ── %u payouts, total %llu QU ──\n", poolEntries, (unsigned long long)poolAmount);
            grandTotal += poolEntries;
            grandAmount += poolAmount;
        }
    }

    if (grandTotal == 0)
        LOG("\n  (keine Payouts fuer diese Adresse gefunden)\n");
    else
        LOG("\n  GESAMT: %u payouts, %llu QU\n", grandTotal, (unsigned long long)grandAmount);
}

// ─── qrwaStatus — overview (totals + addresses) ───────────────

void qrwaStatus(const char* nodeIp, int nodePort)
{
    LOG("═══ qRWA Contract Status ═══════════════════════════════\n\n");

    // Totals
    QRWAGetTotalDistributed_output totals = {};
    if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_TOTAL_DISTRIBUTED, nullptr, 0, &totals, sizeof(totals)))
    {
        LOG("Total Distributed:\n");
        LOG("  Pool A (Qubic Mining): %llu QU\n", (unsigned long long)totals.totalPoolADistributed);
        LOG("  Pool B (SC Assets):    %llu QU\n", (unsigned long long)totals.totalPoolBDistributed);
        LOG("  Pool C (BTC Mining):   %llu QU\n", (unsigned long long)totals.totalPoolCDistributed);
        LOG("  Pool D (MLM Water):    %llu QU\n", (unsigned long long)totals.totalPoolDDistributed);
        LOG("  Payout QMINE Begin:    %llu\n", (unsigned long long)totals.payoutTotalQmineBegin);
    }
    else
    {
        LOG("ERROR: Could not query GetTotalDistributed (fn 6)\n");
    }

    // Contract addresses
    QRWAGetContractAddresses_output addrs = {};
    if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_CONTRACT_ADDRESSES, nullptr, 0, &addrs, sizeof(addrs)))
    {
        LOG("\nContract Addresses:\n");
        printIdentity("  Pool A Revenue:    ", addrs.poolARevenueAddress);
        printIdentity("  Pool C Dedicated:  ", addrs.dedicatedRevenueAddress);
        printIdentity("  Pool D Revenue:    ", addrs.poolDRevenueAddress);
        printIdentity("  Fundraising:       ", addrs.fundraisingAddress);
        printIdentity("  Exchange:          ", addrs.exchangeAddress);
    }
    else
    {
        LOG("ERROR: Could not query GetContractAddresses (fn 12)\n");
    }

    // Treasury
    QRWAGetTreasuryBalance_output treasury = {};
    if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_TREASURY_BALANCE, nullptr, 0, &treasury, sizeof(treasury)))
    {
        LOG("\nTreasury:\n");
        LOG("  QMINE Balance: %llu\n", (unsigned long long)treasury.balance);
    }

    // Dividend balances
    QRWAGetDividendBalances_output divs = {};
    if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_DIVIDEND_BALANCES, nullptr, 0, &divs, sizeof(divs)))
    {
        LOG("\nDividend Balances:\n");
        LOG("  Revenue Pool A:       %llu QU\n", (unsigned long long)divs.revenuePoolA);
        LOG("  Revenue Pool B:       %llu QU\n", (unsigned long long)divs.revenuePoolB);
        LOG("  Dedicated Revenue:    %llu QU\n", (unsigned long long)divs.dedicatedRevenuePool);
        LOG("  Pool A QMINE Div:     %llu QU\n", (unsigned long long)divs.poolAQmineDividend);
        LOG("  Pool A qRWA Div:      %llu QU\n", (unsigned long long)divs.poolAQrwaDividend);
        LOG("  Pool B QMINE Div:     %llu QU\n", (unsigned long long)divs.poolBQmineDividend);
        LOG("  Pool B qRWA Div:      %llu QU\n", (unsigned long long)divs.poolBQrwaDividend);
        LOG("  Pool C QMINE Div:     %llu QU\n", (unsigned long long)divs.poolCQmineDividend);
        LOG("  Pool C qRWA Div:      %llu QU\n", (unsigned long long)divs.poolCQrwaDividend);
        LOG("  Pool D Revenue:       %llu QU\n", (unsigned long long)divs.poolDRevenuePool);
        LOG("  Pool D QMINE Div:     %llu QU\n", (unsigned long long)divs.poolDQmineDividend);
        LOG("  Pool D qRWA Div:      %llu QU\n", (unsigned long long)divs.poolDQrwaDividend);
    }

    // Governance params
    QRWAGetGovParams_output gov = {};
    if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_GOV_PARAMS, nullptr, 0, &gov, sizeof(gov)))
    {
        LOG("\nGovernance Parameters:\n");
        printIdentity("  Admin:         ", gov.params.mAdminAddress);
        printIdentity("  Electricity:   ", gov.params.electricityAddress);
        printIdentity("  Maintenance:   ", gov.params.maintenanceAddress);
        printIdentity("  Reinvestment:  ", gov.params.reinvestmentAddress);
        printIdentity("  QMINE Dev:     ", gov.params.qmineDevAddress);
        LOG("  Electricity %%:  %llu\n", (unsigned long long)gov.params.electricityPercent);
        LOG("  Maintenance %%:  %llu\n", (unsigned long long)gov.params.maintenancePercent);
        LOG("  Reinvestment %%: %llu\n", (unsigned long long)gov.params.reinvestmentPercent);
    }
}

// ─── qrwaAssets — show assets held by the contract ─────────────

void qrwaAssets(const char* nodeIp, int nodePort)
{
    // Treasury balance (QMINE tokens held)
    QRWAGetTreasuryBalance_output treasury = {};
    if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_TREASURY_BALANCE, nullptr, 0, &treasury, sizeof(treasury)))
    {
        LOG("QMINE Treasury Balance: %llu QU\n", (unsigned long long)treasury.balance);
    }
    else
    {
        LOG("ERROR: Could not query GetTreasuryBalance (fn 4)\n");
    }

    // General assets (non-QMINE tokens/shares)
    QRWAGetGeneralAssets_output* assets = new QRWAGetGeneralAssets_output();
    memset(assets, 0, sizeof(QRWAGetGeneralAssets_output));
    if (runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_GENERAL_ASSETS, nullptr, 0, assets, sizeof(QRWAGetGeneralAssets_output)))
    {
        uint64_t count = assets->count;
        LOG("\nGeneral Assets held by qRWA contract: %llu\n", (unsigned long long)count);

        if (count > QRWA_MAX_ASSETS) count = QRWA_MAX_ASSETS;

        for (uint64_t i = 0; i < count; i++)
        {
            char assetName[8] = {};
            assetNameToString(assets->assets[i].assetName, assetName);

            char issuerIdentity[128] = {};
            getIdentityFromPublicKey(assets->assets[i].issuer, issuerIdentity, false);

            LOG("  [%llu] %s / %s  —  Balance: %llu\n",
                (unsigned long long)(i + 1),
                assetName,
                issuerIdentity,
                (unsigned long long)assets->balances[i]);
        }

        if (count == 0)
        {
            LOG("  (no assets)\n");
        }
    }
    else
    {
        LOG("ERROR: Could not query GetGeneralAssets (fn 10)\n");
    }
    delete assets;
}

// ─── qrwaGovParams — show current governance parameters ────────

void qrwaGovParams(const char* nodeIp, int nodePort)
{
    QRWAGetGovParams_output result = {};
    if (!runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_GOV_PARAMS, nullptr, 0, &result, sizeof(result)))
    {
        LOG("ERROR: Could not query GetGovParams (fn 1)\n");
        return;
    }

    LOG("═══ qRWA Governance Parameters ═══════════════════════════\n");
    printIdentity("  Admin:         ", result.params.mAdminAddress);
    printIdentity("  Electricity:   ", result.params.electricityAddress);
    printIdentity("  Maintenance:   ", result.params.maintenanceAddress);
    printIdentity("  Reinvestment:  ", result.params.reinvestmentAddress);
    printIdentity("  QMINE Dev:     ", result.params.qmineDevAddress);
    LOG("  Electricity %%:  %llu\n", (unsigned long long)result.params.electricityPercent);
    LOG("  Maintenance %%:  %llu\n", (unsigned long long)result.params.maintenancePercent);
    LOG("  Reinvestment %%: %llu\n", (unsigned long long)result.params.reinvestmentPercent);
}

// ─── qrwaGovPoll — show details of a specific governance poll ──

void qrwaGovPoll(const char* nodeIp, int nodePort, uint64_t proposalId)
{
    QRWAGetGovPoll_input input = {};
    input.proposalId = proposalId;

    QRWAGetGovPoll_output result = {};
    if (!runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_GOV_POLL, &input, sizeof(input), &result, sizeof(result)))
    {
        LOG("ERROR: Could not query GetGovPoll (fn 2)\n");
        return;
    }

    if (result.status == 0)
    {
        LOG("Governance poll %llu: NOT FOUND\n", (unsigned long long)proposalId);
        return;
    }

    const char* statusStr = "Unknown";
    switch (result.proposal.status)
    {
        case 0: statusStr = "Empty"; break;
        case 1: statusStr = "Active"; break;
        case 2: statusStr = "Passed"; break;
        case 3: statusStr = "Failed"; break;
    }

    LOG("═══ Governance Poll #%llu ═══════════════════════════════\n", (unsigned long long)proposalId);
    LOG("  Status: %s (%llu)\n", statusStr, (unsigned long long)result.proposal.status);
    LOG("  Score:  %llu\n", (unsigned long long)result.proposal.score);
    LOG("  Proposed Parameters:\n");
    printIdentity("    Admin:         ", result.proposal.params.mAdminAddress);
    printIdentity("    Electricity:   ", result.proposal.params.electricityAddress);
    printIdentity("    Maintenance:   ", result.proposal.params.maintenanceAddress);
    printIdentity("    Reinvestment:  ", result.proposal.params.reinvestmentAddress);
    printIdentity("    QMINE Dev:     ", result.proposal.params.qmineDevAddress);
    LOG("    Electricity %%:  %llu\n", (unsigned long long)result.proposal.params.electricityPercent);
    LOG("    Maintenance %%:  %llu\n", (unsigned long long)result.proposal.params.maintenancePercent);
    LOG("    Reinvestment %%: %llu\n", (unsigned long long)result.proposal.params.reinvestmentPercent);
}

// ─── qrwaGovPollIds — list active governance poll IDs ──────────

void qrwaGovPollIds(const char* nodeIp, int nodePort)
{
    QRWAGetActiveGovPollIds_output result = {};
    if (!runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_ACTIVE_GOV_POLL_IDS, nullptr, 0, &result, sizeof(result)))
    {
        LOG("ERROR: Could not query GetActiveGovPollIds (fn 8)\n");
        return;
    }

    LOG("═══ Active Governance Polls ═══════════════════════════════\n");
    LOG("  Count: %llu\n", (unsigned long long)result.count);
    uint64_t count = result.count;
    if (count > QRWA_MAX_GOV_POLLS) count = QRWA_MAX_GOV_POLLS;

    for (uint64_t i = 0; i < count; i++)
    {
        LOG("  [%llu] Proposal ID: %llu\n", (unsigned long long)(i + 1), (unsigned long long)result.ids[i]);
    }

    if (count == 0)
        LOG("  (no active governance polls)\n");
}

// ─── qrwaDividends — show dividend balances for all pools ──────

void qrwaDividends(const char* nodeIp, int nodePort)
{
    QRWAGetDividendBalances_output result = {};
    if (!runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_DIVIDEND_BALANCES, nullptr, 0, &result, sizeof(result)))
    {
        LOG("ERROR: Could not query GetDividendBalances (fn 5)\n");
        return;
    }

    LOG("═══ qRWA Dividend Balances ═══════════════════════════════\n");
    LOG("  Revenue Pools:\n");
    LOG("    Pool A (Qubic Mining):   %llu QU\n", (unsigned long long)result.revenuePoolA);
    LOG("    Pool B (SC Assets):      %llu QU\n", (unsigned long long)result.revenuePoolB);
    LOG("    Dedicated Revenue Pool:  %llu QU\n", (unsigned long long)result.dedicatedRevenuePool);
    LOG("\n  Sub-Dividends:\n");
    LOG("    Pool A — QMINE Holders:  %llu QU\n", (unsigned long long)result.poolAQmineDividend);
    LOG("    Pool A — qRWA Holders:   %llu QU\n", (unsigned long long)result.poolAQrwaDividend);
    LOG("    Pool B — QMINE Holders:  %llu QU\n", (unsigned long long)result.poolBQmineDividend);
    LOG("    Pool B — qRWA Holders:   %llu QU\n", (unsigned long long)result.poolBQrwaDividend);
    LOG("    Pool C — QMINE Holders:  %llu QU\n", (unsigned long long)result.poolCQmineDividend);
    LOG("    Pool C — qRWA Holders:   %llu QU\n", (unsigned long long)result.poolCQrwaDividend);
    LOG("    Pool D — Revenue:        %llu QU\n", (unsigned long long)result.poolDRevenuePool);
    LOG("    Pool D — QMINE Holders:  %llu QU\n", (unsigned long long)result.poolDQmineDividend);
    LOG("    Pool D — qRWA Holders:   %llu QU\n", (unsigned long long)result.poolDQrwaDividend);
}

// ─── qrwaScDividends — show SC dividend tracking ──────────────

void qrwaScDividends(const char* nodeIp, int nodePort)
{
    QRWAGetScDividendTracking_output* result = new QRWAGetScDividendTracking_output();
    memset(result, 0, sizeof(QRWAGetScDividendTracking_output));
    if (!runContractFunction(nodeIp, nodePort, QRWA_CONTRACT_INDEX,
            QRWA_GET_SC_DIVIDEND_TRACKING, nullptr, 0, result, sizeof(QRWAGetScDividendTracking_output)))
    {
        LOG("ERROR: Could not query GetScDividendTracking (fn 15)\n");
        delete result;
        return;
    }

    LOG("═══ SC Dividend Tracking (Pool B Revenue Sources) ═══════\n");
    LOG("  Tracked SC assets: %llu\n\n", (unsigned long long)result->count);

    uint64_t count = result->count;
    if (count > QRWA_MAX_ASSETS) count = QRWA_MAX_ASSETS;

    if (count == 0)
    {
        LOG("  (no SC dividends tracked)\n");
        delete result;
        return;
    }

    LOG("  %-60s %8s %20s\n", "SC Contract", "Name", "Cumulative QU");
    LOG("  %-60s %8s %20s\n",
        "------------------------------------------------------------",
        "--------", "--------------------");

    for (uint64_t i = 0; i < count; i++)
    {
        char scIdentity[128] = {};
        getIdentityFromPublicKey(result->scContractIds[i], scIdentity, false);
        uint64_t idx = 0;
        memcpy(&idx, result->scContractIds[i], sizeof(uint64_t));
        const char* name = getContractName((uint32_t)idx);
        if (!name) name = "?";
        LOG("  %-60s %8s %20llu\n",
            scIdentity, name,
            (unsigned long long)result->cumulativeDividends[i]);
    }
    delete result;
}

// ─── Procedure wrappers ────────────────────────────────────────

void qrwaDonateToTreasury(const char* nodeIp, int nodePort, const char* seed,
                           uint64_t amount, uint32_t scheduledTickOffset)
{
    QRWADonateToTreasury_input input = {};
    input.amount = amount;

    LOG("Donating %llu QMINE to qRWA treasury...\n", (unsigned long long)amount);
    makeContractTransaction(nodeIp, nodePort, seed, QRWA_CONTRACT_INDEX,
        QRWA_PROC_DONATE_TO_TREASURY, 0, sizeof(input), &input, scheduledTickOffset);
}

void qrwaVoteGovParams(const char* nodeIp, int nodePort, const char* seed,
                        const QRWAGovParams& proposal, uint32_t scheduledTickOffset)
{
    QRWAVoteGovParams_input input = {};
    input.proposal = proposal;

    LOG("Submitting governance vote...\n");
    makeContractTransaction(nodeIp, nodePort, seed, QRWA_CONTRACT_INDEX,
        QRWA_PROC_VOTE_GOV_PARAMS, 0, sizeof(input), &input, scheduledTickOffset);
}

void qrwaSetPoolARevenueAddress(const char* nodeIp, int nodePort, const char* seed,
                                 const char* newAddress, uint32_t scheduledTickOffset)
{
    QRWASetPoolARevenueAddress_input input = {};
    getPublicKeyFromIdentity(newAddress, input.newAddress);

    char verifiedId[128] = {};
    getIdentityFromPublicKey(input.newAddress, verifiedId, false);
    LOG("Setting Pool A revenue address to: %s\n", verifiedId);

    makeContractTransaction(nodeIp, nodePort, seed, QRWA_CONTRACT_INDEX,
        QRWA_PROC_SET_POOL_A_ADDRESS, 0, sizeof(input), &input, scheduledTickOffset);
}

void qrwaSetPoolDRevenueAddress(const char* nodeIp, int nodePort, const char* seed,
                                 const char* newAddress, uint32_t scheduledTickOffset)
{
    QRWASetPoolDRevenueAddress_input input = {};
    getPublicKeyFromIdentity(newAddress, input.newAddress);

    char verifiedId[128] = {};
    getIdentityFromPublicKey(input.newAddress, verifiedId, false);
    LOG("Setting Pool D revenue address to: %s\n", verifiedId);

    makeContractTransaction(nodeIp, nodePort, seed, QRWA_CONTRACT_INDEX,
        QRWA_PROC_SET_POOL_D_ADDRESS, 0, sizeof(input), &input, scheduledTickOffset);
}

void qrwaDepositGeneralAsset(const char* nodeIp, int nodePort, const char* seed,
                              const char* issuerIdentity, const char* assetName,
                              uint64_t amount, uint32_t scheduledTickOffset)
{
    QRWADepositGeneralAsset_input input = {};
    getPublicKeyFromIdentity(issuerIdentity, input.issuer);
    input.assetName = assetNameFromString(assetName);
    input.amount = amount;

    LOG("Depositing %llu shares of %s into qRWA...\n", (unsigned long long)amount, assetName);
    makeContractTransaction(nodeIp, nodePort, seed, QRWA_CONTRACT_INDEX,
        7, 0, sizeof(input), &input, scheduledTickOffset); // proc 7
}

void qrwaRevokeAssetMgmt(const char* nodeIp, int nodePort, const char* seed,
                          const char* issuerIdentity, const char* assetName,
                          int64_t numberOfShares, uint32_t scheduledTickOffset)
{
    QRWARevokeAssetManagementRights_input input = {};
    getPublicKeyFromIdentity(issuerIdentity, input.issuer);
    input.assetName = assetNameFromString(assetName);
    input.numberOfShares = numberOfShares;

    LOG("Revoking management of %lld shares of %s from qRWA (100 QU fee)...\n",
        (long long)numberOfShares, assetName);
    makeContractTransaction(nodeIp, nodePort, seed, QRWA_CONTRACT_INDEX,
        8, 100, sizeof(input), &input, scheduledTickOffset); // proc 8, 100 QU fee
}
