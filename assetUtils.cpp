#include <cstdint>
#include <cstring>
#include "structs.h"
#include "walletUtils.h"
#include "keyUtils.h"
#include "assetUtil.h"
#include "connection.h"
#include "logger.h"
#include "nodeUtils.h"
#include "K12AndKeyUtil.h"

std::vector<RespondOwnedAssets> getOwnedAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    std::vector<RespondOwnedAssets> result;
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(requestedIdentity, publicKey);
    struct {
        RequestResponseHeader header;
        RequestOwnedAssets req;
    } packet;
    memcpy(packet.req.publicKey, publicKey, 32);
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_OWNED_ASSETS);
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RESPOND_OWNED_ASSETS){
            auto roa = (RespondOwnedAssets *)(data + ptr + sizeof(RequestResponseHeader));
            result.push_back(* roa);
        }
        ptr+= header->size();
    }
    return result;
}
std::vector<RespondPossessedAssets> getPossessionAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    std::vector<RespondPossessedAssets> result;
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(requestedIdentity, publicKey);
    struct {
        RequestResponseHeader header;
        RequestOwnedAssets req;
    } packet;
    memcpy(packet.req.publicKey, publicKey, 32);
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_POSSESSED_ASSETS);
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RESPOND_POSSESSED_ASSETS){
            auto rpa = (RespondPossessedAssets *)(data + ptr + sizeof(RequestResponseHeader));
            result.push_back(* rpa);
        }
        ptr+= header->size();
    }
    return result;
}
static void printOwnedAsset(Asset owned, Asset iss)
{
    char hexIssuer[128];
    char name[8] = {0};
    char unitOfMeasurement[7] = {0};
    memcpy(name, iss.varStruct.issuance.name, 7);
    memcpy(unitOfMeasurement, iss.varStruct.issuance.unitOfMeasurement, 7);
    byteToHex(iss.varStruct.issuance.publicKey, hexIssuer, 32);
    LOG("Asset issuer: %s\n", hexIssuer);
    LOG("Asset name: %s\n", name);
    LOG("Managing contract index: %d\n", owned.varStruct.ownership.managingContractIndex);
    LOG("Issuance Index: %d\n", owned.varStruct.ownership.issuanceIndex);
    LOG("Number Of Units: %d\n", owned.varStruct.ownership.numberOfUnits);
}
static void printPossessionAsset(Asset owner, Asset possession, Asset iss)
{
    char hexIssuer[128];
    char name[8] = {0};
    char ownerId[128] = {0};
    getIdentityFromPublicKey(owner.varStruct.ownership.publicKey, ownerId, false);
    memcpy(name, iss.varStruct.issuance.name, 7);
    byteToHex(iss.varStruct.issuance.publicKey, hexIssuer, 32);
    LOG("Asset issuer: %s\n", hexIssuer);
    LOG("Asset name: %s\n", name);
    LOG("Managing contract index: %d\n", possession.varStruct.possession.managingContractIndex);
    LOG("Owner index: %u\n", possession.varStruct.possession.ownershipIndex);
    LOG("Owner ID: %s\n", ownerId);
    LOG("Number Of Units: %d\n", possession.varStruct.possession.numberOfUnits);
}
void printOwnedAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    LOG("======== OWNERSHIP ========\n");
    auto vroa = getOwnedAsset(nodeIp, nodePort, requestedIdentity);
    for (auto& roa : vroa){
        printOwnedAsset(roa.asset, roa.issuanceAsset);
        LOG("Tick: %u\n", roa.tick);
    }
}
void printPossessionAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    LOG("======== POSSESSION ========\n");
    auto vrpa = getPossessionAsset(nodeIp, nodePort, requestedIdentity);
    for (auto& rpa : vrpa){
        printPossessionAsset(rpa.ownershipAsset, rpa.asset, rpa.issuanceAsset);
        LOG("Tick: %u\n", rpa.tick);
    }
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
    if (scheduledTickOffset < 50000){
        uint32_t currentTick = getTickNumberFromNode(qc);
        scheduledTick = currentTick + scheduledTickOffset;
    } else {
        scheduledTick = scheduledTickOffset;
    }
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = 1;
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
                     const char* pIssuer,
                     const char* possessorIdentity,
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
    uint8_t possessorPublicKey[32] = {0};
    uint8_t newOwnerPublicKey[32] = {0};
    char txHash[128] = {0};
    char assetNameU1[8] = {0};

    memcpy(assetNameU1, pAssetName, strlen(pAssetName));
    hexToByte(pIssuer, issuer, 32);

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(QX_ADDRESS, destPublicKey);
    getPublicKeyFromIdentity(possessorIdentity, possessorPublicKey);
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
    uint32_t scheduledTick = 0;
    if (scheduledTickOffset < 50000){
        uint32_t currentTick = getTickNumberFromNode(qc);
        scheduledTick = currentTick + scheduledTickOffset;
    } else {
        scheduledTick = scheduledTickOffset;
    }
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = 2;
    packet.transaction.inputSize = sizeof(TransferAssetOwnershipAndPossession_input);

    // fill the input
    memcpy(&packet.ta.assetName, assetNameU1, 8);
    memcpy(packet.ta.issuer, issuer, 32);
    memcpy(packet.ta.possessor, possessorPublicKey, 32);
    memcpy(packet.ta.newOwner, newOwnerPublicKey, 32);
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