#include <cstdint>
#include <cstring>
#include "utils.h"
#include "nodeUtils.h"
#include "keyUtils.h"
#include "logger.h"
#include "structs.h"
#include "connection.h"
#include "K12AndKeyUtil.h"

void printWalletInfo(const char* seed)
{
	uint8_t privateKey[32] = {0};
	uint8_t publicKey[32] = {0};
    uint8_t subseed[32] = {0};
	char privateKeyHex[128] = {0};
	char publicKeyHex[128] = {0};
    char publicIdentity[128] = {0};
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, publicKey);
    const bool isLowerCase = false;
    getIdentityFromPublicKey(publicKey, publicIdentity, isLowerCase);

    byteToHex(privateKey, privateKeyHex, 32);
    byteToHex(publicKey, publicKeyHex, 32);
    LOG("Seed: %s\n", seed);
    LOG("Private key: %s\n", privateKeyHex);
    LOG("Public key: %s\n", publicKeyHex);
    LOG("Identity: %s\n", publicIdentity);
}
RespondedEntity getBalance(const char* nodeIp, const int nodePort, const uint8_t* publicKey)
{
    RespondedEntity result;
    memset(&result, 0, sizeof(RespondedEntity));
    std::vector<uint8_t> buffer;
    buffer.resize(0);
    uint8_t tmp[1024] = {0};
    auto qc = new QubicConnection(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestedEntity req;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_ENTITY);
    memcpy(packet.req.publicKey, publicKey, 32);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    int recvByte = qc->receiveData(tmp, 1024);
    while (recvByte > 0)
    {
        buffer.resize(recvByte + buffer.size());
        memcpy(buffer.data() + buffer.size() - recvByte, tmp, recvByte);
        recvByte = qc->receiveData(tmp, 1024);
    }
    uint8_t* data = buffer.data();
    recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RESPOND_ENTITY){
            auto entity = (RespondedEntity*)(data + ptr + sizeof(RequestResponseHeader));
            result = *entity;
        }
        ptr+= header->size();
    }
    delete qc;
    return result;
}
void printBalance(const char* publicIdentity, const char* nodeIp, int nodePort)
{
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(publicIdentity, publicKey);
    auto entity = getBalance(nodeIp, nodePort, publicKey);
    LOG("Identity: %s\n", publicIdentity);
    LOG("Balance: %ld\n", entity.entity.incomingAmount - entity.entity.outgoingAmount);
    LOG("Incoming Amount: %ld\n", entity.entity.incomingAmount);
    LOG("Outgoing Amount: %ld\n", entity.entity.outgoingAmount);
    LOG("Number Of Incoming Transfers: %ld\n", entity.entity.numberOfIncomingTransfers);
    LOG("Number Of Outgoing Transfers: %ld\n", entity.entity.numberOfOutgoingTransfers);
    LOG("Latest Incoming Transfer Tick: %u\n", entity.entity.latestIncomingTransferTick);
    LOG("Latest Outgoing Transfer Tick: %u\n", entity.entity.latestOutgoingTransferTick);
}

void printReceipt(Transaction& tx, const char* txHash, const uint8_t* extraData)
{
    char sourceIdentity[128] = {0};
    char dstIdentity[128] = {0};
    char txHashClean[128] = {0};
    bool isLowerCase = false;
    getIdentityFromPublicKey(tx.sourcePublicKey, sourceIdentity, isLowerCase);
    getIdentityFromPublicKey(tx.destinationPublicKey, dstIdentity, isLowerCase);
    LOG("~~~~~RECEIPT~~~~~\n");
    if (txHash != nullptr) {
        memcpy(txHashClean, txHash, 60);
        LOG("TxHash: %s\n", txHashClean);
    }
    LOG("From: %s\n", sourceIdentity);
    LOG("To: %s\n", dstIdentity);
    LOG("Input type: %u\n", tx.inputType);
    LOG("Amount: %lu\n", tx.amount);
    LOG("Tick: %u\n", tx.tick);
    LOG("Extra data size: %u\n", tx.inputSize);
    if (extraData != nullptr){
        char hex_tring[1024] = {0};
        for (int i = 0; i < tx.inputSize; i++)
            sprintf(hex_tring + i * 2, "%02x", extraData[i]);

        LOG("Extra data: %s\n", hex_tring);
    }
    LOG("~~~~~END-RECEIPT~~~~~\n");
}
bool verifyTx(Transaction& tx, const uint8_t* extraData, const uint8_t* signature)
{
    std::vector<uint8_t> buffer;
    uint8_t digest[32] = {0};
    buffer.resize(sizeof(Transaction) + tx.inputSize);
    memcpy(buffer.data(), &tx, sizeof(Transaction));
    memcpy(buffer.data() + sizeof(Transaction), extraData, tx.inputSize);
    KangarooTwelve(buffer.data(),
                   buffer.size(),
                   digest,
                   32);
    return verify(tx.sourcePublicKey, digest, signature);
}

void makeStandardTransaction(const char* nodeIp, int nodePort, const char* seed,
                             const char* targetIdentity, const uint64_t amount, uint32_t scheduledTickOffset)
{
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
    getPublicKeyFromIdentity(targetIdentity, destPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        unsigned char signature[64];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = amount;
    uint32_t currentTick = getTickNumberFromNode(nodeIp, nodePort);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = 0;
    packet.transaction.inputSize = 0;
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet.header)+sizeof(packet.transaction) + 64);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    auto qc = new QubicConnection(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + 64,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void makeCustomTransaction(const char* nodeIp, int nodePort,
                           const char* seed,
                           const char* targetIdentity,
                           uint16_t txType,
                           uint64_t amount,
                           int extraDataSize,
                           const uint8_t* extraData,
                           uint32_t scheduledTickOffset)
{
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
    getPublicKeyFromIdentity(targetIdentity, destPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
    } temp_packet;
    std::vector<uint8_t> packet;
    memcpy(temp_packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(temp_packet.transaction.destinationPublicKey, destPublicKey, 32);
    temp_packet.transaction.amount = amount;
    uint32_t currentTick = getTickNumberFromNode(nodeIp, nodePort);
    temp_packet.transaction.tick = currentTick + scheduledTickOffset;
    temp_packet.transaction.inputType = txType;
    temp_packet.transaction.inputSize = extraDataSize;

    packet.resize(sizeof(RequestResponseHeader) + sizeof(Transaction) + extraDataSize + SIGNATURE_SIZE);
    memcpy(packet.data() + sizeof(RequestResponseHeader), &temp_packet.transaction, sizeof(Transaction));
    memcpy(packet.data() + sizeof(RequestResponseHeader) + sizeof(Transaction), extraData, extraDataSize);

    KangarooTwelve(packet.data() + sizeof(RequestResponseHeader),
                   sizeof(Transaction) + extraDataSize,
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.data() + sizeof(RequestResponseHeader) + sizeof(Transaction) + extraDataSize, signature, SIGNATURE_SIZE);
    temp_packet.header.setSize(sizeof(RequestResponseHeader) + sizeof(Transaction) + extraDataSize + SIGNATURE_SIZE);
    temp_packet.header.zeroDejavu();
    temp_packet.header.setType(BROADCAST_TRANSACTION);
    memcpy(packet.data(), &temp_packet.header, sizeof(RequestResponseHeader));
    auto qc = new QubicConnection(nodeIp, nodePort);
    qc->sendData(packet.data(), packet.size());

    KangarooTwelve(packet.data() + sizeof(RequestResponseHeader),
                   sizeof(Transaction) + extraDataSize + 64,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(temp_packet.transaction, txHash, extraData);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}