#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cinttypes>

#include "qutil.h"
#include "key_utils.h"
#include "structs.h"
#include "defines.h"
#include "logger.h"
#include "node_utils.h"
#include "k12_and_key_utils.h"
#include "connection.h"
#include "wallet_utils.h"
#include "sanity_check.h"

constexpr int QUTIL_CONTRACT_ID = 4;

struct SendToManyV1_input
{
    uint8_t addresses[25][32];
    int64_t amounts[25];
};

struct BurnQubic_input
{
    long long amount;
};

struct BurnQubicForContract_input
{
    uint32_t contractIndexBurnedFor;
};

struct QueryFeeReserve_input
{
    uint32_t contractIndex;
};
struct QueryFeeReserve_output
{
    int64_t reserveAmount;
};

struct SendToManyBenchmark_input
{
    int64_t dstCount;
    int64_t numTransfersEach;
};

void readPayoutList(const char* payoutListFile, std::vector<std::string>& addresses, std::vector<int64_t>& amounts)
{
    addresses.resize(0);
    amounts.resize(0);
    std::ifstream infile(payoutListFile);
    std::string line;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        std::string a;
        int64_t b;
        if (!(iss >> a >> b)) { break; } // error
        addresses.push_back(a);
        amounts.push_back(b);
    }
}

static std::string assetNameFromInt64(unsigned long long assetName)
{
    char buffer[8];
    memcpy(buffer, &assetName, sizeof(assetName));
    buffer[7] = '\0';
    return std::string(buffer);
}

long long getSendToManyV1Fee(QCPtr qc)
{
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = qutilFunctionId::GetSendToManyV1Fee;
    packet.rcf.contractIndex = QUTIL_CONTRACT_ID;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    GetSendToManyV1Fee_output fee;
    memset(&fee, 0, sizeof(GetSendToManyV1Fee_output));
    try
    {
        fee = qc->receivePacketWithHeaderAs<GetSendToManyV1Fee_output>();
        return fee.fee;
    }
    catch (std::logic_error& e)
    {
        LOG(e.what());
        return -1;
    }
}

struct GetFees_output
{
    int64_t smt1InvocationFee;
    int64_t pollCreationFee;
    int64_t pollVoteFee;
    int64_t distributeQuToShareholderFeePerShareholder;
    int64_t shareholderProposalFee;

    // Placeholder for future fees, preventing incompatibilities for some extensions
    int64_t _futureFeePlaceholder0;
    int64_t _futureFeePlaceholder1;
    int64_t _futureFeePlaceholder2;
    int64_t _futureFeePlaceholder3;
    int64_t _futureFeePlaceholder4;
    int64_t _futureFeePlaceholder5;
};


bool getFees(QCPtr qc, GetFees_output& fees)
{
    if (!runContractFunction(nullptr, 0, QUTIL_CONTRACT_ID, qutilFunctionId::GetFees, nullptr, 0, &fees, sizeof(fees), &qc))
    {
        LOG("ERROR: Didn't receive valid response from GetFees!\n");
        return false;
    }
    return true;
}

void qutilPrintFees(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc)
    {
        LOG("Failed to connect to node.\n");
        return;
    }

    GetFees_output fees;
    if (!getFees(qc, fees))
        return;

    LOG("SendToManyV1 fee (var 0):               %" PRIi64 "\n", fees.smt1InvocationFee);
    LOG("Poll creation fee (var 1):              %" PRIi64 "\n", fees.pollCreationFee);
    LOG("Poll vote fee (var 2):                  %" PRIi64 "\n", fees.pollVoteFee);
    LOG("DistributeQuToShareholders fee (var 3): %" PRIi64 " per shareholder\n", fees.distributeQuToShareholderFeePerShareholder);
    LOG("Shareholder proposal fee (var 4):       %" PRIi64 "\n", fees.shareholderProposalFee);
}

void qutilSendToManyV1(const char* nodeIp, int nodePort, const char* seed, const char* payoutListFile, uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);

    std::vector<std::string> addresses;
    std::vector<int64_t> amounts;
    readPayoutList(payoutListFile, addresses, amounts);
    if (addresses.size() > 25)
    {
        LOG("WARNING: payout list has more than 25 addresses, only the first 25 addresses will be paid\n");
    }
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subseed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char publicIdentity[128] = {0};
    char txHash[128] = {0};
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    const bool isLowerCase = false;
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    ((uint64_t*)destPublicKey)[0] = QUTIL_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SendToManyV1_input stm;
        unsigned char signature[64];
    } packet;
    memset(&packet.stm, 0, sizeof(SendToManyV1_input));
    packet.transaction.amount = 0;
    for (int i = 0; i < std::min(25, int(addresses.size())); i++)
    {
        getPublicKeyFromIdentity(addresses[i].data(), packet.stm.addresses[i]);
        packet.stm.amounts[i] = amounts[i];
        packet.transaction.amount += amounts[i];
    }
    long long fee = getSendToManyV1Fee(qc);
    if (fee == -1)
        return;
    LOG("Send to many V1 fee: %lld\n", fee);
    packet.transaction.amount += fee; // fee
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = qutilProcedureId::SendToManyV1;
    packet.transaction.inputSize = sizeof(SendToManyV1_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(SendToManyV1_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(SendToManyV1_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("SendToManyV1 tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qutilBurnQubic(const char* nodeIp, int nodePort, const char* seed, long long amount, uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subseed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char publicIdentity[128] = {0};
    char txHash[128] = {0};
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    const bool isLowerCase = false;
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    ((uint64_t*)destPublicKey)[0] = QUTIL_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        BurnQubic_input bqi;
        unsigned char signature[64];
    } packet;
    packet.bqi.amount = amount;
    packet.transaction.amount = amount;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = qutilProcedureId::BurnQubic;
    packet.transaction.inputSize = sizeof(BurnQubic_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(BurnQubic_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(BurnQubic_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("BurnQubic tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qutilBurnQubicForContract(const char* nodeIp, int nodePort, const char* seed, long long amount, uint32_t contractIndex, uint32_t scheduledTickOffset)
{
    BurnQubicForContract_input input{ contractIndex };
    makeContractTransaction(nodeIp, nodePort, seed, QUTIL_CONTRACT_ID, qutilProcedureId::BurnQubicForContract, amount, sizeof(input), &input, scheduledTickOffset);
}

void qutilQueryFeeReserve(const char* nodeIp, int nodePort, uint32_t contractIndex)
{
    QueryFeeReserve_input input{ contractIndex };
    QueryFeeReserve_output output;
    if (runContractFunction(nodeIp, nodePort, QUTIL_CONTRACT_ID, qutilFunctionId::QueryFeeReserve,
        &input, sizeof(QueryFeeReserve_input), &output, sizeof(QueryFeeReserve_output)))
    {
        LOG("Fee reserve for contract %u: %" PRIi64 " Qu\n", contractIndex, output.reserveAmount);
    }
    else
    {
        LOG("Failed to query fee reserve for contract %u\n", contractIndex);
    }
}

void qutilSendToManyBenchmark(const char* nodeIp, int nodePort, const char* seed, uint32_t destinationCount, uint32_t numTransfersEach, uint32_t scheduledTickOffset)
{
    if (destinationCount <= 0 || numTransfersEach <= 0 || destinationCount * numTransfersEach + 2 > CONTRACT_ACTION_TRACKER_SIZE)
    {
        LOG("Invalid number of total transfers (0 or exceeds %llu)\n", CONTRACT_ACTION_TRACKER_SIZE - 2);
        return;
    }

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subseed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    const bool isLowerCase = false;
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    ((uint64_t*)destPublicKey)[0] = QUTIL_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SendToManyBenchmark_input bm;
        unsigned char signature[64];
    } packet;
    memset(&packet.bm, 0, sizeof(SendToManyBenchmark_input));
    packet.bm.dstCount = destinationCount;
    packet.bm.numTransfersEach = numTransfersEach;
    packet.transaction.amount = destinationCount * numTransfersEach; // no fee at the moment
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = qutilProcedureId::SendToManyBenchmark;
    packet.transaction.inputSize = sizeof(SendToManyBenchmark_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(SendToManyBenchmark_input),
        digest,
        32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(SendToManyBenchmark_input) + SIGNATURE_SIZE,
        digest,
        32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("SendToManyBenchmark tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qutilGetTotalNumberOfAssetShares(const char* nodeIp, int nodePort, const char* issuerIdentity, const char* assetName)
{
    struct
    {
        uint8_t issuer[32];
        uint64_t assetName;
    } input;
    uint64_t output;

    sanityCheckIdentity(issuerIdentity);
    getPublicKeyFromIdentity(issuerIdentity, input.issuer);

    sanityCheckValidAssetName(assetName);
    input.assetName = 0;
    memcpy(&input.assetName, assetName, std::min(size_t(7), strlen(assetName)));

    if (!runContractFunction(nodeIp, nodePort, QUTIL_CONTRACT_ID,
        GetTotalNumberOfAssetShares, &input, sizeof(input), &output, sizeof(output)))
    {
        LOG("ERROR: Didn't receive valid response from GetTotalNumberOfAssetShares!\n");
        return;
    }

    LOG("%" PRIu64 "\n", output);
}

void qutilDistributeQuToShareholders(const char* nodeIp, int nodePort, const char* seed,
    const char* issuerIdentity, const char* assetName, long long amount,
    uint32_t scheduledTickOffset)
{
    struct
    {
        uint8_t issuer[32];
        uint64_t assetName;
    } input;

    sanityCheckIdentity(issuerIdentity);
    getPublicKeyFromIdentity(issuerIdentity, input.issuer);

    sanityCheckValidAssetName(assetName);
    input.assetName = 0;
    memcpy(&input.assetName, assetName, std::min(size_t(7), strlen(assetName)));

    LOG("\nSending transaction to distribute QUs among shareholders ...\n");
    makeContractTransaction(nodeIp, nodePort, seed,
        QUTIL_CONTRACT_ID, DistributeQuToShareholders, amount,
        sizeof(input), &input, scheduledTickOffset);
}


// **********************
// *** Voting related ***
// **********************

// Helper function
std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

void qutilCreatePoll(const char* nodeIp, int nodePort, const char* seed,
    const char* poll_name, uint64_t poll_type, uint64_t min_amount,
    const char* github_link, const char* semicolon_separated_assets,
    uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc)
    {
        LOG("Failed to connect to node.\n");
        return;
    }

    CreatePoll_input input;
    memset(&input, 0, sizeof(input));
    strncpy((char*)input.poll_name, poll_name, 32);
    input.poll_type = poll_type;
    input.min_amount = min_amount;
    strncpy((char*)input.github_link, github_link, 256);

    if (poll_type == QUTIL_POLL_TYPE_ASSET && semicolon_separated_assets != nullptr)
    {
        std::string assets_str(semicolon_separated_assets);
        std::vector<std::string> asset_entries = split(assets_str, ';');
        if (asset_entries.empty() || asset_entries.size() > QUTIL_MAX_ASSETS_PER_POLL)
        {
            LOG("Invalid number of assets. Must be between 1 and %" PRIu64 ".\n", QUTIL_MAX_ASSETS_PER_POLL);
            return;
        }
        input.num_assets = asset_entries.size();
        for (size_t i = 0; i < asset_entries.size(); ++i)
        {
            std::vector<std::string> asset_pair = split(asset_entries[i], ',');
            if (asset_pair.size() != 2)
            {
                LOG("Invalid asset format at index %zu: expected 'asset_name,issuer'.\n", i);
                return;
            }
            const std::string& asset_name = asset_pair[0];
            const std::string& issuer = asset_pair[1];
            if (asset_name.empty() || issuer.empty())
            {
                LOG("Asset name or issuer cannot be empty at index %zu.\n", i);
                return;
            }
            qpi::Asset& asset = input.allowed_assets[i];
            memset(&asset, 0, sizeof(qpi::Asset));
            getPublicKeyFromIdentity(issuer.c_str(), asset.issuer);
            asset.assetName = assetNameFromString(asset_name.c_str());
        }
    }
    else if (poll_type == QUTIL_POLL_TYPE_QUBIC)
    {
        input.num_assets = 0;
    }
    else
    {
        LOG("Invalid poll type: %" PRIu64 ". Must be 1 (Qubic) or 2 (Asset).\n", poll_type);
        return;
    }

    GetFees_output fees;
    if (!getFees(qc, fees))
        return;

    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    ((uint64_t*)destPublicKey)[0] = QUTIL_CONTRACT_ID;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        CreatePoll_input inputData;
        unsigned char signature[64];
    } packet;
    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = fees.pollCreationFee;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = qutilProcedureId::CreatePoll;
    packet.transaction.inputSize = sizeof(CreatePoll_input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(input),
        digest,
        32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);
    LOG("CreatePoll transaction sent.\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qutilVote(const char* nodeIp, int nodePort, const char* seed,
    uint64_t poll_id, uint64_t amount, uint64_t chosen_option,
    uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc)
    {
        LOG("Failed to connect to node.\n");
        return;
    }

    GetFees_output fees;
    if (!getFees(qc, fees))
        return;

    Vote_input input;
    memset(&input, 0, sizeof(input));
    input.poll_id = poll_id;
    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    memcpy(input.address, sourcePublicKey, 32);
    input.amount = amount;
    input.chosen_option = chosen_option;

    uint8_t destPublicKey[32] = { 0 };
    ((uint64_t*)destPublicKey)[0] = QUTIL_CONTRACT_ID;
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        Vote_input inputData;
        unsigned char signature[64];
    } packet;
    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = fees.pollVoteFee;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = qutilProcedureId::Vote;
    packet.transaction.inputSize = sizeof(Vote_input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction,
                    sizeof(packet.transaction) + sizeof(input),
                    digest,
                    32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction,
                    sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE,
                    digest,
                    32);
    getTxHashFromDigest(digest, txHash);
    LOG("Vote transaction sent.\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qutilGetCurrentResult(const char* nodeIp, int nodePort, uint64_t poll_id)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc)
    {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct
    {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetCurrentResult_input input;
    } req;
    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QUTIL_CONTRACT_ID;
    req.rcf.inputType = qutilFunctionId::GetCurrentResult;
    req.rcf.inputSize = sizeof(GetCurrentResult_input);
    req.input.poll_id = poll_id;
    req.header.setSize(sizeof(req));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetCurrentResult_output output;
    try
    {
        output = qc->receivePacketWithHeaderAs<GetCurrentResult_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to get current result: %s\n", e.what());
        return;
    }

    if (output.is_active != 0)
    {
        LOG("Poll %" PRIu64 " status is ACTIVE.\n", poll_id);
    }
    else
    {
        LOG("Poll %" PRIu64 " status is INACTIVE.\n", poll_id);
    }

    // Log the votes for each option with votes > 0
    for (int i = 0; i < QUTIL_MAX_OPTIONS; i++)
    {
        if (output.votes[i] > 0)
        {
            LOG("Option %d: %" PRIu64 " votes\n", i, output.votes[i]);
        }
    }
}

void qutilGetPollsByCreator(const char* nodeIp, int nodePort, const char* creator_address)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc)
    {
        LOG("Failed to connect to node.\n");
        return;
    }

    uint8_t creator_pubkey[32];
    getPublicKeyFromIdentity(creator_address, creator_pubkey);

    struct
    {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetPollsByCreator_input input;
    } req;
    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QUTIL_CONTRACT_ID;
    req.rcf.inputType = qutilFunctionId::GetPollsByCreator;
    req.rcf.inputSize = sizeof(GetPollsByCreator_input);
    memcpy(req.input.creator, creator_pubkey, 32);
    req.header.setSize(sizeof(req));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetPollsByCreator_output output;
    try
    {
        output = qc->receivePacketWithHeaderAs<GetPollsByCreator_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to get polls by creator: %s\n", e.what());
        return;
    }

    if (output.num_polls == 0)
    {
        LOG("No polls found for creator %s.\n", creator_address);
        return;
    }

    LOG("Polls created by %s:\n", creator_address);
    for (uint64_t i = 0; i < output.num_polls; i++)
    {
        LOG("Poll ID: %" PRIu64 "\n", output.poll_ids[i]);
    }
}

void qutilGetCurrentPollId(const char* nodeIp, int nodePort) {
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = qutilFunctionId::GetCurrentPollId;
    packet.rcf.contractIndex = QUTIL_CONTRACT_ID;
    qc->sendData((uint8_t*)&packet, packet.header.size());

    GetCurrentPollId_output output;
    try {
        output = qc->receivePacketWithHeaderAs<GetCurrentPollId_output>();
    }
    catch (std::logic_error& e) {
        LOG("Failed to get current poll ID: %s\n", e.what());
        return;
    }

    LOG("Current Poll ID: %" PRIu64 "\n", output.current_poll_id);
    LOG("Active Polls: %" PRIu64 "\n", output.active_count);
    for (uint64_t i = 0; i < output.active_count; i++) {
        LOG("Poll ID: %" PRIu64 "\n", output.active_poll_ids[i]);
    }
}

void qutilGetPollInfo(const char* nodeIp, int nodePort, uint64_t poll_id)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc)
    {
        LOG("Failed to connect to node.\n");
        return;
    }

    GetPollInfo_input input;
    input.poll_id = poll_id;

    struct
    {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetPollInfo_input inputData;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(GetPollInfo_input);
    packet.rcf.inputType = qutilFunctionId::GetPollInfo;
    packet.rcf.contractIndex = QUTIL_CONTRACT_ID;
    memcpy(&packet.inputData, &input, sizeof(input));
    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        GetPollInfo_output output = qc->receivePacketWithHeaderAs<GetPollInfo_output>();
        if (output.found == 0)
        {
            LOG("Poll not found.\n");
        }
        else
        {
            char buf[128] = { 0 };
            LOG("Poll Info:\n");
            char poll_name[33] = { 0 };
            memcpy(poll_name, output.poll_info.poll_name, 32);
            poll_name[32] = '\0';
            LOG("Poll Name: %s\n", poll_name);
            LOG("Poll Type: %" PRIu64 " (%s)\n", output.poll_info.poll_type,
                output.poll_info.poll_type == QUTIL_POLL_TYPE_QUBIC ? "Qubic" : "Asset");
            LOG("Min Amount: %" PRIu64 "\n", output.poll_info.min_amount);
            LOG("Is Active: %" PRIu64 "\n", output.poll_info.is_active);
            memset(buf, 0, 128);
            getIdentityFromPublicKey(output.poll_info.creator, buf, false);
            LOG("Creator: %s\n", buf);
            LOG("Num Assets: %" PRIu64 "\n", output.poll_info.num_assets);
            if (output.poll_info.num_assets > 0)
            {
                LOG("Allowed Assets:\n");
                for (uint64_t i = 0; i < output.poll_info.num_assets; i++)
                {
                    memset(buf, 0, 128);
                    getIdentityFromPublicKey(output.poll_info.allowed_assets[i].issuer, buf, false);
                    std::string assetNameStr = assetNameFromInt64(output.poll_info.allowed_assets[i].assetName);
                    LOG("Asset %" PRIu64 ": Issuer %s, Name %s\n", i, buf, assetNameStr.c_str());
                }
            }
            LOG("GitHub Link: %s\n", output.poll_link);
        }
    }
    catch (std::logic_error& e)
    {
        LOG("Error receiving poll info: %s\n", e.what());
    }
}

void qutilCancelPoll(const char* nodeIp, int nodePort, const char* seed, uint64_t poll_id, uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) 
    {
        LOG("Failed to connect to node.\n");
        return;
    }

    GetFees_output fees;
    if (!getFees(qc, fees))
        return;

    CancelPoll_input input;
    input.poll_id = poll_id;
    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    uint8_t destPublicKey[32] = { 0 };
    ((uint64_t*)destPublicKey)[0] = QUTIL_CONTRACT_ID;
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        CancelPoll_input inputData;
        unsigned char signature[64];
    } packet;
    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = fees.pollCreationFee;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = qutilProcedureId::CancelPoll;
    packet.transaction.inputSize = sizeof(CancelPoll_input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction,
                    sizeof(packet.transaction) + sizeof(CancelPoll_input),
                    digest,
                    32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction,
                    sizeof(packet.transaction) + sizeof(CancelPoll_input) + SIGNATURE_SIZE,
                    digest,
                    32);
    getTxHashFromDigest(digest, txHash);
    LOG("CancelPoll transaction sent.\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}
