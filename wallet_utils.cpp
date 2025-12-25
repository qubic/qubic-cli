#include <chrono>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <atomic>

#include "wallet_utils.h"
#include "connection.h"
#include "k12_and_key_utils.h"
#include "key_utils.h"
#include "logger.h"
#include "node_utils.h"
#include "sc_utils.h"
#include "structs.h"
#include "utils.h"

void printWalletInfo(const char* seed)
{
	uint8_t privateKey[32] = {0};
	uint8_t publicKey[32] = {0};
    uint8_t subseed[32] = {0};
	char privateKeyQubicFormat[128] = {0};
	char publicKeyQubicFormat[128] = {0};
    char publicIdentity[128] = {0};
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, publicKey);
    getIdentityFromPublicKey(publicKey, publicIdentity, /*isLowerCase=*/false);

    getIdentityFromPublicKey(privateKey, privateKeyQubicFormat, /*isLowerCase=*/true);
    getIdentityFromPublicKey(publicKey, publicKeyQubicFormat, /*isLowerCase=*/true);
    LOG("Seed: %s\n", seed);
    LOG("Private key: %s\n", privateKeyQubicFormat);
    LOG("Public key: %s\n", publicKeyQubicFormat);
    LOG("Identity: %s\n", publicIdentity);
}

RespondedEntity getBalance(const char* nodeIp, const int nodePort, const uint8_t* publicKey)
{
    RespondedEntity result;
    memset(&result, 0, sizeof(RespondedEntity));
    std::vector<uint8_t> buffer;
    buffer.resize(0);
    uint8_t tmp[1024] = {0};
    auto qc = make_qc(nodeIp, nodePort);
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
    recvByte = int(buffer.size());
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RESPOND_ENTITY)
        {
            auto entity = (RespondedEntity*)(data + ptr + sizeof(RequestResponseHeader));
            result = *entity;
        }
        ptr+= header->size();
    }

    return result;
}

void getSpectrumDigest(RespondedEntity& respondedEntity, uint8_t* spectrumDigest)
{
    // Check if the size of entity is good
    const size_t entity_size = sizeof(respondedEntity.entity);
    if (entity_size != 64)
    {
        LOG("Size of entity is unexpected: %lu\n", sizeof(entity_size));
        return;
    }

    // Compute the root hash from the entity and siblings
    if (respondedEntity.spectrumIndex < 0 )
    {
        LOG("Spectrum index is invalid: %d\n", respondedEntity.spectrumIndex);
        return;
    }

    // Compute the spectrum digest
    getDigestFromSiblings<32>(
        SPECTRUM_DEPTH,
        (uint8_t*)(&respondedEntity.entity),
        entity_size,
        respondedEntity.spectrumIndex,
        respondedEntity.siblings,
        spectrumDigest);
}

void printBalance(const char* publicIdentity, const char* nodeIp, int nodePort)
{
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(publicIdentity, publicKey);
    auto entity = getBalance(nodeIp, nodePort, publicKey);
    LOG("Identity: %s\n", publicIdentity);
    LOG("Balance: %lld\n", entity.entity.incomingAmount - entity.entity.outgoingAmount);
    LOG("Incoming Amount: %lld\n", entity.entity.incomingAmount);
    LOG("Outgoing Amount: %lld\n", entity.entity.outgoingAmount);
    LOG("Number Of Incoming Transfers: %ld\n", entity.entity.numberOfIncomingTransfers);
    LOG("Number Of Outgoing Transfers: %ld\n", entity.entity.numberOfOutgoingTransfers);
    LOG("Latest Incoming Transfer Tick: %u\n", entity.entity.latestIncomingTransferTick);
    LOG("Latest Outgoing Transfer Tick: %u\n", entity.entity.latestOutgoingTransferTick);

    // Get the spectrum digest from entity
    uint8_t spectumDigest[32] = {0};
    getSpectrumDigest(entity, spectumDigest);
    char hex[65];
    LOG("Tick: %u\n", entity.tick);
    byteToHex(spectumDigest, hex, 32);
    LOG("Spectum Digest: %s\n", hex);
}

void printReceipt(Transaction& tx, const char* txHash, const uint8_t* extraData, int moneyFlew)
{
    char sourceIdentity[128] = {0};
    char dstIdentity[128] = {0};
    char txHashClean[128] = {0};
    bool isLowerCase = false;
    getIdentityFromPublicKey(tx.sourcePublicKey, sourceIdentity, isLowerCase);
    getIdentityFromPublicKey(tx.destinationPublicKey, dstIdentity, isLowerCase);
    LOG("~~~~~RECEIPT~~~~~\n");
    if (txHash != nullptr)
    {
        memcpy(txHashClean, txHash, 60);
        LOG("TxHash: %s\n", txHashClean);
    }
    LOG("From: %s\n", sourceIdentity);
    LOG("To: %s\n", dstIdentity);
    LOG("Input type: %u\n", tx.inputType);
    LOG("Amount: %lu\n", tx.amount);
    LOG("Tick: %u\n", tx.tick);
    LOG("Extra data size: %u\n", tx.inputSize);
    if (extraData != nullptr && tx.inputSize)
    {
        char hex_tring[1024*2+1] = {0};
        for (int i = 0; i < tx.inputSize; i++)
            snprintf(hex_tring + i * 2, 3, "%02x", extraData[i]);

        LOG("Extra data: %s\n", hex_tring);
    }
    if (moneyFlew != -1)
    {
        if (moneyFlew) LOG("MoneyFlew: Yes\n");
        else LOG("MoneyFlew: No\n");
    }
    else
    {
        LOG("MoneyFlew: N/A\n");
    }
    LOG("~~~~~END-RECEIPT~~~~~\n");
}

bool verifyTx(Transaction& tx, const uint8_t* extraData, const uint8_t* signature)
{
    std::vector<uint8_t> buffer;
    uint8_t digest[32] = {0};
    buffer.resize(sizeof(Transaction) + tx.inputSize);
    memcpy(buffer.data(), &tx, sizeof(Transaction));
    if (extraData && tx.inputSize) memcpy(buffer.data() + sizeof(Transaction), extraData, tx.inputSize);
    KangarooTwelve(buffer.data(),
                   uint32_t(buffer.size()),
                   digest,
                   32);
    return verify(tx.sourcePublicKey, digest, signature);
}

void makeStandardTransactionInTick(const char* nodeIp, int nodePort, const char* seed,
                             const char* targetIdentity, const uint64_t amount, uint32_t txTick,
                             int waitUntilFinish)
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
    getPublicKeyFromIdentity(targetIdentity, destPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        unsigned char signature[64];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = amount;
    packet.transaction.tick = txTick;
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
    qc->sendData((uint8_t *) &packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + 64,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    if (waitUntilFinish)
    {
        LOG("Waiting for tick:\n");
        uint32_t currentTick = getTickNumberFromNode(qc);
        while (currentTick <= packet.transaction.tick)
        {
            LOG("%d/%d\n", currentTick, packet.transaction.tick);
            Q_SLEEP(1000);
            currentTick = getTickNumberFromNode(qc);
        }
        checkTxOnTick(nodeIp, nodePort, txHash, packet.transaction.tick);
    }
    else
    {
        LOG("run ./qubic-cli [...] -checktxontick %u %s\n", txTick, txHash);
        LOG("to check your tx confirmation status\n");
    }
}

void makeStandardTransaction(const char* nodeIp, int nodePort, const char* seed,
                             const char* targetIdentity, const uint64_t amount, uint32_t scheduledTickOffset,
                             int waitUntilFinish)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t txTick = currentTick + scheduledTickOffset;
    makeStandardTransactionInTick(nodeIp, nodePort, seed, targetIdentity, amount, txTick, waitUntilFinish); 
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
    if (extraDataSize < 0)
        throw std::invalid_argument("extraDataSize < 0");

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
    getPublicKeyFromIdentity(targetIdentity, destPublicKey);
    struct {
        RequestResponseHeader header;
        Transaction transaction;
    } temp_packet;
    std::vector<uint8_t> packet;
    memcpy(temp_packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(temp_packet.transaction.destinationPublicKey, destPublicKey, 32);
    temp_packet.transaction.amount = amount;
    uint32_t currentTick = getTickNumberFromNode(qc);
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
    qc->sendData(packet.data(), uint32_t(packet.size()));

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

void makeContractTransaction(const char* nodeIp, int nodePort,
    const char* seed,
    uint64_t contractIndex,
    uint16_t txType,
    uint64_t amount,
    int extraDataSize,
    const void* extraData,
    uint32_t scheduledTickOffset,
    QCPtr* qcPtr)
{
    QCPtr qc = (!qcPtr) ? make_qc(nodeIp, nodePort) : *qcPtr;

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint64_t destPublicKey[4] = { contractIndex, 0, 0, 0 };
    uint8_t subseed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    std::vector<uint8_t> packet(sizeof(RequestResponseHeader) + sizeof(Transaction) + extraDataSize + SIGNATURE_SIZE);
    RequestResponseHeader& packetHeader = (RequestResponseHeader&)packet[0];
    Transaction& packetTransaction = (Transaction&)packet[sizeof(RequestResponseHeader)];
    uint8_t* packetInputData = &packet[sizeof(RequestResponseHeader) + sizeof(Transaction)];
    uint8_t* packetSignature = packetInputData + extraDataSize;

    packetHeader.setSize(uint32_t(packet.size()));
    packetHeader.zeroDejavu();
    packetHeader.setType(BROADCAST_TRANSACTION);

    memcpy(packetTransaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packetTransaction.destinationPublicKey, destPublicKey, 32);
    packetTransaction.amount = amount;
    packetTransaction.tick = getTickNumberFromNode(qc) + scheduledTickOffset;
    packetTransaction.inputType = txType;
    packetTransaction.inputSize = extraDataSize;

    if (extraDataSize)
        memcpy(packetInputData, extraData, extraDataSize);

    KangarooTwelve(packet.data() + sizeof(RequestResponseHeader),
        sizeof(Transaction) + extraDataSize,
        digest,
        32);
    sign(subseed, sourcePublicKey, digest, packetSignature);

    qc->sendData(packet.data(), int(packet.size()));

    KangarooTwelve(packet.data() + sizeof(RequestResponseHeader),
        sizeof(Transaction) + extraDataSize + 64,
        digest,
        32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packetTransaction, txHash, (uint8_t*)extraData);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", packetTransaction.tick, txHash);
    LOG("to check your tx confirmation status\n");
}

bool runContractFunction(const char* nodeIp, int nodePort,
    unsigned int contractIndex,
    unsigned short funcNumber,
    void* inputPtr,
    size_t inputSize,
    void* outputPtr,
    size_t outputSize,
    QCPtr* qcPtr)
{
    QCPtr qc = (!qcPtr) ? make_qc(nodeIp, nodePort) : *qcPtr;
    std::vector<uint8_t> packet(sizeof(RequestResponseHeader) + sizeof(RequestContractFunction) + inputSize);
    RequestResponseHeader& packetHeader = (RequestResponseHeader&)packet[0];
    RequestContractFunction& packetRcf = (RequestContractFunction&)packet[sizeof(RequestResponseHeader)];
    uint8_t* packetInputData = (inputSize) ? &packet[sizeof(RequestResponseHeader) + sizeof(RequestContractFunction)] : nullptr;

    packetHeader.setSize(uint32_t(packet.size()));
    packetHeader.randomizeDejavu();
    packetHeader.setType(RequestContractFunction::type());
    packetRcf.inputSize = uint16_t(inputSize);
    packetRcf.inputType = funcNumber;
    packetRcf.contractIndex = contractIndex;
    if (inputSize)
        memcpy(packetInputData, inputPtr, inputSize);
    qc->sendData(&packet[0], packetHeader.size());

    const size_t fullPacketSize = sizeof(RequestResponseHeader) + outputSize;
    std::vector<uint8_t> buffer(fullPacketSize);
    int recvByte = qc->receiveAllDataOrThrowException(buffer.data(), int(buffer.size()));

    auto header = (RequestResponseHeader*)buffer.data();
    if (header->type() == RespondContractFunction::type() && 
        recvByte == fullPacketSize)
    {
        memcpy(outputPtr, (buffer.data() + sizeof(RequestResponseHeader)), outputSize);

        if (header->size() > fullPacketSize)
        {
            const size_t dropSize = header->size() - fullPacketSize;
            LOG("WARNING: Response of runContractFunction() is %llu bytes longer than expected. Dropping unexpected part that cannot be interpreted.\n", (unsigned long long)dropSize);
            if (dropSize > buffer.size())
                buffer.resize(dropSize);
            qc->receiveAllDataOrThrowException(buffer.data(), int(dropSize));
        }

        return true;
    }
    
    return false;
}

bool runContractFunction(const char* nodeIp, int nodePort,
    unsigned int contractIndex,
    unsigned short funcNumber,
    const char* formatInput,
    const char* formatOutput) {
    LOG("Calling contract %u function %u | input : %s | output %s\n", contractIndex, funcNumber, formatInput, formatOutput);
    QCPtr qc = make_qc(nodeIp, nodePort);
    ContractObject contractObject = buildContractObject(formatInput, true);
    auto inputSize = contractObject.getSize();
    void* inputData = nullptr;
    if (inputSize) {
        inputData = malloc(inputSize);
        if (!inputData) {
            throw std::runtime_error("Failed to allocate memory for inputData");
        }
    }
    contractObject.dumpIntoBuffer(inputData);
    std::vector<uint8_t> packet(sizeof(RequestResponseHeader) + sizeof(RequestContractFunction) + inputSize);
    RequestResponseHeader& packetHeader = (RequestResponseHeader&)packet[0];
    RequestContractFunction& packetRcf = (RequestContractFunction&)packet[sizeof(RequestResponseHeader)];
    uint8_t* packetInputData = (inputSize) ? &packet[sizeof(RequestResponseHeader) + sizeof(RequestContractFunction)] : nullptr;

    packetHeader.setSize(uint32_t(packet.size()));
    packetHeader.randomizeDejavu();
    packetHeader.setType(RequestContractFunction::type());
    packetRcf.inputSize = uint16_t(inputSize);
    packetRcf.inputType = funcNumber;
    packetRcf.contractIndex = contractIndex;
    if (inputSize)
        memcpy(packetInputData, inputData, inputSize);
    qc->sendData(&packet[0], packetHeader.size());

    ContractObject outputContractObject = buildContractObject(formatOutput, true);
    auto outputSize = outputContractObject.getSize();
    void* outputData = nullptr;
    if (outputSize) {
        outputData = malloc(outputSize);
        if (!outputData) {
            free(inputData);
            throw std::runtime_error("Failed to allocate memory for outputData");
        }
    }
    std::vector<uint8_t> buffer(sizeof(RequestResponseHeader) + outputSize);
    int recvByte = qc->receiveAllDataOrThrowException(buffer.data(), int(buffer.size()));

    auto header = (RequestResponseHeader*)buffer.data();
    if (header->type() == RespondContractFunction::type() &&
        recvByte - sizeof(RequestResponseHeader) == outputSize)
    {
        memcpy(outputData, (buffer.data() + sizeof(RequestResponseHeader)), outputSize);
        outputContractObject = ContractObject::fromBuffer(outputData, formatOutput, true);
        LOG("------------------ Contract Function Output ------------------\n");
        outputContractObject.print();
        return true;
    }

    return false;
}


void invokeContractProcedure(const char* nodeIp, int nodePort,
    const char* seed,
    uint64_t contractIndex,
    uint16_t txType,
    uint64_t amount,
    const char* formatInput,
    uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint64_t destPublicKey[4] = { contractIndex, 0, 0, 0 };
    uint8_t subseed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ContractObject contractObject = buildContractObject(formatInput, true);
    auto extraDataSize = (unsigned short)contractObject.getSize();
    if (extraDataSize > MAX_INPUT_SIZE) {
        throw std::runtime_error("Input data size exceeds maximum allowed size");
    }
    void* extraData = malloc(extraDataSize);
    if (!extraData) {
        throw std::runtime_error("Failed to allocate memory for extraData");
    }
    contractObject.dumpIntoBuffer(extraData);

    std::vector<uint8_t> packet(sizeof(RequestResponseHeader) + sizeof(Transaction) + extraDataSize + SIGNATURE_SIZE);
    RequestResponseHeader& packetHeader = (RequestResponseHeader&)packet[0];
    Transaction& packetTransaction = (Transaction&)packet[sizeof(RequestResponseHeader)];
    uint8_t* packetInputData = &packet[sizeof(RequestResponseHeader) + sizeof(Transaction)];
    uint8_t* packetSignature = packetInputData + extraDataSize;

    packetHeader.setSize(uint32_t(packet.size()));
    packetHeader.zeroDejavu();
    packetHeader.setType(BROADCAST_TRANSACTION);

    memcpy(packetTransaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packetTransaction.destinationPublicKey, destPublicKey, 32);
    packetTransaction.amount = amount;
    packetTransaction.tick = getTickNumberFromNode(qc) + scheduledTickOffset;
    packetTransaction.inputType = txType;
    packetTransaction.inputSize = extraDataSize;

    if (extraDataSize)
        memcpy(packetInputData, extraData, extraDataSize);

    KangarooTwelve(packet.data() + sizeof(RequestResponseHeader),
        sizeof(Transaction) + extraDataSize,
        digest,
        32);
    sign(subseed, sourcePublicKey, digest, packetSignature);

    qc->sendData(packet.data(), int(packet.size()));

    KangarooTwelve(packet.data() + sizeof(RequestResponseHeader),
        sizeof(Transaction) + extraDataSize + 64,
        digest,
        32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packetTransaction, txHash, (uint8_t*)extraData);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", packetTransaction.tick, txHash);
    LOG("to check your tx confirmation status\n");
}

void makeIPOBid(const char* nodeIp, int nodePort,
                const char* seed,
                uint32_t contractIndex,
                uint64_t pricePerShare,
                uint16_t numberOfShare,
                uint32_t scheduledTickOffset)
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
    // Contracts are identified by their index stored in the first 64 bits of the id, all
    // other bits are zeroed. However, the max number of contracts is limited to 2^32 - 1,
    // only 32 bits are used for the contract index.
    ((uint64_t*)destPublicKey)[0] = contractIndex;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        ContractIPOBid ipo;
        unsigned char signature[64];
    } packet;
    memset(&packet.ipo, 0, sizeof(packet.ipo));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 0;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = 0;
    packet.transaction.inputSize = sizeof(packet.ipo);
    packet.ipo.price = pricePerShare;
    packet.ipo.quantity = numberOfShare;
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(packet.ipo),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(packet.ipo) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("IPO bidding has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

RespondContractIPO _getIPOStatus(const char* nodeIp, int nodePort, uint32_t contractIndex)
{
    RespondContractIPO result;
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractIPO req;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_CONTRACT_IPO);
    packet.req.contractIndex = contractIndex;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<RespondContractIPO>();
    }
    catch (std::logic_error)
    {
        memset(&result, 0, sizeof(RespondContractIPO));
    }

    return result;
}

std::vector<RespondActiveIPO> _getActiveIPOs(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    RequestResponseHeader header;
    header.setSize(sizeof(header));
    header.randomizeDejavu();
    header.setType(RequestActiveIPOs::type());
    qc->sendData((uint8_t*)&header, header.size());

    return qc->getLatestVectorPacketAs<RespondActiveIPO>();
}

void printIPOStatus(const char* nodeIp, int nodePort, uint32_t contractIndex)
{
    RespondContractIPO status = _getIPOStatus(nodeIp, nodePort, contractIndex);
    LOG("Identity - Price per share\n");
    for (int i = 0; i < NUMBER_OF_COMPUTORS; i++)
    {
        if (!isZeroPubkey(status.publicKeys[i]))
        {
            char identity[128] = {0};
            getIdentityFromPublicKey(status.publicKeys[i], identity, false);

            LOG("%s: %lld\n", identity, status.prices[i]);
        }
    }
}

void printActiveIPOs(const char* nodeIp, int nodePort)
{
    std::vector<RespondActiveIPO> activeIPOs = _getActiveIPOs(nodeIp, nodePort);
    if (activeIPOs.empty())
    {
        LOG("No active IPOs in this epoch.\n");
        return;
    }

    LOG("Active IPOs in this epoch:\n");
    for (const RespondActiveIPO& ipo : activeIPOs)
    {
        LOG("- contract index: %u, asset name: %s\n", ipo.contractIndex, ipo.assetName);
    }
}

VanityAddress generateVanityAddress(const char* pattern, unsigned int vanityGenerationThreads, bool isSufix) {
    auto isAddressValid = [](const char* identity, const char* pattern, bool isSufix) -> bool {
        size_t patternLen = strlen(pattern);
        size_t identityLen = strlen(identity);
        if (isSufix) {
            if (patternLen > identityLen) return false;
            return strcmp(identity + (identityLen - patternLen), pattern) == 0;
        } else {
            if (patternLen > identityLen) return false;
            return strncmp(identity, pattern, patternLen) == 0;
        }
    };

    unsigned long long estimatedAttempts = static_cast<unsigned long long>(std::pow(24, strlen(pattern)));
    std::atomic<unsigned long long> attemptsCounter(0);

    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastPrintTime = startTime;

    // random seed (55 lowercase char)
    auto randomSeed = []() -> std::string {
      constexpr char charset[] = "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        std::string str(55, 0);
        for (size_t i = 0; i < 55; ++i) {
            str[i] = charset[rand() % max_index];
        }
        return str;
    };

    auto generateThread = [&](VanityAddress& result, const char* pattern, bool isSufix, bool& found) {
        while (!found) {
            uint8_t publicKey[32] = {0};
            char identity[61] = {0};
            auto seed = randomSeed();
            getPublicKeyFromSeed(seed.c_str(), publicKey);
            getIdentityFromPublicKey(publicKey, identity, false);
            if (isAddressValid(identity, pattern, isSufix)) {
                memcpy(result.seed, seed.c_str(), sizeof(result.seed));
                memcpy(result.identity, identity, sizeof(result.identity));
                found = true;
                break;
            }
            ++attemptsCounter;
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto durationSinceLastPrint = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastPrintTime).count();
            if (durationSinceLastPrint >= 1) {
                lastPrintTime = currentTime;
                const auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
                const auto attempts = static_cast<double>(attemptsCounter.load());
                const double seconds = static_cast<double>(elapsedTime) > 0.0
                                           ? static_cast<double>(elapsedTime)
                                           : 1.0;
                const double attemptsPerSecond = attempts / seconds;
                const double percentTried = static_cast<double>(attemptsCounter.load()) / static_cast<double>(estimatedAttempts) * 100.0;
                LOG("Attempts: %llu | Estimated Attempts: %llu (%.6f%%) | Attempts/s: %.2f | Elapsed Time: %llus\n",
                    attemptsCounter.load(), estimatedAttempts, percentTried, attemptsPerSecond, elapsedTime);
            }
        }
    };

    // Start multiple threads to speed up the search
    const unsigned int numThreads = std::min(vanityGenerationThreads, std::thread::hardware_concurrency());
    std::vector<std::thread> threads;
    VanityAddress result{};
    bool found = false;
    for (unsigned int i = 0; i < numThreads; ++i) {
        threads.emplace_back(generateThread, std::ref(result), pattern, isSufix, std::ref(found));
    }
    std::string patternStr = isSufix ? "*" + std::string(pattern) : std::string(pattern) + "*";
    printf("-------- Starting vanity address generation with %u threads | Pattern %s --------\n", numThreads, patternStr.c_str());
    for (auto& t : threads) {
        t.join();
    }

    printf("Found vanity address!\n");
    printf("Seed: %s\n", result.seed);
    printf("Identity: %s\n", result.identity);
    return result;
}