#include "qbond.h"
#include "structs.h"
#include "logger.h"
#include "connection.h"
#include "wallet_utils.h"
#include "node_utils.h"
#include "key_utils.h"
#include "k12_and_key_utils.h"

#define QBOND_CONTRACT_INDEX 17

#define QBOND_STAKE 1
#define QBOND_TRANSFER 2
#define QBOND_ADD_ASK_ORDER 3
#define QBOND_REMOVE_ASK_ORDER 4
#define QBOND_ADD_BID_ORDER 5
#define QBOND_REMOVE_BID_ORDER 6
#define QBOND_BURN 7
#define QBOND_UPDATE_CFA 8

#define QBOND_GET_FEES 1
#define QBOND_GET_EARNED_FEES 2
#define QBOND_GET_INFO_PER_EPOCH 3
#define QBOND_GET_ORDERS 4
#define QBOND_GET_USER_ORDERS 5
#define QBOND_GET_TABLE 6
#define QBOND_GET_USER_MBONDS 7
#define QBOND_GET_CFA 8

constexpr int64_t QBOND_BASE_STAKE_AMOUNT = 1000000ULL;
constexpr uint64_t QBOND_STAKE_FEE = 50; // 0.5%
constexpr auto NULL_ID = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB";

void convertToString(int64_t num, char num_S[])
{
    int64_t tmp = num;
    int32_t t = 0, i = 0;

    if (num == 0)
    {
        num_S[0] = '0';
        return;
    }

    while (tmp)
    {
        num_S[i++] = (tmp % 10) + '0';
        tmp /= 10;
        t++;
        if (t % 3 == 0 && tmp != 0) 
        {
            num_S[i++] = ',';
        }
    }

    for (size_t r = 0, j = i - 1; r < j; ++r, --j) 
    {
        std::swap(num_S[r], num_S[j]);
    }
}

void qbondStake(const char* nodeIp, int nodePort, const char* seed, const int64_t millionsOfQu, const uint32_t scheduledTickOffset)
{
    Stake_input input;
    input.millions = millionsOfQu;

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
    ((uint64_t*) destPublicKey)[0] = QBOND_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        Stake_input inputData;
        uint8_t sig[64];
    } packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = millionsOfQu * QBOND_BASE_STAKE_AMOUNT + millionsOfQu * QBOND_BASE_STAKE_AMOUNT / 10000ULL * QBOND_STAKE_FEE;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QBOND_STAKE;
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
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qbondTransfer(const char* nodeIp, int nodePort, const char* seed, const char* targetIdentity, const int64_t epoch, const int64_t mbondsAmount, const uint32_t scheduledTickOffset)
{
    TransferMBond_input input;
    input.epoch = epoch;
    input.numberOfMBonds = mbondsAmount;
    memset(input.newOwnerAndPossessor, 0, 32);
    getPublicKeyFromIdentity(targetIdentity, input.newOwnerAndPossessor);

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
    ((uint64_t*) destPublicKey)[0] = QBOND_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        TransferMBond_input inputData;
        uint8_t sig[64];
    } packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 100;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QBOND_TRANSFER;
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
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qbondAddAskOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const uint32_t scheduledTickOffset)
{
    qbondOperateOrder(nodeIp, nodePort, seed, epoch, mbondPrice, mbondsAmount, 1, QBOND_ADD_ASK_ORDER, scheduledTickOffset);
}

void qbondRemoveAskOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const uint32_t scheduledTickOffset)
{
    qbondOperateOrder(nodeIp, nodePort, seed, epoch, mbondPrice, mbondsAmount, 1, QBOND_REMOVE_ASK_ORDER, scheduledTickOffset);
}

void qbondAddBidOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const uint32_t scheduledTickOffset)
{
    qbondOperateOrder(nodeIp, nodePort, seed, epoch, mbondPrice, mbondsAmount, mbondPrice * mbondsAmount, QBOND_ADD_BID_ORDER, scheduledTickOffset);
}

void qbondRemoveBidOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const uint32_t scheduledTickOffset)
{
    qbondOperateOrder(nodeIp, nodePort, seed, epoch, mbondPrice, mbondsAmount, 1, QBOND_REMOVE_BID_ORDER, scheduledTickOffset);
}

void qbondOperateOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const int64_t fee, const uint16_t inputType, const uint32_t scheduledTickOffset)
{
    OrderOperation_input input;
    input.epoch = epoch;
    input.price = mbondPrice;
    input.numberOfMBonds = mbondsAmount;

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
    ((uint64_t*) destPublicKey)[0] = QBOND_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        OrderOperation_input inputData;
        uint8_t sig[64];
    } packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = fee;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
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
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qbondBurn(const char* nodeIp, int nodePort, const char* seed, const int64_t burnAmount, const uint32_t scheduledTickOffset)
{
    Burn_input input;
    input.amount = burnAmount;

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
    ((uint64_t*) destPublicKey)[0] = QBOND_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        Burn_input inputData;
        uint8_t sig[64];
    } packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = burnAmount;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QBOND_BURN;
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
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qbondUpdateCFA(const char* nodeIp, int nodePort, const char* seed, const char* user, const bool operation, const uint32_t scheduledTickOffset)
{
    UpdateCFA_input input;
    memset(input.user, 0, 32);
    getPublicKeyFromIdentity(user, input.user);
    input.operation = operation;

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
    ((uint64_t*) destPublicKey)[0] = QBOND_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        UpdateCFA_input inputData;
        uint8_t sig[64];
    } packet;

    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QBOND_UPDATE_CFA;
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
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qbondGetFees(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetFees_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QBOND_CONTRACT_INDEX;
    req.rcf.inputType = QBOND_GET_FEES;
    req.rcf.inputSize = sizeof(req.in);
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(req.in));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetFees_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<GetFees_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get fees.\n");
        return;
    }

    LOG("Stake fee percent: %.1f %%\n", double(output.stakeFeePercent) / 100.);
    LOG("Trade fee percent: %.2f %%\n", double(output.tradeFeePercent) / 100.);
    LOG("Transfer fee: %llu QU\n", output.transferFee);
}

void qbondGetEarnedFees(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetFees_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QBOND_CONTRACT_INDEX;
    req.rcf.inputType = QBOND_GET_EARNED_FEES;
    req.rcf.inputSize = sizeof(req.in);
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(req.in));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetEarnedFees_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<GetEarnedFees_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get fees.\n");
        return;
    }

    LOG("Stake fees: %llu QU\n", output.stakeFees);
    LOG("Trade fees: %llu QU\n", output.tradeFees);
    LOG("Total fees: %llu QU\n", output.stakeFees + output.tradeFees);
}

void qbondGetInfoPerEpoch(const char* nodeIp, int nodePort, const int64_t epoch)
{
    GetInfoPerEpoch_input input;
    input.epoch = epoch;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetInfoPerEpoch_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QBOND_CONTRACT_INDEX;
    req.rcf.inputType = QBOND_GET_INFO_PER_EPOCH;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetInfoPerEpoch_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<GetInfoPerEpoch_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get deals.\n");
        return;
    }

    char total[100] = {0,};
    char revenue[100] = {0,};
    convertToString(output.totalStaked * QBOND_BASE_STAKE_AMOUNT, total);
    convertToString((int64_t) (QBOND_BASE_STAKE_AMOUNT * (1.0 + double(output.apy) / 10000000.0)), revenue);
    LOG("Stats for %d epoch:\n   Stakers: %llu\n   Total staked: %s QU\n   Estimated revenue per MBond: %s QU\n   APY: %.2f %%\n", epoch, output.stakersAmount, total, revenue, double(output.apy) / 100000.0);
}

void qbondGetOrders(const char* nodeIp, int nodePort, const int64_t epoch, const int64_t asksOffset, const int64_t bidsOffset)
{
    GetOrders_input input;
    input.epoch = epoch;
    input.asksOffset = asksOffset;
    input.bidsOffset = bidsOffset;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetOrders_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QBOND_CONTRACT_INDEX;
    req.rcf.inputType = QBOND_GET_ORDERS;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetOrders_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<GetOrders_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get orders.\n");
        return;
    }

    printOrders("ASK Orders", output.askOrders);
    printOrders("BID Orders", output.bidOrders);
}

void qbondGetUserOrders(const char* nodeIp, int nodePort, const char* owner, const int64_t asksOffset, const int64_t bidsOffset)
{
    GetUserOrders_input input;
    memset(input.owner, 0, 32);
    getPublicKeyFromIdentity(owner, input.owner);
    input.asksOffset = asksOffset;
    input.bidsOffset = bidsOffset;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetUserOrders_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QBOND_CONTRACT_INDEX;
    req.rcf.inputType = QBOND_GET_USER_ORDERS;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetUserOrders_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<GetUserOrders_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get orders.\n");
        return;
    }

    printOrders("ASK Orders", output.askOrders);
    printOrders("BID Orders", output.bidOrders);
}

void qbondGetTable(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        MBondsTable_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QBOND_CONTRACT_INDEX;
    req.rcf.inputType = QBOND_GET_TABLE;
    req.rcf.inputSize = sizeof(req.in);
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(req.in));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    MBondsTable_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<MBondsTable_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get MBonds table.\n");
        return;
    }

    LOG("%-9s%-20s%-20s%-19s%s\n", "MBond", "Total staked QBond", "Total staked QEarn", "Estimated revenue", "APY");
    for (int i = 0; i < 512; i++)
    {
        if (output.entries[i].epoch == 0)
        {
            LOG("\n **APY of the current epoch is not final\n");
            break;
        }
        char revenue[100] = {0,};
        convertToString((int64_t) (QBOND_BASE_STAKE_AMOUNT * (1.0 + double(output.entries[i].apy) / 10000000.0)), revenue);
        LOG("MBND%-5lld%-20lld%-20lld%-19s%.2f %%\n", output.entries[i].epoch, output.entries[i].totalStakedQBond, output.entries[i].totalStakedQEarn, revenue, double(output.entries[i].apy) / 100000.0);
    }
}

void qbondGetUserMBonds(const char* nodeIp, int nodePort, const char* owner)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetUserMBonds_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QBOND_CONTRACT_INDEX;
    req.rcf.inputType = QBOND_GET_USER_MBONDS;
    req.rcf.inputSize = sizeof(req.in);
    memset(req.in.owner, 0, 32);
    getPublicKeyFromIdentity(owner, req.in.owner);
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(req.in));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetUserMBonds_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<GetUserMBonds_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get user MBonds.\n");
        return;
    }

    LOG("Total staked by user: %lld millions of QU\n", output.totalMBondsAmount);
    LOG("MBonds owned by %s:\n\n%-9s%-11s%s\n", owner, "MBond", "Amount", "APY");
    for (int i = 0; i < 256; i++)
    {
        if (output.mbonds[i].epoch == 0)
        {
            LOG("\n **APY of the current epoch is not final\n");
            break;
        }
        LOG("MBND%-5lld%-11lld%.2f %%\n", output.mbonds[i].epoch, output.mbonds[i].amount, double(output.mbonds[i].apy) / 100000.0);
    }
}

void qbondGetCFA(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetCFA_input in;
    } req;

    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = QBOND_CONTRACT_INDEX;
    req.rcf.inputType = QBOND_GET_CFA;
    req.rcf.inputSize = sizeof(req.in);
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(req.in));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    GetCFA_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<GetCFA_output>();
    }
    catch (std::logic_error) {
        LOG("Failed to get user MBonds.\n");
        return;
    }

    LOG("Commission free addresses:\n");
    for (int i = 0; i < 1024; i++)
    {
        char iden[61];
        memset(iden, 0, 61);
        getIdentityFromPublicKey(output.cfa[i], iden, false);
        if (strcmp(iden, NULL_ID) == 0)
        {
            break;
        }
        LOG("%d. %s\n", i + 1, iden);
    }
}

void printOrders(const char* ordersType, const Order orders[])
{
    LOG("%s\n%-62s%-13s%-13s%-8s%s\n", ordersType, "Owner", "MBond name", "Price", "Amount", "Total");
    for (int i = 0; i < 256; i++)
    {
        if (!isZeroPubkey(orders[i].owner))
        {
            char iden[61];
            memset(iden, 0, 61);
            getIdentityFromPublicKey(orders[i].owner, iden, false);
            char price[100] = {0,};
            char total[100] = {0,};
            convertToString(orders[i].price, price);
            convertToString(orders[i].price * orders[i].numberOfMBonds, total);
            LOG("%-62sMBND%-9lld%-13s%-8lld%s\n", iden, orders[i].epoch, price, orders[i].numberOfMBonds, total);
        }
        else
        {
            LOG("\n");
            break;
        }
    }
}
