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
#include "qx.h"
#include "qxStruct.h"

#define QX_CONTRACT_INDEX 1
#define QX_ADDRESS "BAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARMID"

// QX FUNCTIONS
#define QX_GET_FEE 1
#define QX_GET_ASSET_ASK_ORDER 2
#define QX_GET_ASSET_BID_ORDER 3
#define QX_GET_ENTITY_ASK_ORDER 4
#define QX_GET_ENTITY_BID_ORDER 5

// QX PROCEDURES
#define QX_ISSUE_ASSET 1
#define QX_TRANSFER_SHARE 2
#define QX_PLACEHOLDER0 3
#define QX_PLACEHOLDER1 4
#define QX_ADD_ASK_ORDER 5
#define QX_ADD_BID_ORDER 6
#define QX_REMOVE_ASK_ORDER 7
#define QX_REMOVE_BID_ORDER 8
#define QX_TRANSFER_SHARE_MANAGEMENT_RIGHTS 9

void getQxFees(const char* nodeIp, const int nodePort, QxFees_output& result)
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
    packet.rcf.inputType = QX_GET_FEE;
    packet.rcf.contractIndex = QX_CONTRACT_INDEX;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<QxFees_output>();
    }
    catch (std::logic_error& e)
    {
        memset(&result, 0, sizeof(result));
    }
}

void printQxFee(const char* nodeIp, const int nodePort)
{
    QxFees_output result;
    getQxFees(nodeIp, nodePort, result);
    LOG("Asset issuance fee: %u\n", result.assetIssuanceFee);
    LOG("Transfer fee: %u\n", result.transferFee);
    LOG("Trade fee: %u\n", result.tradeFee);
}

void qxIssueAsset(const char* nodeIp, int nodePort,
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
    getPublicKeyFromIdentity(QX_ADDRESS, destPublicKey);

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        IssueAsset_input ia;
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
    packet.transaction.inputType = QX_ISSUE_ASSET;
    packet.transaction.inputSize = sizeof(IssueAsset_input);

    // fill the input
    memcpy(&packet.ia.name, assetNameS1, 8);
    memcpy(&packet.ia.unitOfMeasurement, UoMS1, 8);
    packet.ia.numberOfUnits = numberOfUnits;
    packet.ia.numberOfDecimalPlaces = numberOfDecimalPlaces;
    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(IssueAsset_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(IssueAsset_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(IssueAsset_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ia));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qxTransferAsset(const char* nodeIp, int nodePort,
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
    getPublicKeyFromIdentity(QX_ADDRESS, destPublicKey);
    getPublicKeyFromIdentity(newOwnerIdentity, newOwnerPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        TransferAssetOwnershipAndPossession_input ta;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000;
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QX_TRANSFER_SHARE;
    packet.transaction.inputSize = sizeof(TransferAssetOwnershipAndPossession_input);

    // fill the input
    memcpy(&packet.ta.assetName, assetNameU1, 8);
    memcpy(packet.ta.issuer, issuer, 32);
    memcpy(packet.ta.newOwnerAndPossessor, newOwnerPublicKey, 32);
    packet.ta.numberOfUnits = numberOfUnits;
    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(TransferAssetOwnershipAndPossession_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(TransferAssetOwnershipAndPossession_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(TransferAssetOwnershipAndPossession_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ta));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

template <int functionNumber>
void qxOrderAction(const char* nodeIp, int nodePort,
                   const char* seed,
                   const char* pAssetName,
                   const char* pIssuerInQubicFormat,
                   const long long price,
                   const long long numberOfShares,
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
    getPublicKeyFromIdentity(QX_ADDRESS, destPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        qxOrderAction_input qoa;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1; // free
    if (functionNumber == QX_ADD_BID_ORDER)
    {
        packet.transaction.amount = price * numberOfShares;
    }

    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = functionNumber;
    packet.transaction.inputSize = sizeof(qxOrderAction_input);

    // DEBUG LOG
    LOG("\n-------------------------------------\n\n");
    LOG("Sending QX action - functionNumber: %d\n", functionNumber);
    LOG("Issuer: %s\n", pIssuerInQubicFormat);
    LOG("assetName: %s\n", assetNameU1);
    LOG("price: %lld\n", price);
    LOG("numberOfShares: %lld\n", numberOfShares);
    LOG("\n-------------------------------------\n\n");

    // fill the input
    memcpy(&packet.qoa.assetName, assetNameU1, 8);
    memcpy(packet.qoa.issuer, issuer, 32);
    packet.qoa.price = price;
    packet.qoa.numberOfShares = numberOfShares;
    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(qxOrderAction_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(qxOrderAction_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(qxOrderAction_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.qoa));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qxTransferAssetManagementRights(const char* nodeIp, int nodePort,
    const char* seed,
    const char* pAssetName,
    const char* pIssuerInQubicFormat,
    uint32_t newManagingContractIndex,
    int64_t numberOfShares,
    uint32_t scheduledTickOffset)
{
    qxTransferShareManagementRights_input v;
    memset(&v, 0, sizeof(v));
    getPublicKeyFromIdentity(pIssuerInQubicFormat, v.asset.issuer);
    v.asset.assetName = 0;
    memcpy(&v.asset.assetName, pAssetName, strlen(pAssetName));
    v.newManagingContractIndex = newManagingContractIndex;
    v.numberOfShares = numberOfShares;

    LOG("\nSending tx for transferring management rights ...\n");
    makeContractTransaction(nodeIp, nodePort, seed, QX_CONTRACT_INDEX, QX_TRANSFER_SHARE_MANAGEMENT_RIGHTS,
        0, sizeof(v), (uint8_t*)&v, scheduledTickOffset);
}

void qxAddToAskOrder(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* pAssetName,
                     const char* pIssuerInQubicFormat,
                     const long long price,
                     const long long numberOfShares,
                     uint32_t scheduledTickOffset)
{
    qxOrderAction<QX_ADD_ASK_ORDER>(nodeIp, nodePort, seed, pAssetName, pIssuerInQubicFormat, price, numberOfShares, scheduledTickOffset);
}

void qxAddToBidOrder(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* pAssetName,
                     const char* pIssuerInQubicFormat,
                     const long long price,
                     const long long numberOfShares,
                     uint32_t scheduledTickOffset)
{
    qxOrderAction<QX_ADD_BID_ORDER>(nodeIp, nodePort, seed, pAssetName, pIssuerInQubicFormat, price, numberOfShares, scheduledTickOffset);
}

void qxRemoveToAskOrder(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* pAssetName,
                     const char* pIssuerInQubicFormat,
                     const long long price,
                     const long long numberOfShares,
                     uint32_t scheduledTickOffset)
{
    qxOrderAction<QX_REMOVE_ASK_ORDER>(nodeIp, nodePort, seed, pAssetName, pIssuerInQubicFormat, price, numberOfShares, scheduledTickOffset);
}

void qxRemoveToBidOrder(const char* nodeIp, int nodePort,
                        const char* seed,
                        const char* pAssetName,
                        const char* pIssuerInQubicFormat,
                        const long long price,
                        const long long numberOfShares,
                        uint32_t scheduledTickOffset)
{
    qxOrderAction<QX_REMOVE_BID_ORDER>(nodeIp, nodePort, seed, pAssetName, pIssuerInQubicFormat, price, numberOfShares, scheduledTickOffset);
}

void printAssetOrders(qxGetAssetOrder_output& orders)
{
    int N = sizeof(orders) /sizeof(orders.orders[0]);
    LOG("Entity\t\tPrice\tNumberOfShares\n");
    for (int i = 0; i < N; i++)
    {
        if (!isZeroPubkey(orders.orders[i].entity))
        {
            char iden[120];
            memset(iden, 0, 120);
            getIdentityFromPublicKey(orders.orders[i].entity, iden, false);
            LOG("%s\t%lld\t%lld\n", iden, orders.orders[i].price, orders.orders[i].numberOfShares);
        }
        else
        {
            break;
        }
    }
}

template <int procedureNumber>
void qxGetAssetOrder(const char* nodeIp, int nodePort,
                const char* pAssetName,
                const char* pIssuer,
                const long long offset)
{
    uint8_t issuer[32] = {0};
    char assetNameU1[8] = {0};
    memcpy(assetNameU1, pAssetName, strlen(pAssetName));
    if (strlen(pIssuer) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pIssuer, issuer);

    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        qxGetAssetOrder_input qgao;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(qxGetAssetOrder_input);
    packet.rcf.inputType = procedureNumber;
    packet.rcf.contractIndex = QX_CONTRACT_INDEX;
    memcpy(packet.qgao.issuer, issuer, 32);
    memcpy(&packet.qgao.assetName, assetNameU1, 8);
    packet.qgao.offset = offset;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        qxGetAssetOrder_output orders = qc->receivePacketWithHeaderAs<qxGetAssetOrder_output>();
        printAssetOrders(orders);
    }
    catch (std::logic_error& e) {}
}

void qxGetAssetAskOrder(const char* nodeIp, int nodePort,
                        const char* pAssetName,
                        const char* pIssuerInQubicFormat,
                        const long long offset)
{
    qxGetAssetOrder<QX_GET_ASSET_ASK_ORDER>(nodeIp, nodePort, pAssetName, pIssuerInQubicFormat, offset);
}

void qxGetAssetBidOrder(const char* nodeIp, int nodePort,
                     const char* pAssetName,
                     const char* pIssuerInQubicFormat,
                     const long long offset)
{
    qxGetAssetOrder<QX_GET_ASSET_BID_ORDER>(nodeIp, nodePort, pAssetName, pIssuerInQubicFormat, offset);
}

void printEntityOrders(qxGetEntityOrder_output& orders)
{
    int N = sizeof(orders) /sizeof(orders.orders[0]);
    LOG("Issuer\t\tAssetName\tPrice\tNumberOfShares\n");
    for (int i = 0; i < N; i++)
    {
        if (orders.orders[i].price ||
            orders.orders[i].numberOfShares)
        {
            char iden[120];
            char assetName[8] = {'N','O',' ', 'N','A','M','E', 0};
            memset(iden, 0, 120);
            getIdentityFromPublicKey(orders.orders[i].issuer, iden, false);
            if (orders.orders[i].assetName)
                memcpy(assetName, &orders.orders[i].assetName, 8);
            LOG("%s\t%s\t%lld\t%lld\n", iden, assetName, orders.orders[i].price, orders.orders[i].numberOfShares);
        }
        else
        {
            break;
        }
    }
}

template <int procedureNumber>
void qxGetEntityOrder(const char* nodeIp, int nodePort,
                     const char* pEntity,
                     const long long offset)
{
    uint8_t entity[32] = {0};
    if (strlen(pEntity) != 60)
    {
        LOG("WARNING: Stop supporting hex format, please use qubic format 60-char length addresses\n");
        exit(0);
    }
    getPublicKeyFromIdentity(pEntity, entity);

    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        qxGetEntityOrder_input qgeo;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(qxGetEntityOrder_input);
    packet.rcf.inputType = procedureNumber;
    packet.rcf.contractIndex = QX_CONTRACT_INDEX;
    memcpy(packet.qgeo.entity, entity, 32);
    packet.qgeo.offset = offset;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        qxGetEntityOrder_output orders = qc->receivePacketWithHeaderAs<qxGetEntityOrder_output>();
        printEntityOrders(orders);
    }
    catch (std::logic_error& e) {}
}

void qxGetEntityAskOrder(const char* nodeIp, int nodePort,
                        const char* pHexEntity,
                        const long long offset)
{
    qxGetEntityOrder<QX_GET_ENTITY_ASK_ORDER>(nodeIp, nodePort, pHexEntity, offset);
}

void qxGetEntityBidOrder(const char* nodeIp, int nodePort,
                         const char* pHexEntity,
                         const long long offset)
{
    qxGetEntityOrder<QX_GET_ENTITY_BID_ORDER>(nodeIp, nodePort, pHexEntity, offset);
}
