#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstring>

#include "qutil.h"
#include "keyUtils.h"
#include "structs.h"
#include "defines.h"
#include "logger.h"
#include "nodeUtils.h"
#include "K12AndKeyUtil.h"
#include "connection.h"
#include "walletUtils.h"

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
struct BurnQubic_output
{
    long long amount;
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
    const char* github_link, const char* comma_separated_asset_names,
    const char* comma_separated_asset_issuers, uint32_t scheduledTickOffset) 
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc)
    {
        LOG("Failed to connect to node.\n");
        return;
    }

    std::vector<std::string> asset_names = split(comma_separated_asset_names, ',');
    std::vector<std::string> asset_issuers = split(comma_separated_asset_issuers, ',');
    if (asset_names.size() != asset_issuers.size()) 
    {
        LOG("Mismatch between number of asset names and issuers.\n");
        return;
    }
    if (asset_names.size() > QUTIL_MAX_ASSETS_PER_POLL) 
    {
        LOG("Too many assets specified. Maximum is %llu.\n", QUTIL_MAX_ASSETS_PER_POLL);
        return;
    }

    CreatePoll_input input;
    memset(&input, 0, sizeof(input));
    strncpy((char*)input.poll_name, poll_name, 32);
    input.poll_type = poll_type;
    input.min_amount = min_amount;
    strncpy((char*)input.github_link, github_link, 256);
    input.num_assets = asset_names.size();
    for (size_t i = 0; i < asset_names.size(); ++i) 
    {
        qpi::Asset& asset = input.allowed_assets[i];
        memset(&asset, 0, sizeof(qpi::Asset));
        getPublicKeyFromIdentity(asset_issuers[i].c_str(), asset.issuer);
        char assetNameBuf[8] = { 0 };
        strncpy(assetNameBuf, asset_names[i].c_str(), 8);
        memcpy(&asset.assetName, assetNameBuf, sizeof(uint64_t));
    }

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
    packet.transaction.amount = QUTIL_POLL_CREATION_FEE;
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
    packet.transaction.amount = amount + QUTIL_VOTE_FEE;
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

    if (output.poll_id == 0) 
    {
        LOG("Poll not found or invalid poll ID.\n");
        return;
    }

    LOG("Poll ID: %llu\n", output.poll_id);
    for (int i = 0; i < QUTIL_MAX_OPTIONS; i++) 
    {
        if (output.votes[i] > 0) {
            LOG("Option %d: %llu votes\n", i, output.votes[i]);
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
        LOG("Poll ID: %llu\n", output.poll_ids[i]);
    }
}
