#include "ggwp.h"
#include "wallet_utils.h"
#include "key_utils.h"
#include "logger.h"

static void printIdentity(const char* label, const uint8_t* pubkey)
{
    char id[128] = {};
    getIdentityFromPublicKey(pubkey, id, false);
    LOG("%s%s\n", label, id);
}

// ─── Query functions ───────────────────────────────────────────

void wpStatus(const char* nodeIp, int nodePort)
{
    LOG("═══ WolfPack Contract Status ═══════════════════════════\n\n");

    WPGetStatus_output out = {};
    if (runContractFunction(nodeIp, nodePort, WP_CONTRACT_INDEX,
            WP_GET_STATUS, nullptr, 0, &out, sizeof(out)))
    {
        LOG("Holder Count:            %llu\n", (unsigned long long)out.holderCount);
        LOG("Total Tokens Snapshot:   %llu\n", (unsigned long long)out.totalTokensSnapshot);
        LOG("Shareholder Count:       %llu\n", (unsigned long long)out.shareholderCount);
        LOG("Total Shares Snapshot:   %llu\n", (unsigned long long)out.totalSharesSnapshot);
        LOG("Clan Member Count:       %llu\n", (unsigned long long)out.clanMemberCount);
        LOG("Clan Weighted Total:     %llu\n", (unsigned long long)out.clanWeightedTotal);
        LOG("Pending Revenue:         %llu QU\n", (unsigned long long)out.pendingRevenue);
        LOG("Reinvestment Fund:       %llu QU\n", (unsigned long long)out.reinvestmentFund);
        LOG("Total Distributed:       %llu QU\n", (unsigned long long)out.totalDistributed);
        LOG("Total Deposited:         %llu QU\n", (unsigned long long)out.totalDeposited);
        LOG("Last Payout Tick:        %llu\n", (unsigned long long)out.lastPayoutTick);
        LOG("Last Distribution Epoch: %llu\n", (unsigned long long)out.lastDistributionEpoch);
        printIdentity("Admin:                   ", out.adminAddress);
    }
    else
    {
        LOG("ERROR: Could not query WP GetStatus (fn 1)\n");
    }
}

void wpHolderInfo(const char* nodeIp, int nodePort, const char* address)
{
    WPGetHolderInfo_input input = {};
    getPublicKeyFromIdentity(address, input.holderAddress);

    WPGetHolderInfo_output out = {};
    if (runContractFunction(nodeIp, nodePort, WP_CONTRACT_INDEX,
            WP_GET_HOLDER_INFO, &input, sizeof(input), &out, sizeof(out)))
    {
        LOG("═══ WolfPack Holder Info ════════════════════════════════\n\n");
        LOG("Address:       %s\n", address);
        LOG("Token Balance: %llu\n", (unsigned long long)out.tokenBalance);
        LOG("Is Holder:     %s\n", out.isHolder ? "YES" : "NO");
    }
    else
    {
        LOG("ERROR: Could not query WP GetHolderInfo (fn 2)\n");
    }
}

void wpClanMemberInfo(const char* nodeIp, int nodePort, const char* address)
{
    WPGetClanMemberInfo_input input = {};
    getPublicKeyFromIdentity(address, input.memberAddress);

    WPGetClanMemberInfo_output out = {};
    if (runContractFunction(nodeIp, nodePort, WP_CONTRACT_INDEX,
            WP_GET_CLAN_MEMBER_INFO, &input, sizeof(input), &out, sizeof(out)))
    {
        LOG("═══ WolfPack Clan Member Info ══════════════════════════\n\n");
        LOG("Address:   %s\n", address);
        LOG("Rank:      %llu\n", (unsigned long long)out.rank);
        LOG("Is Member: %s\n", out.isMember ? "YES" : "NO");
    }
    else
    {
        LOG("ERROR: Could not query WP GetClanMemberInfo (fn 3)\n");
    }
}

void wpShareholderInfo(const char* nodeIp, int nodePort, const char* address)
{
    WPGetShareholderInfo_input input = {};
    getPublicKeyFromIdentity(address, input.shareholderAddress);

    WPGetShareholderInfo_output out = {};
    if (runContractFunction(nodeIp, nodePort, WP_CONTRACT_INDEX,
            WP_GET_SHAREHOLDER_INFO, &input, sizeof(input), &out, sizeof(out)))
    {
        LOG("═══ WolfPack Shareholder Info ══════════════════════════\n\n");
        LOG("Address:        %s\n", address);
        LOG("SC Shares:      %llu\n", (unsigned long long)out.shares);
        LOG("Is Shareholder: %s\n", out.isShareholder ? "YES" : "NO");
    }
    else
    {
        LOG("ERROR: Could not query WP GetShareholderInfo (fn 4)\n");
    }
}

// ─── Procedures ────────────────────────────────────────────────

void wpDepositRevenue(const char* nodeIp, int nodePort, const char* seed,
                      uint64_t amount, uint32_t scheduledTickOffset)
{
    LOG("Depositing %llu QU revenue to WolfPack...\n", (unsigned long long)amount);
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_DEPOSIT_REVENUE, amount, 0, nullptr, scheduledTickOffset);
}

void wpAddClanMember(const char* nodeIp, int nodePort, const char* seed,
                     const char* memberAddress, uint64_t rank, uint32_t scheduledTickOffset)
{
    WPAddClanMember_input input = {};
    getPublicKeyFromIdentity(memberAddress, input.memberAddress);
    input.rank = rank;

    LOG("Adding clan member (rank %llu): %s\n", (unsigned long long)rank, memberAddress);
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_ADD_CLAN_MEMBER, 0, sizeof(input), &input, scheduledTickOffset);
}

void wpRemoveClanMember(const char* nodeIp, int nodePort, const char* seed,
                        const char* memberAddress, uint32_t scheduledTickOffset)
{
    WPRemoveClanMember_input input = {};
    getPublicKeyFromIdentity(memberAddress, input.memberAddress);

    LOG("Removing clan member: %s\n", memberAddress);
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_REMOVE_CLAN_MEMBER, 0, sizeof(input), &input, scheduledTickOffset);
}

void wpSetClanRank(const char* nodeIp, int nodePort, const char* seed,
                   const char* memberAddress, uint64_t rank, uint32_t scheduledTickOffset)
{
    WPSetClanRank_input input = {};
    getPublicKeyFromIdentity(memberAddress, input.memberAddress);
    input.rank = rank;

    LOG("Setting clan rank to %llu for: %s\n", (unsigned long long)rank, memberAddress);
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_SET_CLAN_RANK, 0, sizeof(input), &input, scheduledTickOffset);
}

void wpSetAdmin(const char* nodeIp, int nodePort, const char* seed,
                const char* newAdmin, uint32_t scheduledTickOffset)
{
    WPSetAdmin_input input = {};
    getPublicKeyFromIdentity(newAdmin, input.newAdmin);

    char verifiedId[128] = {};
    getIdentityFromPublicKey(input.newAdmin, verifiedId, false);
    LOG("Setting WP admin to: %s\n", verifiedId);
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_SET_ADMIN, 0, sizeof(input), &input, scheduledTickOffset);
}

void wpSetExcludeAddress(const char* nodeIp, int nodePort, const char* seed,
                         uint64_t slot, const char* address, uint32_t scheduledTickOffset)
{
    WPSetExcludeAddress_input input = {};
    input.slot = slot;
    getPublicKeyFromIdentity(address, input.address);

    LOG("Setting exclude slot %llu to: %s\n", (unsigned long long)slot, address);
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_SET_EXCLUDE_ADDRESS, 0, sizeof(input), &input, scheduledTickOffset);
}

// ─── Staking ───────────────────────────────────────────────────

void wpStakingInfo(const char* nodeIp, int nodePort, const char* address)
{
    WPGetStakingInfo_input input = {};
    getPublicKeyFromIdentity(address, input.stakerAddress);

    WPGetStakingInfo_output out = {};
    if (runContractFunction(nodeIp, nodePort, WP_CONTRACT_INDEX,
            WP_GET_STAKING_INFO, &input, sizeof(input), &out, sizeof(out)))
    {
        LOG("═══ WolfPack Staking Info ══════════════════════════════\n\n");
        LOG("Address:            %s\n", address);
        LOG("Is Staker:          %s\n", out.isStaker ? "YES" : "NO");
        LOG("Staked Amount:      %llu WP\n", (unsigned long long)out.stakedAmount);
        LOG("Pending Rewards:    %llu WP\n", (unsigned long long)out.pendingRewards);
        LOG("Unstake Amount:     %llu WP\n", (unsigned long long)out.unstakeAmount);
        LOG("Unstake Epoch:      %llu\n",    (unsigned long long)out.unstakeEpoch);
        LOG("Total Staked (all): %llu WP\n", (unsigned long long)out.totalStaked);
        LOG("Reward Pool:        %llu WP\n", (unsigned long long)out.stakingRewardPool);
    }
    else
    {
        LOG("ERROR: Could not query WP GetStakingInfo (fn 5)\n");
    }
}

void wpStake(const char* nodeIp, int nodePort, const char* seed,
             uint64_t numberOfShares, uint32_t scheduledTickOffset)
{
    WPStake_input input = {};
    input.numberOfShares = numberOfShares;

    LOG("Staking %llu WP tokens...\n", (unsigned long long)numberOfShares);
    LOG("NOTE: Transfer management rights on QX first (newMgmtIdx: 27)\n");
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_STAKE, 0, sizeof(input), &input, scheduledTickOffset);
}

void wpRequestUnstake(const char* nodeIp, int nodePort, const char* seed,
                      uint64_t numberOfShares, uint32_t scheduledTickOffset)
{
    WPRequestUnstake_input input = {};
    input.numberOfShares = numberOfShares;

    LOG("Requesting unstake of %llu WP tokens...\n", (unsigned long long)numberOfShares);
    LOG("WARNING: 2-epoch cooldown before FinalizeUnstake is possible\n");
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_REQUEST_UNSTAKE, 0, sizeof(input), &input, scheduledTickOffset);
}

void wpFinalizeUnstake(const char* nodeIp, int nodePort, const char* seed,
                       uint32_t scheduledTickOffset)
{
    LOG("Finalizing unstake...\n");
    LOG("NOTE: 100 QU QX fee will be charged\n");
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_FINALIZE_UNSTAKE, 0, 0, nullptr, scheduledTickOffset);
}

void wpDepositStakingRewards(const char* nodeIp, int nodePort, const char* seed,
                             uint64_t numberOfShares, uint32_t scheduledTickOffset)
{
    WPDepositStakingRewards_input input = {};
    input.numberOfShares = numberOfShares;

    LOG("Depositing %llu WP tokens into staking reward pool...\n", (unsigned long long)numberOfShares);
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_DEPOSIT_STAKING_REWARDS, 0, sizeof(input), &input, scheduledTickOffset);
}

void wpClaimStakingRewards(const char* nodeIp, int nodePort, const char* seed,
                           uint32_t scheduledTickOffset)
{
    LOG("Claiming staking rewards...\n");
    LOG("NOTE: 100 QU QX fee will be charged\n");
    makeContractTransaction(nodeIp, nodePort, seed, WP_CONTRACT_INDEX,
        WP_PROC_CLAIM_STAKING_REWARDS, 0, 0, nullptr, scheduledTickOffset);
}


void wpExcludeAddresses(const char* nodeIp, int nodePort)
{
    LOG("=== WolfPack Exclude Addresses ===\n\n");

    WPGetExcludeAddresses_output out = {};
    if (runContractFunction(nodeIp, nodePort, WP_CONTRACT_INDEX,
            WP_GET_EXCLUDE_ADDRESSES, nullptr, 0, &out, sizeof(out)))
    {
        printIdentity("Exclude Slot 1: ", out.address1);
        printIdentity("Exclude Slot 2: ", out.address2);
    }
    else
    {
        LOG("ERROR: Could not query WP GetExcludeAddresses (fn 6)\n");
    }
}


void wpDistPreview(const char* nodeIp, int nodePort, uint64_t amount)
{
    LOG("=== WolfPack Distribution Preview for %llu QU ===\n", (unsigned long long)amount);

    WPGetDistributionPreview_input in = {};
    in.amount = amount;
    WPGetDistributionPreview_output out = {};
    if (runContractFunction(nodeIp, nodePort, WP_CONTRACT_INDEX,
            WP_GET_DISTRIBUTION_PREVIEW, &in, sizeof(in), &out, sizeof(out)))
    {
        LOG("Holder Share:       %llu\n", (unsigned long long)out.holderShare);
        LOG("Shareholder Share:  %llu\n", (unsigned long long)out.shareholderShare);
        LOG("Clan Share:         %llu\n", (unsigned long long)out.clanShare);
        LOG("Reinvest Share:     %llu\n", (unsigned long long)out.reinvestShare);
    }
    else
    {
        LOG("ERROR: Could not query WP GetDistributionPreview (fn 7)\n");
    }
}
