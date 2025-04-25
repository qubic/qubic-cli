#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "structs.h"
#include "walletUtils.h"
#include "keyUtils.h"
#include "assetUtil.h"
#include "connection.h"
#include "logger.h"
#include "nodeUtils.h"
#include "K12AndKeyUtil.h"
#include "qswap.h"
#include "qswapStruct.h"

#define QSWAP_CONTRACT_INDEX 13
#define QSWAP_ADDRESS "NAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMAML"

// QSWAP FUNCTIONS
#define QSWAP_GET_FEE 1
#define QSWAP_GET_POOL_BASIC_STATE 2
#define QSWAP_GET_LIQUDITY_OF 3
#define QSWAP_QUOTE_EXACT_QU_INPUT 4
#define QSWAP_QUOTE_EXACT_QU_OUTPUT 5
#define QSWAP_QUOTE_EXACT_ASSET_INPUT 6
#define QSWAP_QUOTE_EXACT_ASSET_OUTPUT 7

// QSWAP PROCEDURES
#define QSWAP_ISSUE_ASSET 1
#define QSWAP_TRANSFER_SHARE 2

#define QSWAP_CREATE_POOL 3
#define QSWAP_ADD_LIQUDITY 4
#define QSWAP_REMOVE_LIQUDITY 5

#define QSWAP_SWAP_EXACT_QU_FOR_ASSET 6
#define QSWAP_SWAP_QU_FOR_EXACT_ASSET 7
#define QSWAP_SWAP_EXACT_ASSET_FOR_QU 8
#define QSWAP_SWAP_ASSET_FOR_EXACT_QU 9

void getQswapFees(const char* nodeIp, const int nodePort, QswapFees_output& result)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = QSWAP_GET_FEE;
    packet.rcf.contractIndex = QSWAP_CONTRACT_INDEX;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<QswapFees_output>();
    }
    catch (std::logic_error& e)
    {
        memset(&result, 0, sizeof(result));
    }
}

void printQswapFee(const char* nodeIp, const int nodePort)
{
    QswapFees_output result;
    getQswapFees(nodeIp, nodePort, result);
    LOG("Asset issuance fee: %u\n", result.assetIssuanceFee);
    LOG("Pool creation fee: %u\n", result.poolCreationFee);
    LOG("Transfer fee: %u\n", result.transferFee);
    LOG("Swap rate: %u / 10000 \n", result.swapRate);
    LOG("Protocol rate: %u / 100 \n", result.protocolRate);
}

void qswapIssueAsset(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* assetName,
                     const char* unitOfMeasurement,
                     int64_t numberOfUnits,
                     char numberOfDecimalPlaces,
                     uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    char assetNameS1[8] = {0};
    char UoMS1[8] = {0};
    memcpy(assetNameS1, assetName, strlen(assetName));
    for (int i = 0; i < 7; i++) UoMS1[i] = unitOfMeasurement[i] - 48;
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(QSWAP_ADDRESS, destPublicKey);

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        QswapIssueAsset_input ia;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000000;
    uint32_t scheduledTick = 0;
    if (scheduledTickOffset < 50000)
    {
        uint32_t currentTick = getTickNumberFromNode(qc);
        scheduledTick = currentTick + scheduledTickOffset;
    }
    else
    {
        scheduledTick = scheduledTickOffset;
    }
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QSWAP_ISSUE_ASSET;
    packet.transaction.inputSize = sizeof(QswapIssueAsset_input);

    // fill the input
    memcpy(&packet.ia.name, assetNameS1, 8);
    memcpy(&packet.ia.unitOfMeasurement, UoMS1, 8);
    packet.ia.numberOfUnits = numberOfUnits;
    packet.ia.numberOfDecimalPlaces = numberOfDecimalPlaces;
    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(QswapIssueAsset_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(QswapIssueAsset_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(QswapIssueAsset_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ia));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qswapTransferAsset(const char* nodeIp, int nodePort,
                        const char* seed,
                        const char* pAssetName,
                        const char* pIssuerInQubicFormat,
                        const char* newOwnerIdentity,
                        long long numberOfUnits,
                        uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    uint8_t issuer[32] = {0};
    uint8_t newOwnerPublicKey[32] = {0};
    char txHash[128] = {0};
    char assetNameU1[8] = {0};

    memcpy(assetNameU1, pAssetName, strlen(pAssetName));
    if (strlen(pIssuerInQubicFormat) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pIssuerInQubicFormat, issuer);

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(QSWAP_ADDRESS, destPublicKey);
    getPublicKeyFromIdentity(newOwnerIdentity, newOwnerPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        QswapTransferAssetOwnershipAndPossession_input ta;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000;
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QSWAP_TRANSFER_SHARE;
    packet.transaction.inputSize = sizeof(QswapTransferAssetOwnershipAndPossession_input);

    // fill the input
    memcpy(&packet.ta.assetName, assetNameU1, 8);
    memcpy(packet.ta.issuer, issuer, 32);
    memcpy(packet.ta.newOwnerAndPossessor, newOwnerPublicKey, 32);
    packet.ta.numberOfUnits = numberOfUnits;
    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(QswapTransferAssetOwnershipAndPossession_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(QswapTransferAssetOwnershipAndPossession_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(QswapTransferAssetOwnershipAndPossession_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ta));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qswapCreatePool(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* pAssetName,
                     const char* pIssuerInQubicFormat,
                     uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    uint8_t issuer[32] = {0};
    char txHash[128] = {0};
    char assetNameS1[8] = {0};

    memcpy(assetNameS1, pAssetName, strlen(pAssetName));
    if (strlen(pIssuerInQubicFormat) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pIssuerInQubicFormat, issuer);

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(QSWAP_ADDRESS, destPublicKey);

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        CreatePool_input cp;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000000;
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QSWAP_CREATE_POOL;
    packet.transaction.inputSize = sizeof(CreatePool_input);

    // DEBUG LOG
    LOG("\n-------------------------------------\n\n");
    LOG("Sending QSWAP - CreatePool\n");
    LOG("Issuer: %s\n", pIssuerInQubicFormat);
    LOG("assetName: %s\n", assetNameS1);
    LOG("\n-------------------------------------\n\n");

    // fill the input
    memcpy(&packet.cp.assetName, assetNameS1, 8);
    memcpy(packet.cp.issuer, issuer, 32);

    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(CreatePool_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(CreatePool_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(CreatePool_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.cp));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qswapAddLiqudity(const char* nodeIp, int nodePort,
                      const char* seed,
                      const char* pAssetName,
                      const char* pIssuerInQubicFormat,
                      int64_t quAmountDesired,
                      int64_t assetAmountDesired,
                      int64_t quAmountMin,
                      int64_t assetAmountMin,
                      uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    uint8_t issuer[32] = {0};
    char txHash[128] = {0};
    char assetNameS1[8] = {0};

    memcpy(assetNameS1, pAssetName, strlen(pAssetName));
    if (strlen(pIssuerInQubicFormat) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pIssuerInQubicFormat, issuer);

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(QSWAP_ADDRESS, destPublicKey);

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        AddLiqudity_input al;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = quAmountDesired;
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QSWAP_ADD_LIQUDITY;
    packet.transaction.inputSize = sizeof(AddLiqudity_input);

    // DEBUG LOG
    LOG("\n-------------------------------------\n\n");
    LOG("Sending QSWAP - AddLiqudity\n");
    LOG("Issuer: %s\n", pIssuerInQubicFormat);
    LOG("assetName: %s\n", assetNameS1);
    LOG("quAmountDesired: %lld\n", quAmountDesired);
    LOG("amountDesired: %lld\n", assetAmountDesired);
    LOG("quAmountMin: %lld\n", quAmountMin);
    LOG("assetAmountMin: %lld\n", assetAmountMin);
    LOG("\n-------------------------------------\n\n");

    // fill the input
    memcpy(&packet.al.assetName, assetNameS1, 8);
    memcpy(packet.al.issuer, issuer, 32);
    packet.al.assetAmountDesired = assetAmountDesired;
    packet.al.quAmountMin = quAmountMin;
    packet.al.assetAmountMin = assetAmountMin;

    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(AddLiqudity_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(AddLiqudity_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(AddLiqudity_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.al));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qswapRemoveLiqudity(const char* nodeIp, int nodePort,
                      const char* seed,
                      const char* pAssetName,
                      const char* pIssuerInQubicFormat,
                      int64_t burnLiqudity,
                      int64_t quAmountMin,
                      int64_t assetAmountMin,
                      uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    uint8_t issuer[32] = {0};
    char txHash[128] = {0};
    char assetNameS1[8] = {0};

    memcpy(assetNameS1, pAssetName, strlen(pAssetName));
    if (strlen(pIssuerInQubicFormat) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pIssuerInQubicFormat, issuer);

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(QSWAP_ADDRESS, destPublicKey);

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        RemoveLiqudity_input rl;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 0;
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QSWAP_REMOVE_LIQUDITY;
    packet.transaction.inputSize = sizeof(RemoveLiqudity_input);

    // DEBUG LOG
    LOG("\n-------------------------------------\n\n");
    LOG("Sending QSWAP - RemoveLiqudity\n");
    LOG("Issuer: %s\n", pIssuerInQubicFormat);
    LOG("assetName: %s\n", assetNameS1);
    LOG("burnLiqudity: %lld\n", burnLiqudity);
    LOG("quAmountMin: %lld\n", quAmountMin);
    LOG("assetAmountMin: %lld\n", assetAmountMin);
    LOG("\n-------------------------------------\n\n");

    // fill the input
    memcpy(&packet.rl.assetName, assetNameS1, 8);
    memcpy(packet.rl.issuer, issuer, 32);
    packet.rl.burnLiqudity = burnLiqudity;
    packet.rl.quAmountMin = quAmountMin;
    packet.rl.assetAmountMin = assetAmountMin;

    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(RemoveLiqudity_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(RemoveLiqudity_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(RemoveLiqudity_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.rl));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

template <int procedureNumber>
void qswapSwapQuForAssetAction(const char* nodeIp, int nodePort,
                               const char* seed,
                               const char* pAssetName,
                               const char* pIssuerInQubicFormat,
                               int64_t quAmountIn,
                               int64_t assetAmountOut,
                               uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    uint8_t issuer[32] = {0};
    char txHash[128] = {0};
    char assetNameU1[8] = {0};
    memcpy(assetNameU1, pAssetName, strlen(pAssetName));
    if (strlen(pIssuerInQubicFormat) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pIssuerInQubicFormat, issuer);

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(QSWAP_ADDRESS, destPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SwapQuForAssetAction_input sqfa;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = quAmountIn;

    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = procedureNumber;
    packet.transaction.inputSize = sizeof(SwapQuForAssetAction_input);

    // DEBUG LOG
    LOG("\n-------------------------------------\n\n");
    if (procedureNumber == QSWAP_SWAP_EXACT_QU_FOR_ASSET){
        LOG("Sending QSWAP swapExactQuForAsset action - procedureNumber: %d\n", procedureNumber);
        LOG("Issuer: %s\n", pIssuerInQubicFormat);
        LOG("assetName: %s\n", assetNameU1);
        LOG("qu amount in: %d\n", quAmountIn);
        LOG("asset amount out min: %lld\n", assetAmountOut);
        LOG("Sending QSWAP swapExactQuForAsset action - procedureNumber: %d\n", procedureNumber);
    } else if (procedureNumber == QSWAP_SWAP_QU_FOR_EXACT_ASSET ){
        LOG("Sending QSWAP swapQuForExactAsset action - procedureNumber: %d\n", procedureNumber);
        LOG("Issuer: %s\n", pIssuerInQubicFormat);
        LOG("assetName: %s\n", assetNameU1);
        LOG("qu amount in max: %d\n", quAmountIn);
        LOG("asset amount out: %lld\n", assetAmountOut);
        LOG("Sending QSWAP swapQuForExactAsset action - procedureNumber: %d\n", procedureNumber);
    }
    LOG("\n-------------------------------------\n\n");

    // fill the input
    memcpy(&packet.sqfa.assetName, assetNameU1, 8);
    memcpy(packet.sqfa.issuer, issuer, 32);
    packet.sqfa.assetAmountOut = assetAmountOut;
    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapQuForAssetAction_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(SwapQuForAssetAction_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(SwapQuForAssetAction_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.sqfa));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qswapSwapExactQuForAsset(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* pAssetName,
                              const char* pHexIssuer,
                              int64_t quAmountIn,
                              int64_t assetAmountOutMin,
                              uint32_t scheduledTickOffset)
{
    qswapSwapQuForAssetAction<QSWAP_SWAP_EXACT_QU_FOR_ASSET>(
        nodeIp,
        nodePort,
        seed,
        pAssetName,
        pHexIssuer,
        quAmountIn,
        assetAmountOutMin,
        scheduledTickOffset
    );
}


void qswapSwapQuForExactAsset(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* pAssetName,
                              const char* pHexIssuer,
                              int64_t quAmountInMax,
                              int64_t assetAmountOut,
                              uint32_t scheduledTickOffset)
{
    qswapSwapQuForAssetAction<QSWAP_SWAP_QU_FOR_EXACT_ASSET>(
        nodeIp,
        nodePort,
        seed,
        pAssetName,
        pHexIssuer,
        quAmountInMax,
        assetAmountOut,
        scheduledTickOffset
    );
}

template <int procedureNumber>
void qswapSwapAssetForQuAction(const char* nodeIp, int nodePort,
                               const char* seed,
                               const char* pAssetName,
                               const char* pIssuerInQubicFormat,
                               int64_t assetAmountIn,
                               int64_t quAmountOut,
                               uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    uint8_t issuer[32] = {0};
    char txHash[128] = {0};
    char assetNameU1[8] = {0};
    memcpy(assetNameU1, pAssetName, strlen(pAssetName));
    if (strlen(pIssuerInQubicFormat) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pIssuerInQubicFormat, issuer);

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(QSWAP_ADDRESS, destPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SwapAssetForQuAction_input safq;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1; // free

    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = procedureNumber;
    packet.transaction.inputSize = sizeof(SwapAssetForQuAction_input);

    // DEBUG LOG
    LOG("\n-------------------------------------\n\n");
    // LOG("Sending Qswap action - functionNumber: %d\n", functionNumber);
    if (procedureNumber == QSWAP_SWAP_EXACT_ASSET_FOR_QU){
        LOG("Sending qswap swapExactAssetForQu action - procedureNumber: %d\n", procedureNumber);
        LOG("Issuer: %s\n", pIssuerInQubicFormat);
        LOG("assetName: %s\n", assetNameU1);
        LOG("assetAmountIn: %lld\n", assetAmountIn);
        LOG("quAmountOutMin : %lld\n", quAmountOut);
    } else if (procedureNumber == QSWAP_SWAP_ASSET_FOR_EXACT_QU) {
        LOG("Sending qswap swapAssetForExactQu action - procedureNumber: %d\n", procedureNumber);
        LOG("Issuer: %s\n", pIssuerInQubicFormat);
        LOG("assetName: %s\n", assetNameU1);
        LOG("assetAmountInMax: %lld\n", assetAmountIn);
        LOG("quAmountOut : %lld\n", quAmountOut);
    }
    LOG("\n-------------------------------------\n\n");

    // fill the input
    memcpy(&packet.safq.assetName, assetNameU1, 8);
    memcpy(packet.safq.issuer, issuer, 32);
    packet.safq.assetAmountIn = assetAmountIn;
    packet.safq.quAmountOut = quAmountOut;
    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapAssetForQuAction_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(SwapAssetForQuAction_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(SwapAssetForQuAction_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.safq));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qswapSwapExactAssetForQu(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* pAssetName,
                              const char* pHexIssuer,
                              int64_t assetAmountIn,
                              int64_t quAmountOutMin,
                              uint32_t scheduledTickOffset)
{
    qswapSwapAssetForQuAction<QSWAP_SWAP_EXACT_ASSET_FOR_QU>(nodeIp, nodePort, seed, pAssetName, pHexIssuer, assetAmountIn, quAmountOutMin, scheduledTickOffset);
}

void qswapSwapAssetForExactQu(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* pAssetName,
                              const char* pHexIssuer,
                              int64_t assetAmountInMax,
                              int64_t quAmountOut,
                              uint32_t scheduledTickOffset)
{
    qswapSwapAssetForQuAction<QSWAP_SWAP_ASSET_FOR_EXACT_QU>(nodeIp, nodePort, seed, pAssetName, pHexIssuer, assetAmountInMax, quAmountOut, scheduledTickOffset);
}


void qswapGetPoolBasicState(const char* nodeIp, int nodePort,
                            const char* pAssetName,
                            const char* pHexIssuer)
{
    uint8_t issuer[32] = {0};
    char assetNameU1[8] = {0};
    memcpy(assetNameU1, pAssetName, strlen(pAssetName));
    if (strlen(pHexIssuer) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pHexIssuer, issuer);

    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        qswapGetPoolBasicState_input gpbs;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(qswapGetPoolBasicState_input);
    packet.rcf.inputType = QSWAP_GET_POOL_BASIC_STATE;
    packet.rcf.contractIndex = QSWAP_CONTRACT_INDEX;
    memcpy(packet.gpbs.issuer, issuer, 32);
    memcpy(&packet.gpbs.assetName, assetNameU1, 8);

    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        qswapGetPoolBasicState_output output = qc->receivePacketWithHeaderAs<qswapGetPoolBasicState_output>();
        if (output.poolExists) {
            LOG("GetPoolBasicState reserveQu: %u, reserveAsset: %u, liqudity: %u\n", output.reservedQuAmount, output.reservedAssetAmount, output.totalLiqudity);
        } else {
            LOG("GetPoolBasicState pool not exist\n");
        }
    }
    catch (std::logic_error& e) {}
}

void qswapGetLiqudityOf(const char* nodeIp, int nodePort,
                        const char* pAssetName,
                        const char* pHexIssuer,
                        const char* pHexAccount)
{
    uint8_t issuer[32] = {0};
    uint8_t account[32] = {0};
    char assetNameU1[8] = {0};
    memcpy(assetNameU1, pAssetName, strlen(pAssetName));
    if (strlen(pHexIssuer) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pHexIssuer, issuer);
    getPublicKeyFromIdentity(pHexAccount, account);

    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        qswapGetLiqudityOf_input glo;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(qswapGetLiqudityOf_input);
    packet.rcf.inputType = QSWAP_GET_LIQUDITY_OF;
    packet.rcf.contractIndex = QSWAP_CONTRACT_INDEX;
    memcpy(packet.glo.issuer, issuer, 32);
    memcpy(&packet.glo.assetName, assetNameU1, 8);
    memcpy(&packet.glo.account, account, 32);

    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        qswapGetLiqudityOf_output output = qc->receivePacketWithHeaderAs<qswapGetLiqudityOf_output>();
        LOG("GetLiqudityOf result amount: %u\n", output.liqudity);
    }
    catch (std::logic_error& e) {}
}

template <int functionNumber>
void qswapQuoteAction(const char* nodeIp, int nodePort,
                               const char* pAssetName,
                               const char* pIssuerInQubicFormat,
                               int64_t amount)
{
    uint8_t issuer[32] = {0};
    char assetNameU1[8] = {0};
    memcpy(assetNameU1, pAssetName, strlen(pAssetName));
    if (strlen(pIssuerInQubicFormat) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pIssuerInQubicFormat, issuer);

    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        qswapQuote_input q;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(qswapQuote_input);
    packet.rcf.inputType = functionNumber;
    packet.rcf.contractIndex = QSWAP_CONTRACT_INDEX;
    memcpy(packet.q.issuer, issuer, 32);
    memcpy(&packet.q.assetName, assetNameU1, 8);
    packet.q.amount = amount;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        qswapQuote_output output = qc->receivePacketWithHeaderAs<qswapQuote_output>();
        LOG("Quote result amount: %u\n", output.amount);
    }
    catch (std::logic_error& e) {}
}

void qswapQuoteExactQuInput(const char* nodeIp, int nodePort,
                            const char* pAssetName,
                            const char* pHexIssuer,
                            int64_t quAmountIn)
{
    qswapQuoteAction<QSWAP_QUOTE_EXACT_QU_INPUT>(nodeIp, nodePort, pAssetName, pHexIssuer, quAmountIn);
}

void qswapQuoteExactQuOutput(const char* nodeIp, int nodePort,
                            const char* pAssetName,
                            const char* pHexIssuer,
                            int64_t quAmountOut)
{
    qswapQuoteAction<QSWAP_QUOTE_EXACT_QU_OUTPUT>(nodeIp, nodePort, pAssetName, pHexIssuer, quAmountOut);
}

void qswapQuoteExactAssetOutput(const char* nodeIp, int nodePort,
                               const char* pAssetName,
                               const char* pHexIssuer,
                               int64_t assetAmountOut)
{
    qswapQuoteAction<QSWAP_QUOTE_EXACT_ASSET_OUTPUT>(nodeIp, nodePort, pAssetName, pHexIssuer, assetAmountOut);
}

void qswapQuoteExactAssetInput(const char* nodeIp, int nodePort,
                               const char* pAssetName,
                               const char* pHexIssuer,
                               int64_t assetAmountIn)
{
    qswapQuoteAction<QSWAP_QUOTE_EXACT_ASSET_INPUT>(nodeIp, nodePort, pAssetName, pHexIssuer, assetAmountIn);
}
