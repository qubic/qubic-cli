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

enum qutilFunctionId
{
    GetSendToManyV1Fee = 1,
};

enum qutilProcedureId
{
    SendToManyV1 = 1,
    BurnQubic = 2,
    SendToManyBenchmark = 3,
};

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