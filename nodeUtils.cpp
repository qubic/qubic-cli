#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "structs.h"
#include "connection.h"
#include "nodeUtils.h"
#include "logger.h"
#include "K12AndKeyUtil.h"
#include "keyUtils.h"
#include "walletUtils.h"
#include "qubicLogParser.h"

static CurrentTickInfo getTickInfoFromNode(const char* nodeIp, int nodePort)
{
	CurrentTickInfo result;
	memset(&result, 0, sizeof(CurrentTickInfo));
	auto qc = new QubicConnection(nodeIp, nodePort);
	struct {
        RequestResponseHeader header;
    } packet;
    if ( qc->error < 0 )
	return(result);
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_CURRENT_TICK_INFO);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
    	auto header = (RequestResponseHeader*)(data+ptr);
    	if (header->type() == RESPOND_CURRENT_TICK_INFO){
    		auto curTickInfo = (CurrentTickInfo*)(data + ptr + sizeof(RequestResponseHeader));
    		result = *curTickInfo;
    	}
        ptr+= header->size();
    }
    delete qc;
	return result;
}
uint32_t getTickNumberFromNode(const char* nodeIp, int nodePort)
{
    auto curTickInfo = getTickInfoFromNode(nodeIp, nodePort);
    return curTickInfo.tick;
}
void printTickInfoFromNode(const char* nodeIp, int nodePort)
{
	auto curTickInfo = getTickInfoFromNode(nodeIp, nodePort);
	if (curTickInfo.epoch != 0){
		LOG("Tick: %u\n", curTickInfo.tick);
		LOG("Epoch: %u\n", curTickInfo.epoch);
		LOG("Number Of Aligned Votes: %u\n", curTickInfo.numberOfAlignedVotes);
		LOG("Number Of Misaligned Votes: %u\n", curTickInfo.numberOfMisalignedVotes);	
	} else {
		LOG("Error while getting tick info from %s:%d\n", nodeIp, nodePort);
	}
}
static void getTickTransactions(const char* nodeIp, const int nodePort, const uint32_t requestedTick, int nTx,
                                std::vector<Transaction>& txs, //out
                                std::vector<TxhashStruct>* hashes, //out
                                std::vector<extraDataStruct>* extraData, // out
                                std::vector<SignatureStruct>* sigs // out
                                )
{
    txs.resize(0);
    if (hashes != nullptr)
    {
        hashes->resize(0);
    }
    if (extraData != nullptr)
    {
        extraData->resize(0);
    }
    if (sigs != nullptr)
    {
        sigs->resize(0);
    }

    struct {
        RequestResponseHeader header;
        RequestedTickTransactions txs;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_TICK_TRANSACTIONS); // REQUEST_TICK_TRANSACTIONS
    packet.txs.tick = requestedTick;
    for (int i = 0; i < (nTx+7)/8; i++) packet.txs.transactionFlags[i] = 0;
    for (int i = (nTx+7)/8; i < NUMBER_OF_TRANSACTIONS_PER_TICK/8; i++) packet.txs.transactionFlags[i] = 1;
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
        if (header->type() == BROADCAST_TRANSACTION){
            auto tx = (Transaction *)(data + ptr + sizeof(RequestResponseHeader));
            txs.push_back(*tx);
            if (hashes != nullptr){
                TxhashStruct hash;
                uint8_t digest[32] = {0};
                char txHash[128] = {0};
                KangarooTwelve(reinterpret_cast<const uint8_t *>(tx),
                               sizeof(Transaction) + tx->inputSize + SIGNATURE_SIZE,
                               digest,
                               32);
                getTxHashFromDigest(digest, txHash);
                memcpy(hash.hash, txHash, 60);
                hashes->push_back(hash);
            }
            if (extraData != nullptr){
                extraDataStruct ed;
                ed.vecU8.resize(tx->inputSize);
                if (tx->inputSize != 0){
                    memcpy(ed.vecU8.data(), reinterpret_cast<const uint8_t *>(tx) + sizeof(Transaction), tx->inputSize);
                }
                extraData->push_back(ed);
            }
            if (sigs != nullptr){
                SignatureStruct sig;
                memcpy(sig.sig, reinterpret_cast<const uint8_t *>(tx) + sizeof(Transaction) + tx->inputSize, 64);
                sigs->push_back(sig);
            }
        }
        ptr+= header->size();
    }
    delete qc;
}
static void getTickData(const char* nodeIp, const int nodePort, const uint32_t tick, TickData& result)
{
    memset(&result, 0, sizeof(TickData));
    static struct
    {
        RequestResponseHeader header;
        RequestTickData requestTickData;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_TICK_DATA);
    packet.requestTickData.requestedTickData.tick = tick;
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
        if (header->type() == BROADCAST_FUTURE_TICK_DATA){
            auto curTickData = (TickData*)(data + ptr + sizeof(RequestResponseHeader));
            result = *curTickData;
        }
        ptr+= header->size();
    }
    delete qc;
}
bool checkTxOnTick(const char* nodeIp, const int nodePort, const char* txHash, uint32_t requestedTick)
{
    // conditions:
    // - current Tick is higher than requested tick
    // - has tick data
    // - has txHash in tick transactions
    uint32_t currenTick = getTickNumberFromNode(nodeIp, nodePort);
    if (currenTick <= requestedTick)
    {
        LOG("Please wait a bit more. Requested tick %u, current tick %u\n", requestedTick, currenTick);
        return false;
    }
    TickData td;
    getTickData(nodeIp, nodePort, requestedTick, td);
    if (td.epoch == 0)
    {
        LOG("Tick %u is empty\n", requestedTick);
        return false;
    }
    int numTx = 0;
    uint8_t all_zero[32] = {0};
    for (int i = 0; i < NUMBER_OF_TRANSACTIONS_PER_TICK; i++){
        if (memcmp(all_zero, td.transactionDigests[i], 32) != 0) numTx++;
    }
    std::vector<Transaction> txs;
    std::vector<TxhashStruct> txHashesFromTick;
    std::vector<extraDataStruct> extraData;
    std::vector<SignatureStruct> signatureStruct;
    getTickTransactions(nodeIp, nodePort, requestedTick, numTx, txs, &txHashesFromTick, &extraData, &signatureStruct);
    for (int i = 0; i < txHashesFromTick.size(); i++)
    {
        if (memcmp(txHashesFromTick[i].hash, txHash, 60) == 0)
        {
            LOG("Found tx %s on tick %u\n", txHash, requestedTick);
            printReceipt(txs[i], txHash, extraData[i].vecU8.data());
            return true;
        }
    }
    LOG("Can NOT find tx %s on tick %u\n", txHash, requestedTick);
    return false;
}

void getTickDataToFile(const char* nodeIp, const int nodePort, uint32_t requestedTick, const char* fileName)
{
    uint32_t currenTick = getTickNumberFromNode(nodeIp, nodePort);
    if (currenTick <= requestedTick)
    {
        LOG("Please wait a bit more. Requested tick %u, current tick %u\n", requestedTick, currenTick);
        return;
    }
    TickData td;
    getTickData(nodeIp, nodePort, requestedTick, td);
    if (td.epoch == 0)
    {
        LOG("Tick %u is empty\n", requestedTick);
        return;
    }
    int numTx = 0;
    uint8_t all_zero[32] = {0};
    for (int i = 0; i < NUMBER_OF_TRANSACTIONS_PER_TICK; i++){
        if (memcmp(all_zero, td.transactionDigests[i], 32) != 0) numTx++;
    }
    std::vector<Transaction> txs;
    std::vector<extraDataStruct> extraData;
    std::vector<SignatureStruct> signatures;
    getTickTransactions(nodeIp, nodePort, requestedTick, numTx, txs, nullptr, &extraData, &signatures);

    FILE* f = fopen(fileName, "wb");
    fwrite(&td, 1, sizeof(TickData), f);
    for (int i = 0; i < txs.size(); i++)
    {
        fwrite(&txs[i], 1, sizeof(Transaction), f);
        int extraDataSize = txs[i].inputSize;
        if (extraDataSize != 0){
            fwrite(extraData[i].vecU8.data(), 1, extraDataSize, f);
        }
        fwrite(signatures[i].sig, 1, SIGNATURE_SIZE, f);
    }
    fclose(f);
    LOG("Tick data and tick transactions have been written to %s\n", fileName);
}

void readTickDataFromFile(const char* fileName, TickData& td,
                          std::vector<Transaction>& txs,
                          std::vector<extraDataStruct>* extraData,
                          std::vector<SignatureStruct>* signatures,
                          std::vector<TxhashStruct>* txHashes)
{
    uint8_t extraDataBuffer[1024] = {0};
    uint8_t signatureBuffer[128] = {0};
    char txHashBuffer[128] = {0};
    uint8_t digest[32] = {0};

    FILE* f = fopen(fileName, "rb");
    fread(&td, 1, sizeof(TickData), f);
    int numTx = 0;
    uint8_t all_zero[32] = {0};
    for (int i = 0; i < NUMBER_OF_TRANSACTIONS_PER_TICK; i++){
        if (memcmp(all_zero, td.transactionDigests[i], 32) != 0) numTx++;
    }
    for (int i = 0; i < numTx; i++){
        Transaction tx;
        fread(&tx, 1, sizeof(Transaction), f);
        int extraDataSize = tx.inputSize;
        if (extraData != nullptr){
            extraDataStruct eds;
            if (extraDataSize != 0){
                fread(extraDataBuffer, 1, extraDataSize, f);
                eds.vecU8.resize(extraDataSize);
                memcpy(eds.vecU8.data(), extraDataBuffer, extraDataSize);
            }
            extraData->push_back(eds);
        }

        fread(signatureBuffer, 1, SIGNATURE_SIZE, f);
        if (signatures != nullptr){
            SignatureStruct sig;
            memcpy(sig.sig, signatureBuffer, SIGNATURE_SIZE);
            signatures->push_back(sig);
        }
        if (txHashes != nullptr){
            std::vector<uint8_t> raw_data;
            raw_data.resize(sizeof(Transaction) + tx.inputSize + SIGNATURE_SIZE);
            auto ptr = raw_data.data();
            memcpy(ptr, &tx, sizeof(Transaction));
            memcpy(ptr + sizeof(Transaction), extraDataBuffer, tx.inputSize);
            memcpy(ptr + sizeof(Transaction) + tx.inputSize, signatureBuffer, SIGNATURE_SIZE);
            KangarooTwelve(ptr,
                           raw_data.size(),
                           digest,
                           32);
            TxhashStruct tx_hash;
            getTxHashFromDigest(digest, txHashBuffer);
            memcpy(tx_hash.hash, txHashBuffer, 60);
            txHashes->push_back(tx_hash);
        }
        txs.push_back(tx);
    }
    fclose(f);
}

BroadcastComputors readComputorListFromFile(const char* fileName);

void printTickDataFromFile(const char* fileName, const char* compFile)
{
    TickData td;
    std::vector<Transaction> txs;
    std::vector<extraDataStruct> extraData;
    std::vector<SignatureStruct> signatures;
    std::vector<TxhashStruct> txHashes;
    uint8_t digest[32];
    readTickDataFromFile(fileName, td, txs, &extraData, &signatures, &txHashes);
    //verifying everything
    BroadcastComputors bc;
    bc = readComputorListFromFile(compFile);
    if (bc.computors.epoch != td.epoch){
        LOG("Computor list epoch (%u) and tick data epoch (%u) are not matched\n", bc.computors.epoch, td.epoch);
    }
    int computorIndex = td.computorIndex;
    td.computorIndex ^= BROADCAST_FUTURE_TICK_DATA;
    KangarooTwelve(reinterpret_cast<const uint8_t *>(&td),
                   sizeof(TickData) - SIGNATURE_SIZE,
                   digest,
                   32);
    uint8_t* computorOfThisTick = bc.computors.publicKeys[computorIndex];
    if (verify(computorOfThisTick, digest, td.signature)){
        LOG("Tick is VERIFIED (signed by correct computor).\n");
    } else {
        LOG("Tick is NOT verified (not signed by correct computor).\n");
    }
    LOG("Epoch: %u\n", td.epoch);
    LOG("Tick: %u\n", td.tick);
    LOG("Computor index: %u\n", computorIndex);
    LOG("Datetime: %u-%u-%u %u:%u:%u.%u\n", td.day, td.month, td.year, td.hour, td.minute, td.second, td.millisecond);

    for (int i = 0; i < txs.size(); i++)
    {
        uint8_t* extraDataPtr = extraData[i].vecU8.empty() ? nullptr : extraData[i].vecU8.data();
        printReceipt(txs[i], txHashes[i].hash, extraDataPtr);
        if (verifyTx(txs[i], extraData[i].vecU8.data(), signatures[i].sig))
        {
            LOG("Transaction is VERIFIED\n");
        } else {
            LOG("Transaction is NOT VERIFIED. Incorrect signature\n");
        }
    }
}

bool checkTxOnFile(const char* txHash, const char* fileName)
{
    TickData td;
    std::vector<Transaction> txs;
    std::vector<extraDataStruct> extraData;
    std::vector<SignatureStruct> signatures;
    std::vector<TxhashStruct> txHashes;

    readTickDataFromFile(fileName, td, txs, &extraData, &signatures, &txHashes);

    for (int i = 0; i < txs.size(); i++)
    {
        if (memcmp(txHashes[i].hash, txHash, 60) == 0){
            LOG("Found tx %s on file %s\n", txHash, fileName);
            printReceipt(txs[i], txHash, extraData[i].vecU8.data());
            return true;
        }
    }
    LOG("Can NOT find tx %s on file %s\n", txHash, fileName);
    return false;
}

void sendRawPacket(const char* nodeIp, const int nodePort, int rawPacketSize, uint8_t* rawPacket)
{
    std::vector<uint8_t> buffer;
    auto qc = new QubicConnection(nodeIp, nodePort);
    qc->sendData(rawPacket, rawPacketSize);
    LOG("Sent %d bytes\n", rawPacketSize);
    qc->receiveDataAll(buffer);
    LOG("Received %d bytes\n", buffer.size());
    for (int i = 0; i < buffer.size(); i++){
        LOG("%02x", buffer[i]);
    }
    LOG("\n");
    delete qc;
}

void sendSpecialCommand(const char* nodeIp, const int nodePort, const char* seed, int command)
{
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t subseed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};

    struct {
        RequestResponseHeader header;
        SpecialCommand cmd;
        uint8_t signature[64];
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(PROCESS_SPECIAL_COMMAND);
    uint64_t curTime = time(NULL);
    uint64_t commandByte = (uint64_t)(command) << 56;
    packet.cmd.everIncreasingNonceAndCommandType = commandByte | curTime;

    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    KangarooTwelve((unsigned char*)&packet.cmd,
                   sizeof(packet.cmd),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    auto qc = new QubicConnection(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    delete qc;
    LOG("Send command %d to node %s\n", command, nodeIp);
}

BroadcastComputors readComputorListFromFile(const char* fileName)
{
    BroadcastComputors result;
    FILE* f = fopen(fileName, "rb");
    fread(&result, 1, sizeof(BroadcastComputors), f);
    fclose(f);
    uint8_t digest[32] = {0};
    uint8_t arbPubkey[32] = {0};
    // verify with arb
    getPublicKeyFromIdentity(ARBITRATOR, arbPubkey);
    KangarooTwelve(reinterpret_cast<const uint8_t *>(&result),
                   sizeof(BroadcastComputors) - SIGNATURE_SIZE,
                   digest,
                   32);
    if (verify(arbPubkey, digest, result.computors.signature)){
        LOG("Computor list is VERIFIED (signed by ARBITRATOR)\n");
    } else {
        LOG("Computor list is NOT verified\n");
    }
    return result;
}

bool getComputorFromNode(const char* nodeIp, const int nodePort, BroadcastComputors& result)
{
    static struct
    {
        RequestResponseHeader header;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_COMPUTORS);
    auto qc = new QubicConnection(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    bool okay = false;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == BROADCAST_COMPUTORS){
            auto bc = (BroadcastComputors*)(data + ptr + sizeof(RequestResponseHeader));
            result = *bc;
            okay = true;
        }
        ptr+= header->size();
    }
    delete qc;
    return okay;
}

void getComputorListToFile(const char* nodeIp, const int nodePort, const char* fileName)
{
    BroadcastComputors bc;
    if (!getComputorFromNode(nodeIp, nodePort, bc))
    {
        LOG("Failed to get valid computor list!");
        return;
    }
    uint8_t digest[32] = {0};
    uint8_t arbPubkey[32] = {0};
    // verify with arb, dump data
    getPublicKeyFromIdentity(ARBITRATOR, arbPubkey);
    for (int i = 0; i < NUMBER_OF_COMPUTORS; i++)
    {
        char identity[128] = {0};
        const bool isLowerCase = false;
        getIdentityFromPublicKey(bc.computors.publicKeys[i], identity, isLowerCase);
        LOG("%d %s\n", i, identity);
    }
    LOG("Epoch: %u\n", bc.computors.epoch);
    KangarooTwelve(reinterpret_cast<const uint8_t *>(&bc),
                  sizeof(BroadcastComputors) - SIGNATURE_SIZE,
                  digest,
                  32);
    if (verify(arbPubkey, digest, bc.computors.signature)){
        LOG("Computor list is VERIFIED (signed by ARBITRATOR)\n");
    } else {
        LOG("Computor list is NOT verified\n");
    }
    FILE* f = fopen(fileName, "wb");
    fwrite(&bc, 1, sizeof(BroadcastComputors), f);
    fclose(f);
}

std::vector<std::string> _getNodeIpList(const char* nodeIp, const int nodePort)
{
    std::vector<std::string> result;
    memset(&result, 0, sizeof(CurrentTickInfo));
    auto qc = new QubicConnection(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_CURRENT_TICK_INFO);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == EXCHANGE_PUBLIC_PEERS){
            auto epp = (ExchangePublicPeers*)(data + ptr + sizeof(RequestResponseHeader));
            for (int i = 0; i < 4; i++){
                std::string new_ip = std::to_string(epp->peers[i][0]) + "." + std::to_string(epp->peers[i][1]) + "." + std::to_string(epp->peers[i][2]) + "." + std::to_string(epp->peers[i][3]);
                result.push_back(new_ip);
            }
        }
        ptr+= header->size();
    }
    delete qc;
    return result;
}
void getNodeIpList(const char* nodeIp, const int nodePort)
{
    LOG("Fetching node ip list from %s\n", nodeIp);
    std::vector<std::string> result = _getNodeIpList(nodeIp, nodePort);
    int count = 0;
    for (int i = 0; i < result.size() && count++ < 4; i++){
        std::vector<std::string> new_result = _getNodeIpList(result[i].c_str(), nodePort);
        result.insert(result.end(), new_result.begin(), new_result.end());
    }
    std::sort(result.begin(), result.end());
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());
    for (auto s : result){
        LOG("%s\n", s.c_str());
    }
}

void getLogFromNode(const char* nodeIp, const int nodePort, uint64_t* passcode)
{
    struct {
        RequestResponseHeader header;
        unsigned long long passcode[4];
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestLog::type());
    memcpy(packet.passcode, passcode, 4 * sizeof(uint64_t));
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
        if (header->type() == RespondLog::type()){
            auto logBuffer = (uint8_t*)(data + ptr + sizeof(RequestResponseHeader));
            printQubicLog(logBuffer, header->size() - sizeof(RequestResponseHeader));
        }
        ptr+= header->size();
    }
    delete qc;
}
static bool isEmptyEntity(const Entity& e){
    bool is_pubkey_zero = true;
    for (int i = 0; i < 32; i++){
        if (e.publicKey[i] != 0){
            is_pubkey_zero = false;
            break;
        }
    }
    if (is_pubkey_zero) return true;
    if (e.outgoingAmount == 0 && e.incomingAmount == 0) return true;
    if (e.latestIncomingTransferTick == 0 && e.latestOutgoingTransferTick == 0) return true;
    return false;
}

void dumpSpectrumToCSV(const char* input, const char* output){
    const size_t SPECTRUM_CAPACITY = 0x1000000ULL; // may be changed in the future
    Entity* spectrum = (Entity*)malloc(SPECTRUM_CAPACITY*sizeof(Entity));
    FILE* f = fopen(input, "rb");
    fread(spectrum, 1, SPECTRUM_CAPACITY*sizeof(Entity), f);
    fclose(f);
    f = fopen(output, "w");
    {
        std::string header ="ID,LastInTick,LastOutTick,AmountIn,AmountOut,Balance\n";
        fwrite(header.c_str(), 1, header.size(), f);
    }
    char buffer[128] = {0};
    for (int i = 0; i < SPECTRUM_CAPACITY; i++){
        if (!isEmptyEntity(spectrum[i])){
            memset(buffer, 0, 128);
            getIdentityFromPublicKey(spectrum[i].publicKey, buffer, false);
            std::string id = buffer;
            std::string line = id + "," + std::to_string(spectrum[i].latestIncomingTransferTick)
                                  + "," + std::to_string(spectrum[i].latestOutgoingTransferTick)
                                  + "," + std::to_string(spectrum[i].incomingAmount)
                                  + "," + std::to_string(spectrum[i].outgoingAmount)
                                  + "," + std::to_string(spectrum[i].incomingAmount-spectrum[i].outgoingAmount) + "\n";
            fwrite(line.c_str(), 1, line.size(), f);
        }
    }
    free(spectrum);
    fclose(f);
}

//only print ownership
void dumpUniverseToCSV(const char* input, const char* output){
    const size_t ASSETS_CAPACITY = 0x1000000ULL; // may be changed in the future
    Asset* asset = (Asset*)malloc(ASSETS_CAPACITY*sizeof(Entity));
    FILE* f = fopen(input, "rb");
    fread(asset, 1, ASSETS_CAPACITY*sizeof(Asset), f);
    fclose(f);
    f = fopen(output, "w");
    {
        std::string header ="Index,Type,ID,OwnerIndex,ContractIndex,AssetName,AssetIssuer,Amount\n";
        fwrite(header.c_str(), 1, header.size(), f);
    }
    char buffer[128] = {0};
    for (int i = 0; i < ASSETS_CAPACITY; i++){
        if (asset[i].varStruct.ownership.type == OWNERSHIP){
            memset(buffer, 0, 128);
            getIdentityFromPublicKey(asset[i].varStruct.ownership.publicKey, buffer, false);
            std::string id = buffer;
            std::string asset_name = "null";
            std::string issuerID = "null";
            size_t issue_index = asset[i].varStruct.ownership.issuanceIndex;
            {
                //get asset name
                memset(buffer, 0, 128);
                memcpy(buffer, asset[issue_index].varStruct.issuance.name, 7);
                asset_name = buffer;
            }
            {
                //get issuer
                memset(buffer, 0, 128);
                getIdentityFromPublicKey(asset[issue_index].varStruct.issuance.publicKey, buffer, false);
                issuerID = buffer;
            }
            std::string line = std::to_string(i) + ",OWNERSHIP,"+ id
                                + "," + std::to_string(i) + ","
                                + std::to_string(asset[i].varStruct.ownership.managingContractIndex) + "," + asset_name
                                + "," + issuerID
                                + "," + std::to_string(asset[i].varStruct.ownership.numberOfUnits) + "\n";
            fwrite(line.c_str(), 1, line.size(), f);
        }
        if (asset[i].varStruct.ownership.type == POSSESSION){
            memset(buffer, 0, 128);
            getIdentityFromPublicKey(asset[i].varStruct.possession.publicKey, buffer, false);
            std::string id = buffer;
            std::string asset_name = "null";
            std::string issuerID = "null";
            std::string str_index = std::to_string(i);
            int owner_index = asset[i].varStruct.possession.ownershipIndex;
            int contract_index = asset[i].varStruct.possession.managingContractIndex;
            std::string str_owner_index = std::to_string(owner_index);
            std::string str_contract_index = std::to_string(contract_index);
            std::string str_amount = std::to_string(asset[i].varStruct.possession.numberOfUnits);
            {
                //get asset name
                int issuance_index = asset[owner_index].varStruct.ownership.issuanceIndex;
                memset(buffer, 0, 128);
                memcpy(buffer, asset[issuance_index].varStruct.issuance.name, 7);
                asset_name = buffer;
                memset(buffer, 0, 128);
                getIdentityFromPublicKey(asset[issuance_index].varStruct.issuance.publicKey, buffer, false);
                issuerID = buffer;
            }
            std::string line = str_index + ",POSSESSION," + id + "," + str_owner_index + "," +
                    str_contract_index + "," + asset_name + "," + issuerID + "," + str_amount + "\n";
            fwrite(line.c_str(), 1, line.size(), f);
        }
        if (asset[i].varStruct.ownership.type == ISSUANCE){
            memset(buffer, 0, 128);
            getIdentityFromPublicKey(asset[i].varStruct.issuance.publicKey, buffer, false);
            std::string id = buffer;
            std::string asset_name = "null";
            std::string issuerID = "null";
            std::string str_index = std::to_string(i);
            std::string str_owner_index = std::to_string(0);
            std::string str_contract_index = std::to_string(1); // don't know how to get this yet
            std::string str_amount = std::to_string(asset[i].varStruct.possession.numberOfUnits);
            {
                //get asset name
                memset(buffer, 0, 128);
                memcpy(buffer, asset[i].varStruct.issuance.name, 7);
                asset_name = buffer;
                memset(buffer, 0, 128);
                getIdentityFromPublicKey(asset[i].varStruct.issuance.publicKey, buffer, false);
                issuerID = buffer;
            }
//            std::string header ="Index,Type,ID,OwnerIndex,ContractIndex,AssetName,AssetIssuer,Amount\n";
            std::string line = str_index + ",ISSUANCE," + id + "," + str_owner_index + "," +
                               str_contract_index + "," + asset_name + "," + issuerID + "," + str_amount + "\n";
            fwrite(line.c_str(), 1, line.size(), f);
        }
    }
    free(asset);
    fclose(f);
}

static int32_t getQuorumTick(const char* nodeIp, const int nodePort, const uint32_t tick, struct QuorumData &result)
{
    memset(&result, 0, sizeof(struct QuorumData));
    static struct
    {
        RequestResponseHeader header;
        RequestQuorumTick requestQuorumTick;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_QUORUMTICK);
    packet.requestQuorumTick.requestedQuorumTick.tick = tick;
    auto qc = new QubicConnection(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int n=0,recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == BROADCAST_TICK)
        {
            memcpy(&result.Quorum[n],(Tick *)(data + ptr + sizeof(RequestResponseHeader)),sizeof(Tick));
            //fprintf(stderr,"%d %d tick.%d\n",n,result.Quorum[n].computorIndex,result.Quorum[n].tick);
            n++;
        }
        ptr += header->size();
    }
    //fprintf(stderr,"got %ld quorum bytes votes.%d\n",buffer.size(),n);
    delete qc;
    return(n);
}

int32_t cmptick(Tick *a,Tick *b)
{
    if ( a->epoch != b->epoch )
        return(-1);
    if ( a->tick != b->tick )
        return(-2);
    if ( a->millisecond != b->millisecond )
        return(-3);
    if ( a->second != b->second )
        return(-4);
    if ( a->minute != b->minute )
        return(-5);
    if ( a->hour != b->hour )
        return(-6);
    if ( a->day != b->day )
        return(-7);
    if ( a->month != b->month )
        return(-8);
    if ( a->year != b->year )
        return(-9);
    if ( a->prevResourceTestingDigest != b->prevResourceTestingDigest )
        return(-10);
    //if ( a->saltedResourceTestingDigest != b->saltedResourceTestingDigest )
    //    return(-11);
    if ( memcmp(a->prevSpectrumDigest,b->prevSpectrumDigest,sizeof(a->prevSpectrumDigest)) != 0 )
        return(-12);
    if ( memcmp(a->prevUniverseDigest,b->prevUniverseDigest,sizeof(a->prevUniverseDigest)) != 0 )
        return(-13);
    if ( memcmp(a->prevComputerDigest,b->prevComputerDigest,sizeof(a->prevComputerDigest)) != 0 )
        return(-14);
    //if ( memcmp(a->saltedSpectrumDigest,b->saltedSpectrumDigest,sizeof(a->saltedSpectrumDigest)) != 0 )
    //    return(-15);
    //if ( memcmp(a->saltedUniverseDigest,b->saltedUniverseDigest,sizeof(a->saltedUniverseDigest)) != 0 )
    //    return(-16);
    //if ( memcmp(a->saltedComputerDigest,b->saltedComputerDigest,sizeof(a->saltedComputerDigest)) != 0 )
    //    return(-17);
    if ( memcmp(a->transactionDigest,b->transactionDigest,sizeof(a->transactionDigest)) != 0 )
        return(-18);
    //if ( memcmp(a->expectedNextTickTransactionDigest,b->expectedNextTickTransactionDigest,sizeof(a->expectedNextTickTransactionDigest)) != 0 )
    //    return(-19);
    return(0);
}

int32_t getQuorumTickToFile(const char* nodeIp, const int nodePort, uint32_t requestedTick, const char* fileName)
{
    struct quorumdiff qdiffs[NUMBER_OF_COMPUTORS],*qp;
    FILE *fp;
    uint8_t bits[(NUMBER_OF_COMPUTORS+7)/8];
    int32_t i,n,err,computor;
    uint32_t currenTick = getTickNumberFromNode(nodeIp, nodePort);
    if (currenTick <= requestedTick)
    {
        //LOG("Please wait a bit more. Requested tick %u, current tick %u\n", requestedTick, currenTick);
        return(-1);
    }
    struct QuorumData qd;
    n = getQuorumTick(nodeIp, nodePort, requestedTick, qd);
    if ( n < 451 )
    {
        LOG("n.%d insufficient for quorum tick.%d\n",n,requestedTick);
        return(-1);
    }
    memset(qdiffs,0,sizeof(qdiffs));
    memset(bits,0,sizeof(bits));
    for (i=1; i<n; i++)
    {
        if ( (err= cmptick(&qd.Quorum[0],&qd.Quorum[i])) != 0 )
        {
            printf("i.%d of %d. cmperr.%d\n",i,n,err);
            return(-1);
        }
        else
        {
            computor = qd.Quorum[i].computorIndex;
            qp = &qdiffs[computor];
            //printf("%d ",computor);
            bits[computor >> 3] |= (1 << (computor & 7));
            qp->saltedResourceTestingDigest = qd.Quorum[i].saltedResourceTestingDigest;
            memcpy(qp->saltedSpectrumDigest,qd.Quorum[i].saltedSpectrumDigest,sizeof(qp->saltedSpectrumDigest));
            memcpy(qp->saltedUniverseDigest,qd.Quorum[i].saltedUniverseDigest,sizeof(qp->saltedUniverseDigest));
            memcpy(qp->saltedComputerDigest,qd.Quorum[i].saltedComputerDigest,sizeof(qp->saltedComputerDigest));
            memcpy(qp->expectedNextTickTransactionDigest,qd.Quorum[i].expectedNextTickTransactionDigest,sizeof(qp->expectedNextTickTransactionDigest));
            memcpy(qp->signature,qd.Quorum[i].signature,sizeof(qp->signature));
        }
    }
    //printf("computors n.%d first one.%d tick.%d\n",n,qd.Quorum[0].computorIndex,requestedTick);
    if ( (fp= fopen(fileName,"wb")) != 0 )
    {
        fwrite(&qd.Quorum[0],1,sizeof(qd.Quorum[i]),fp);
        fwrite(bits,1,sizeof(bits),fp);
        for (i=0; i<NUMBER_OF_COMPUTORS; i++)
        {
            if ( (bits[i >> 3] & (1 << (i & 7))) != 0 )
                fwrite(&qdiffs[i],1,sizeof(qdiffs[i]),fp);
        }
        fclose(fp);
        char fname2[512];
        sprintf(fname2,"%s.raw",fileName);
        if ( (fp=fopen(fname2,"wb")) != 0 )
        {
            fwrite(&qd.Quorum[0],n,sizeof(qd.Quorum[0]),fp);
            fclose(fp);
        }
    } else printf("error opening (%s)\n",fileName);
    //printf("saved computors n.%d\n",n);
    return(1);
}
