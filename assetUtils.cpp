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
    auto qc = new QubicConnection(nodeIp, nodePort);
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
static void printOwnedAsset(Asset owned, Asset iss)
{
    char hexIssuer[128];
    char name[8] = {0};
    memcpy(name, iss.varStruct.issuance.name, 7);
    byteToHex(iss.varStruct.issuance.publicKey, hexIssuer, 32);
    LOG("Asset issuer: %s\n", hexIssuer);
    LOG("Asset name: %s\n", name);
    LOG("Asset type: %u\n", iss.varStruct.issuance.type);
    LOG("Type: %u\n", owned.varStruct.ownership.type);
    LOG("Managing contract index: %d\n", owned.varStruct.ownership.managingContractIndex);
    LOG("Issuance Index: %d\n", owned.varStruct.ownership.issuanceIndex);
    LOG("Number Of Units: %d\n", owned.varStruct.ownership.numberOfUnits);
}
void printOwnedAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    auto vroa = getOwnedAsset(nodeIp, nodePort, requestedIdentity);
    for (auto& roa : vroa){
        printOwnedAsset(roa.asset, roa.issuanceAsset);
        LOG("Tick: %u\n", roa.tick);
    }
}
/*
 * struct TransferAssetOwnershipAndPossession_input
	{
		id issuer;
		id possessor;
		id newOwner;
		unsigned long long assetName;
		long long numberOfUnits;
	};
 */
void transferQxAsset(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* possessorIdentity,
                     const char* newOwnerIdentity,
                     long long numberOfUnits,
                     uint32_t scheduledTickOffset)
{
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
    char assetName[8] = {'Q', 'X', 0, 0,0,0,0,0};
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
    packet.transaction.amount = 0;
    uint32_t scheduledTick = 0;
    if (scheduledTickOffset < 50000){
        uint32_t currentTick = getTickNumberFromNode(nodeIp, nodePort);
        scheduledTick = currentTick + scheduledTickOffset;
    } else {
        scheduledTick = scheduledTickOffset;
    }
    packet.transaction.tick = scheduledTick;

    packet.transaction.inputType = 1;
    packet.transaction.inputSize = sizeof(TransferAssetOwnershipAndPossession_input);

    // fill the input
    memcpy(&packet.ta.assetName, assetName, 8);
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
    auto qc = new QubicConnection(nodeIp, nodePort);
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
    delete qc;
}