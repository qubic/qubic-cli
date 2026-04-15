#include "escrow.h"
#include "structs.h"
#include "logger.h"
#include "connection.h"
#include "wallet_utils.h"
#include "node_utils.h"
#include "key_utils.h"
#include "k12_and_key_utils.h"

#include <sstream>

#define ESCROW_CONTRACT_INDEX 27

#define ESCROW_CREATE_DEAL 1
#define ESCROW_ACCEPT_DEAL 2
#define ESCROW_MAKE_DEAL_PUBLIC 3
#define ESCROW_CANCEL_DEAL 4
#define ESCROW_TRANSFER_RIGHTS 5

#define ESCROW_GET_DEALS 1
#define ESCROW_GET_FREE_ASSET 2

constexpr uint64_t ESCROW_CREATE_DEAL_FEE = 250000ULL;
constexpr uint64_t ESCROW_ACCEPT_DEAL_FEE = 250000ULL;
constexpr uint64_t ESCROW_MAKE_DEAL_PUBLIC_FEE = 1ULL;
constexpr uint64_t ESCROW_CANCEL_DEAL_FEE = 1ULL;
constexpr uint64_t ESCROW_FEE_PER_SHARE = 3000000ULL;
constexpr uint64_t ESCROW_ADDITIONAL_CREATION_FEE = 200; // 2%
constexpr auto ESCROW_SC_ADDRESS = "BBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAXPZM";
constexpr auto SHARES_ISSUER = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB";

void escrowCreateDeal(const char* nodeIp, int nodePort, const char* seed,
    const char* acceptorId,
    const char* offeredAssetsCommaSeparated,
    const char* requestedAssetsCommaSeparated)
{
    EscrowCreateDeal_input input;
    memset(input.acceptorId, 0, 32);
    getPublicKeyFromIdentity(acceptorId, input.acceptorId);

    uint64_t sharesFees = 0;

    input.offeredAssetsAmount = parseAssets(offeredAssetsCommaSeparated, input.offeredAssets, 4, input.offeredQU, sharesFees);
    input.requestedAssetsAmount = parseAssets(requestedAssetsCommaSeparated, input.requestedAssets, 4, input.requestedQU, sharesFees);

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t digest[32];
    uint8_t signature[64];
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    const bool isLowerCase = false;

    getSubseedFromSeed((uint8_t*) seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    memset(destPublicKey, 0, 32);
    ((uint64_t*) destPublicKey)[0] = ESCROW_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        EscrowCreateDeal_input inputData;
        uint8_t sig[64];
    } packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = ESCROW_CREATE_DEAL_FEE + input.offeredQU + sharesFees;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + DEFAULT_SCHEDULED_TICK_OFFSET;
    packet.transaction.inputType = ESCROW_CREATE_DEAL;
    packet.transaction.inputSize = sizeof(input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(input),
                   digest, 
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction, 
                   sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE, 
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("\n%u\n", currentTick);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + DEFAULT_SCHEDULED_TICK_OFFSET, txHash);
    LOG("to check your tx confirmation status\n");
}

void escrowGetDeals(const char* nodeIp, int nodePort, const char* seed, const int64_t proposedOffset, const int64_t publicOffset)
{
    EscrowGetDeals_output output = escrowGetDealsOutput(nodeIp, nodePort, seed, proposedOffset, publicOffset);

    LOG("Current deals amount for owner: %lld\n", output.ownedDealsAmount);
    LOG("Proposed deals amount for owner: %lld\n", output.proposedDealsAmount);
    LOG("Public deals amount: %lld\n\n", output.publicDealsAmount);

    printDeals(output.ownedDealsAmount, output.ownedDeals, "OWNED DEALS\n", " (i give)", " (i get) ", " Acceptor ID");
    printDeals(output.proposedDealsAmount, output.proposedDeals, "\nPROPOSED DEALS\n", " (i get) ", " (i give)", " Creator ID");
    printDeals(output.publicDealsAmount, output.publicDeals, "\nPUBLIC DEALS\n", " (i get) ", " (i give)", " Creator ID");
}

int64_t escrowGetRequestedQUForDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t& index)
{
    EscrowGetDeals_output output = escrowGetDealsOutput(nodeIp, nodePort, seed, 0, 0);

    for (int i = 0; i < output.proposedDealsAmount; i++)
    {
        if (output.proposedDeals[i].index == index)
        {
            return output.proposedDeals[i].requestedQU;
        }
    }

    for (int i = 0; i < output.publicDealsAmount; i++)
    {
        if (output.publicDeals[i].index == index)
        {
            return output.publicDeals[i].requestedQU;
        }
    }

    return -1;
}

int64_t escrowGetSharesFeesForDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t& index)
{
    EscrowGetDeals_output output = escrowGetDealsOutput(nodeIp, nodePort, seed, 0, 0);

    for (int i = 0; i < output.proposedDealsAmount; i++)
    {
        if (output.proposedDeals[i].index == index)
        {
            uint64_t sharesFees = 0;
            for (int j = 0; j < output.proposedDeals[i].offeredAssetsAmount; j++)
            {
                char iden[61];
                memset(iden, 0, 61);
                getIdentityFromPublicKey(output.proposedDeals[i].offeredAssets[j].issuer, iden, false);
                if (strcmp(iden, SHARES_ISSUER) == 0)
                {
                    sharesFees += (output.proposedDeals[i].offeredAssets[j].amount * ESCROW_FEE_PER_SHARE / 2);
                }
            }

            for (int j = 0; j < output.proposedDeals[i].requestedAssetsAmount; j++)
            {
                char iden[61];
                memset(iden, 0, 61);
                getIdentityFromPublicKey(output.proposedDeals[i].requestedAssets[j].issuer, iden, false);
                if (strcmp(iden, SHARES_ISSUER) == 0)
                {
                    sharesFees += (output.proposedDeals[i].requestedAssets[j].amount * ESCROW_FEE_PER_SHARE / 2);
                }
            }

            return sharesFees;
        }
    }

    for (int i = 0; i < output.publicDealsAmount; i++)
    {
        if (output.publicDeals[i].index == index)
        {
            uint64_t sharesFees = 0;
            for (int j = 0; j < output.publicDeals[i].offeredAssetsAmount; j++)
            {
                char iden[61];
                memset(iden, 0, 61);
                getIdentityFromPublicKey(output.publicDeals[i].offeredAssets[j].issuer, iden, false);
                if (strcmp(iden, SHARES_ISSUER) == 0)
                {
                    sharesFees += (output.publicDeals[i].offeredAssets[j].amount * ESCROW_FEE_PER_SHARE / 2);
                }
            }

            for (int j = 0; j < output.publicDeals[i].requestedAssetsAmount; j++)
            {
                char iden[61];
                memset(iden, 0, 61);
                getIdentityFromPublicKey(output.publicDeals[i].requestedAssets[j].issuer, iden, false);
                if (strcmp(iden, SHARES_ISSUER) == 0)
                {
                    sharesFees += (output.publicDeals[i].requestedAssets[j].amount * ESCROW_FEE_PER_SHARE / 2);
                }
            }

            return sharesFees;
        }
    }

    return -1;
}

EscrowGetDeals_output escrowGetDealsOutput(const char* nodeIp, int nodePort, const char* seed, const int64_t proposedOffset, const int64_t publicOffset)
{
    EscrowGetDeals_input input;
    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    getSubseedFromSeed((uint8_t*) seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    memset(input.owner, 0, 32);
    memcpy(input.owner, sourcePublicKey, 32); 
    input.proposedOffset = proposedOffset;
    input.publicOffset = publicOffset;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return EscrowGetDeals_output{};
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        EscrowGetDeals_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = ESCROW_CONTRACT_INDEX;
    req.rcf.inputType = ESCROW_GET_DEALS;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    EscrowGetDeals_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<EscrowGetDeals_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get deals.\n");
        return EscrowGetDeals_output{};
    }

    return output;
}

void escrowAcceptDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t index)
{
    int64_t requestedQU = escrowGetRequestedQUForDeal(nodeIp, nodePort, seed, index);
    int64_t sharesFees = escrowGetSharesFeesForDeal(nodeIp, nodePort, seed, index);

    if (requestedQU < 0 || sharesFees < 0)
    {
        LOG("Failed to get requestedQU or sharesFees for deal with index: %lld", index);
        return;
    }
    uint64_t fee = ESCROW_ACCEPT_DEAL_FEE + requestedQU + sharesFees;
    escrowOperateDeal(nodeIp, nodePort, seed, index, fee, ESCROW_ACCEPT_DEAL);
}

void escrowMakeDealPublic(const char* nodeIp, int nodePort, const char* seed, const int64_t index)
{
    escrowOperateDeal(nodeIp, nodePort, seed, index, ESCROW_MAKE_DEAL_PUBLIC_FEE, ESCROW_MAKE_DEAL_PUBLIC);
}

void escrowCancelDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t index)
{
    escrowOperateDeal(nodeIp, nodePort, seed, index, ESCROW_CANCEL_DEAL_FEE, ESCROW_CANCEL_DEAL);
}

void escrowOperateDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t index, const int64_t fee, const unsigned short inputType)
{
    EscrowOperateDeal_input input;
    input.index = index;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t digest[32];
    uint8_t signature[64];
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    const bool isLowerCase = false;

    getSubseedFromSeed((uint8_t*) seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    memset(destPublicKey, 0, 32);
    ((uint64_t*) destPublicKey)[0] = ESCROW_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        EscrowOperateDeal_input inputData;
        uint8_t sig[64];
    } packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = fee;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + DEFAULT_SCHEDULED_TICK_OFFSET;
    packet.transaction.inputType = inputType;
    packet.transaction.inputSize = sizeof(input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(input),
                   digest, 
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction, 
                   sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE, 
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("\n%u\n", currentTick);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + DEFAULT_SCHEDULED_TICK_OFFSET, txHash);
    LOG("to check your tx confirmation status\n");
}

void escrowTransferRights(const char* nodeIp, int nodePort, const char* seed, const char* assetName, const char* issuer, const int64_t amount)
{
    TransferShareManagementRights_input input;
    memset(&input.asset.assetName, 0, 8);
    memcpy(&input.asset.assetName, assetName, std::min(strlen(assetName), (size_t) 7));
    input.amount = amount;
    memset(input.asset.issuer, 0, 32);
    getPublicKeyFromIdentity(issuer, input.asset.issuer);

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t digest[32];
    uint8_t signature[64];
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    const bool isLowerCase = false;

    getSubseedFromSeed((uint8_t*) seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    memset(destPublicKey, 0, 32);
    ((uint64_t*) destPublicKey)[0] = ESCROW_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        TransferShareManagementRights_input inputData;
        uint8_t sig[64];
    } packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 100;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + DEFAULT_SCHEDULED_TICK_OFFSET;
    packet.transaction.inputType = ESCROW_TRANSFER_RIGHTS;
    packet.transaction.inputSize = sizeof(input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(input),
                   digest, 
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction, 
                   sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE, 
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("\n%u\n", currentTick);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + DEFAULT_SCHEDULED_TICK_OFFSET, txHash);
    LOG("to check your tx confirmation status\n");
}

void escrowGetFreeAsset(const char* nodeIp, int nodePort, const char* seed, const char* assetName, const char* issuer)
{
    EscrowGetFreeAsset_input input;
    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t pk[32] = { 0 };
    getSubseedFromSeed((uint8_t*) seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, pk);
    getPublicKeyFromIdentity(issuer, sourcePublicKey);

    memset(input.owner, 0, 32);
    memcpy(input.owner, pk, 32);
    memset(input.asset.issuer, 0, 32);
    memcpy(input.asset.issuer, sourcePublicKey, 32);
    memset(&input.asset.assetName, 0, 8);
    memcpy(&input.asset.assetName, assetName, std::min(strlen(assetName), (size_t) 7));

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        EscrowGetFreeAsset_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = ESCROW_CONTRACT_INDEX;
    req.rcf.inputType = ESCROW_GET_FREE_ASSET;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    EscrowGetFreeAsset_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<EscrowGetFreeAsset_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get free asset amount.\n");
        return;
    }

    LOG("Free asset amount: %lld\n", output.freeAmount);
}

int parseAssets(const std::string& inputStr, EscrowCreateDeal_input::AssetWithAmount* outputArray, const int& maxCount, uint64_t& QUAmount, uint64_t& sharesFees) 
{
    std::string QUAmountStr;
    std::string assetsStr;
    size_t pos = inputStr.find(':');
    if (pos != std::string::npos) 
    {
        QUAmountStr = inputStr.substr(0, pos);
        QUAmount = std::stoll(QUAmountStr);
        assetsStr = inputStr.substr(pos + 1);
    }
    else 
    {
        QUAmount = std::stoll(inputStr);
        return 0;
    }

    std::stringstream ss(assetsStr);
    std::string asset;
    int count = 0;

    while (std::getline(ss, asset, ':') && count < maxCount)
    {
        std::stringstream ssAsset(asset);
        std::string part;

        for (int i = 0; i < 3; i++)
        {
            if (!std::getline(ssAsset, part, ','))
            {
                LOG("Failed to parse assets");
                return 0;
            }

            if (i == 0)
            {
                memset(&outputArray[count].name, 0, 8);
                memcpy(&outputArray[count].name, part.c_str(), std::min<size_t>(part.size(), 8));
            }
            else if (i == 1)
            {
                uint8_t issuer[32] = {0};
                getPublicKeyFromIdentity(part.c_str(), issuer);
                memcpy(outputArray[count].issuer, issuer, 32);
            } 
            else if (i == 2)
            {
                outputArray[count].amount = std::stoll(part);

                char iden[61];
                memset(iden, 0, 61);
                getIdentityFromPublicKey(outputArray[count].issuer, iden, false);
                if (strcmp(iden, SHARES_ISSUER) == 0)
                {
                    sharesFees += (outputArray[count].amount * ESCROW_FEE_PER_SHARE / 2);
                }
            }
        }

        count++;
    }

    return count;
}

void printDeals(int64_t dealsAmount, const EscrowGetDeals_output::Deal* deals, const char* dealTypeName, const char* p1, const char* p2, const char* p3)
{
    if (dealsAmount <= 0)
    {
        return;
    }
    LOG("%s", dealTypeName);
    LOG("%s\n", std::string().assign(237, '-').c_str());
    LOG("%-81s|%-24s%15s%-38s|%-24s%15s%s\n", "", "", "Offered assets", p1, "", "Requested assets", p2);
    LOG("%s\n", std::string().assign(237, '-').c_str());
    LOG("%-18s|%-62s|%-14s|%-62s|%-14s|%-60s\n", "# (Index / Epoch)", p3, " QU Amount", " Asset (Issuer / Name / Amount)", " QU Amount", " Asset (Issuer / Name / Amount)");
    LOG("%s\n", std::string().assign(237, '-').c_str());
    for (int i = 0; i < dealsAmount; i++)
    {
        char iden[61];
        memset(iden, 0, 61);
        getIdentityFromPublicKey(deals[i].acceptorId, iden, false);
        for (int j = 0; j < 4; j++)
        {
            bool isOffered = j < deals[i].offeredAssetsAmount;
            bool isRequested = j < deals[i].requestedAssetsAmount;
            if (!isOffered && !isRequested && j > 0)
            {
                continue;
            }
            char iden1[61];
            memset(iden1, 0, 61);
            getIdentityFromPublicKey(deals[i].offeredAssets[j].issuer, iden1, false);
            char iden2[61];
            memset(iden2, 0, 61);
            getIdentityFromPublicKey(deals[i].requestedAssets[j].issuer, iden2, false);
            std::string offeredAsset(reinterpret_cast<const char*>(&deals[i].offeredAssets[j].name), 8);
            std::string requestedAsset(reinterpret_cast<const char*>(&deals[i].requestedAssets[j].name), 8);
            LOG("%-18s%-2s%-61s| %-13s| %-61s| %-13s| %-59s\n%-18s%2s%62s%15s %-60s |%15s %-60s\n%82s%15s %-60s |%15s %-60s\n",
                (j == 0) ? std::to_string(deals[i].index).c_str() : "",
                (j == 0) ? "| " : "  ",
                (j == 0 && strcmp(iden, ESCROW_SC_ADDRESS) != 0) ? iden : "",
                (j == 0) ? std::to_string(deals[i].offeredQU).c_str() : "",
                isOffered ? iden1 : "",
                (j == 0) ? std::to_string(deals[i].requestedQU).c_str() : "",
                isRequested ? iden2 : "",
                (j == 0) ? std::to_string(deals[i].creationEpoch).c_str() : "",
                (j == 0) ? "| " : "  ",
                "|",
                "|",
                isOffered ? offeredAsset.c_str() : "",
                "|",
                isRequested ? requestedAsset.c_str() : "",
                "|",
                "|",
                isOffered ? std::to_string(deals[i].offeredAssets[j].amount).c_str() : "",
                "|",
                isRequested ? std::to_string(deals[i].requestedAssets[j].amount).c_str() : "");

            if (j + 1 < deals[i].offeredAssetsAmount || j + 1 < deals[i].requestedAssetsAmount)
            {
                LOG("%82s%15s%s|%15s%s\n",
                    "|",
                    "|",
                    std::string().assign(62, '-').c_str(),
                    "|",
                    std::string().assign(62, '-').c_str());
            }
        }
        LOG("%s\n", std::string().assign(237, '-').c_str());
    }
}
