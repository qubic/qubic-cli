#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <memory>
#include <stdexcept>

#include "structs.h"
#include "connection.h"
#include "nodeUtils.h"
#include "logger.h"
#include "K12AndKeyUtil.h"
#include "keyUtils.h"
#include "walletUtils.h"

static CurrentTickInfo getTickInfoFromNode(QCPtr qc)
{
    CurrentTickInfo result;
    struct {
        RequestResponseHeader header;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_CURRENT_TICK_INFO);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<CurrentTickInfo>();
    } 
    catch (std::logic_error& e)
    {
        memset(&result, 0, sizeof(CurrentTickInfo));
    }

    return result;
}

uint32_t getTickNumberFromNode(QCPtr qc)
{
    auto curTickInfo = getTickInfoFromNode(qc);
    int retryCounter = 0;
    while (curTickInfo.tick == 0 && retryCounter++ < 3)
        curTickInfo = getTickInfoFromNode(qc);
    return curTickInfo.tick;
}

void printTickInfoFromNode(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    auto curTickInfo = getTickInfoFromNode(qc);
    if (curTickInfo.epoch != 0)
    {
        LOG("Tick: %u\n", curTickInfo.tick);
        LOG("Epoch: %u\n", curTickInfo.epoch);
        LOG("Number Of Aligned Votes: %u\n", curTickInfo.numberOfAlignedVotes);
        LOG("Number Of Misaligned Votes: %u\n", curTickInfo.numberOfMisalignedVotes);
        LOG("Initial tick: %u\n", curTickInfo.initialTick);
    }
    else
    {
        LOG("Error while getting tick info from %s:%d\n", nodeIp, nodePort);
    }
}

CurrentSystemInfo getSystemInfoFromNode(QCPtr qc)
{
    CurrentSystemInfo result;
    struct {
        RequestResponseHeader header;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_SYSTEM_INFO);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try 
    {
        result = qc->receivePacketWithHeaderAs<CurrentSystemInfo>();
    }
    catch (std::logic_error& e) 
    {
        memset(&result, 0, sizeof(CurrentSystemInfo));
    }

    return result;
}

void printSystemInfoFromNode(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    auto curSystemInfo = getSystemInfoFromNode(qc);
    if (curSystemInfo.epoch != 0)
    {
        LOG("Version: %u\n", curSystemInfo.version);
        LOG("Epoch: %u\n", curSystemInfo.epoch);
        LOG("Tick: %u\n", curSystemInfo.tick);
        LOG("InitialTick: %u\n", curSystemInfo.initialTick);
        LOG("LatestCreatedTick: %u\n", curSystemInfo.latestCreatedTick);
        LOG("NumberOfEntities: %u\n", curSystemInfo.numberOfEntities);
        LOG("TotalSpectrumAmount: %llu\n", curSystemInfo.totalSpectrumAmount);
        LOG("EntityBalanceDustThreshold: %llu\n", curSystemInfo.currentEntityBalanceDustThreshold);
        LOG("NumberOfTransactions: %u\n", curSystemInfo.numberOfTransactions);
        char hex[65];
        byteToHex(curSystemInfo.randomMiningSeed, hex, 32);
        LOG("RandomMiningSeed: %s\n", hex);
        LOG("SolutionThreshold: %u\n", curSystemInfo.solutionThreshold);

        // todo: add initial time

        LOG("\nAbout EntityBalanceDustThreshold: Entity balances less or euqal this value will be burned if number of entites rises to 75%% of spectrum capacity. Starts to be meaningful if >50%% of spectrum is filled but may still change after that.\n");
    }
    else
    {
        LOG("Error while getting system info from %s:%d\n", nodeIp, nodePort);
    }
}

static void getTickTransactions(QubicConnection* qc, const uint32_t requestedTick, int nTx,
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
    packet.header.setType(REQUEST_TICK_TRANSACTIONS);
    packet.txs.tick = requestedTick;
    for (int i = 0; i < (nTx+7)/8; i++) packet.txs.transactionFlags[i] = 0;
    for (int i = (nTx+7)/8; i < NUMBER_OF_TRANSACTIONS_PER_TICK/8; i++) packet.txs.transactionFlags[i] = 0xff;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    constexpr unsigned long long bufferSize = sizeof(RequestResponseHeader) + MAX_TRANSACTION_SIZE;
    uint8_t buffer[bufferSize];
    int recvByte = qc->receiveData(buffer, sizeof(RequestResponseHeader));
    int recvTx = 0;
    while (recvByte == sizeof(RequestResponseHeader))
    {
        auto header = (RequestResponseHeader*)buffer;
        if (header->type() == BROADCAST_TRANSACTION)
        {
            recvByte = qc->receiveAllDataOrThrowException(buffer + sizeof(RequestResponseHeader), sizeof(Transaction));
            auto tx = (Transaction*)(buffer + sizeof(RequestResponseHeader));
            txs.push_back(*tx);
            if (tx->inputSize > MAX_INPUT_SIZE)
            {
                LOG("Received tx with invalid inputSize!\n");
                exit(1);
            }
            ++recvTx;
            recvByte = qc->receiveAllDataOrThrowException(buffer + sizeof(RequestResponseHeader) + sizeof(Transaction), tx->inputSize + SIGNATURE_SIZE);
            if (hashes != nullptr)
            {
                TxhashStruct hash;
                uint8_t digest[32] = {0};
                char txHash[128] = {0};
                KangarooTwelve(reinterpret_cast<const uint8_t*>(tx),
                               sizeof(Transaction) + tx->inputSize + SIGNATURE_SIZE,
                               digest,
                               32);
                getTxHashFromDigest(digest, txHash);
                memcpy(hash.hash, txHash, 60);
                hashes->push_back(hash);
            }
            if (extraData != nullptr)
            {
                extraDataStruct ed;
                ed.vecU8.resize(tx->inputSize);
                if (tx->inputSize != 0)
                {
                    memcpy(ed.vecU8.data(), reinterpret_cast<const uint8_t*>(tx) + sizeof(Transaction), tx->inputSize);
                }
                extraData->push_back(ed);
            }
            if (sigs != nullptr)
            {
                SignatureStruct sig;
                memcpy(sig.sig, reinterpret_cast<const uint8_t*>(tx) + sizeof(Transaction) + tx->inputSize, SIGNATURE_SIZE);
                sigs->push_back(sig);
            }
        }
        if (recvTx == nTx)
            break;
        recvByte = qc->receiveData(buffer, sizeof(RequestResponseHeader));
    }

}

static void getTickData(const char* nodeIp, const int nodePort, const uint32_t tick, TickData& result)
{
    static struct
    {
        RequestResponseHeader header;
        RequestTickData requestTickData;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_TICK_DATA);
    packet.requestTickData.requestedTickData.tick = tick;
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<TickData>();
    }
    catch (std::logic_error& e) 
    {
        memset(&result, 0, sizeof(TickData));
    }
}

int getMoneyFlewStatus(QubicConnection* qc, const char* txHash, const uint32_t requestedTick)
{
    struct {
        RequestResponseHeader header;
        RequestTxStatus rts;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_TX_STATUS); // REQUEST_TX_STATUS
    packet.rts.tick = requestedTick;
    qc->sendData((uint8_t *) &packet, packet.header.size());
    RespondTxStatus result;
    try
    {
        result = qc->receivePacketWithHeaderAs<RespondTxStatus>();
        // notice: the node not always return full size of RESPOND_TX_STATUS
        // it only returns enough digests
        // -> set remainder in array memory which may contain junk to 0
        memset(result.txDigests[result.txCount], 0, (NUMBER_OF_TRANSACTIONS_PER_TICK - result.txCount) * 32);
    }
    catch (std::logic_error& e) 
    {
        memset(&result, 0, sizeof(RespondTxStatus));
        // it's expected to catch this error on some node that not turn on tx status
        return -1;
    }

    int tx_id = -1;
    for (int i = 0; i < result.txCount; i++)
    {
        char tx_hash[60];
        memset(tx_hash, 0, 60);
        getIdentityFromPublicKey(result.txDigests[i], tx_hash, true);
        if (memcmp(tx_hash, txHash, 60) == 0)
        {
            tx_id = i;
            break;
        }
    }
    if (tx_id == -1)
    {
        return -1; // not found !?
    }
    return (result.moneyFlew[tx_id >> 3] & (1<<(tx_id & 7))) ? 1 : 0;
}

bool checkTxOnTick(const char* nodeIp, const int nodePort, const char* txHash, uint32_t requestedTick)
{
    auto qc = std::make_shared<QubicConnection>(nodeIp, nodePort);
    // conditions:
    // - current Tick is higher than requested tick
    // - has tick data
    // - has txHash in tick transactions
    uint32_t currenTick = getTickNumberFromNode(qc);
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
    for (int i = 0; i < NUMBER_OF_TRANSACTIONS_PER_TICK; i++)
    {
        if (memcmp(all_zero, td.transactionDigests[i], 32) != 0) numTx++;
    }
    std::vector<Transaction> txs;
    std::vector<TxhashStruct> txHashesFromTick;
    std::vector<extraDataStruct> extraData;
    std::vector<SignatureStruct> signatureStruct;
    getTickTransactions(qc.get(), requestedTick, numTx, txs, &txHashesFromTick, &extraData, &signatureStruct);
    for (int i = 0; i < txHashesFromTick.size(); i++)
    {
        if (memcmp(txHashesFromTick[i].hash, txHash, 60) == 0)
        {
            LOG("Found tx %s on tick %u\n", txHash, requestedTick);
            // check for moneyflew status
            int moneyFlew = getMoneyFlewStatus(qc.get(), txHash, requestedTick);
            printReceipt(txs[i], txHash, extraData[i].vecU8.data(), moneyFlew);
            return true;
        }
    }
    LOG("Can NOT find tx %s on tick %u\n", txHash, requestedTick);
    return false;
}

// @return:
// - 0: ok
// - 1: hash doesn't exist
// - 2: failed connection
int _GetInputDataFromTxHash(QCPtr& qc, const char* txHash, uint8_t* outData, int& dataSize)
{
    char txUpperHash[61] = {0};
    for (int i = 0; txHash[i] != '\0'; ++i)
    {
        txUpperHash[i] = std::toupper(txHash[i]);
    }
    struct {
        RequestResponseHeader header;
        RequestedTransactionInfo txs;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_TRANSACTION_INFO);
    getPublicKeyFromIdentity(txUpperHash, packet.txs.transactionDigest);

    qc->sendData((uint8_t *) &packet, packet.header.size());

    // Received the respond and print the receipt
    bool receivedTx = false;
    constexpr unsigned long long bufferSize = sizeof(RequestResponseHeader) + MAX_TRANSACTION_SIZE;
    uint8_t buffer[bufferSize];
    int recvByte = qc->receiveData(buffer, bufferSize);
    if (recvByte > 0)
    {
        auto header = (RequestResponseHeader*)buffer;
        if (header->type() == BROADCAST_TRANSACTION)
        {
            auto tx = (Transaction*)(buffer + sizeof(RequestResponseHeader));
            uint8_t digest[32] = {0};
            char respondTxHash[61] = {0};
            KangarooTwelve(
                    reinterpret_cast<const uint8_t*>(tx),
                    sizeof(Transaction) + tx->inputSize + SIGNATURE_SIZE,
                    digest,
                    32);
            getTxHashFromDigest(digest, respondTxHash);
            // Check the digest of respond transaction
            if (memcmp(txHash, respondTxHash, 60) == 0)
            {
                extraDataStruct ed;
                ed.vecU8.resize(tx->inputSize);
                if (tx->inputSize != 0)
                {
                    memcpy(
                            ed.vecU8.data(),
                            reinterpret_cast<const uint8_t*>(tx) + sizeof(Transaction),
                            tx->inputSize);
                }

                receivedTx = true;
                memcpy(outData, ed.vecU8.data(), tx->inputSize);
                dataSize = tx->inputSize;
            }
        }
    }
    if (!receivedTx)
    {
        LOG("Failed to get tx info. Please check the tx hash and try again later.\n");
        return 1;
    }
    return 0;
}

// @return:
// - 0: ok
// - 1: hash doesn't exist
// - 2: failed connection
int _GetTxInfo(QCPtr& qc, const char* txHash)
{
    char txUpperHash[61] = {0};
    for (int i = 0; txHash[i] != '\0'; ++i)
    {
        txUpperHash[i] = std::toupper(txHash[i]);
    }
    struct {
        RequestResponseHeader header;
        RequestedTransactionInfo txs;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_TRANSACTION_INFO);
    getPublicKeyFromIdentity(txUpperHash, packet.txs.transactionDigest);

    qc->sendData((uint8_t *) &packet, packet.header.size());

    // Received the respond and print the receipt
    bool receivedTx = false;
    constexpr unsigned long long bufferSize = sizeof(RequestResponseHeader) + MAX_TRANSACTION_SIZE;
    uint8_t buffer[bufferSize];
    int recvByte = qc->receiveData(buffer, bufferSize);
    if (recvByte > 0)
    {
        auto header = (RequestResponseHeader*)(buffer);
        if (header->type() == BROADCAST_TRANSACTION)
        {
            auto tx = (Transaction*)(buffer + sizeof(RequestResponseHeader));
            uint8_t digest[32] = {0};
            char respondTxHash[61] = {0};
            KangarooTwelve(
                    reinterpret_cast<const uint8_t*>(tx),
                    sizeof(Transaction) + tx->inputSize + SIGNATURE_SIZE,
                    digest,
                    32);
            getTxHashFromDigest(digest, respondTxHash);
            // Check the digest of respond transaction
            if (memcmp(txHash, respondTxHash, 60) == 0)
            {
                extraDataStruct ed;
                ed.vecU8.resize(tx->inputSize);
                if (tx->inputSize != 0)
                {
                    memcpy(
                            ed.vecU8.data(),
                            reinterpret_cast<const uint8_t*>(tx) + sizeof(Transaction),
                            tx->inputSize);
                }

                receivedTx = true;
                printReceipt(*tx, respondTxHash, ed.vecU8.data(), -1);
            }
        }
    }
    if (!receivedTx)
    {
        LOG("Failed to get tx info. Please check the tx hash and try again later.\n");
        return 1;
    }
    return 0;
}

int getTxInfo(const char* nodeIp, const int nodePort, const char* txHash)
{
    auto qc = std::make_shared<QubicConnection>(nodeIp, nodePort);
    return _GetTxInfo(qc, txHash);
}

static void dumpQuorumTick(const Tick& A, bool dumpComputorIndex = true)
{
    char digest[64] = {0};
    if (dumpComputorIndex) LOG("Computor index: %d\n", A.computorIndex);
    LOG("Epoch: %d\n", A.epoch);
    LOG("Tick: %d\n", A.tick);
    LOG("Time: 20%02u-%02u-%02u %02u:%02u:%02u.%04u\n", A.year, A.month, A.day, A.hour, A.minute, A.second, A.millisecond);
    LOG("prevResourceTestingDigest: %llu\n", A.prevResourceTestingDigest);
    getIdentityFromPublicKey(A.prevSpectrumDigest, digest, true);
    LOG("prevSpectrumDigest: %s\n", digest);
    getIdentityFromPublicKey(A.prevUniverseDigest, digest, true);
    LOG("prevUniverseDigest: %s\n", digest);
    getIdentityFromPublicKey(A.prevComputerDigest, digest, true);
    LOG("prevComputerDigest: %s\n", digest);
    getIdentityFromPublicKey(A.transactionDigest, digest, true);
    LOG("transactionDigest: %s\n", digest);
    getIdentityFromPublicKey(A.expectedNextTickTransactionDigest, digest, true);
    LOG("expectedNextTickTransactionDigest: %s\n", digest);
}

bool compareVote(const Tick&A, const Tick&B)
{
    return (A.epoch == B.epoch) && (A.tick == B.tick) &&
           (A.year == B.year) && (A.month == B.month) && (A.day == B.day) && (A.hour == B.hour) && (A.minute == B.minute) && (A.second == B.second) &&
           (A.millisecond == B.millisecond) &&
           (A.prevResourceTestingDigest == B.prevResourceTestingDigest) &&
           (memcmp(A.prevSpectrumDigest, B.prevSpectrumDigest, 32) == 0) &&
           (memcmp(A.prevUniverseDigest, B.prevUniverseDigest, 32) == 0) &&
           (memcmp(A.prevComputerDigest, B.prevComputerDigest, 32) == 0) &&
           (memcmp(A.transactionDigest, B.transactionDigest, 32) == 0) &&
           (memcmp(A.expectedNextTickTransactionDigest, B.expectedNextTickTransactionDigest, 32) == 0);
}

bool verifyVoteWithSalt(const Tick&A,
                        const BroadcastComputors& bc,
                        const long long prevResourceDigest,
                        const uint8_t* prevSpectrumDigest,
                        const uint8_t* prevUniverseDigest,
                        const uint8_t* prevComputerDigest)
{
    int cid = A.computorIndex;
    uint8_t saltedData[64];
    uint8_t saltedDigest[32];
    memset(saltedData, 0, 64);
    memcpy(saltedData, bc.computors.publicKeys[cid], 32);
    memcpy(saltedData+32, &prevResourceDigest, 8);
    KangarooTwelve(saltedData, 40, saltedDigest, 8);
    if (A.saltedResourceTestingDigest != *((unsigned long long*)(saltedDigest)))
    {
        LOG("Mismatched saltedResourceTestingDigest. Computor index: %d\n", cid);
        return false;
    }
    memcpy(saltedData+32, prevSpectrumDigest, 32);
    KangarooTwelve(saltedData, 64, saltedDigest, 32);
    if (memcmp(saltedDigest, A.saltedSpectrumDigest, 32) != 0)
    {
        LOG("Mismatched saltedSpectrumDigest. Computor index: %d\n", cid);
        return false;
    }

    memcpy(saltedData+32, prevUniverseDigest, 32);
    KangarooTwelve(saltedData, 64, saltedDigest, 32);
    if (memcmp(saltedDigest, A.saltedUniverseDigest, 32) != 0)
    {
        LOG("Mismatched saltedUniverseDigest. Computor index: %d\n", cid);
        return false;
    }

    memcpy(saltedData+32, prevComputerDigest, 32);
    KangarooTwelve(saltedData, 64, saltedDigest, 32);
    if (memcmp(saltedDigest, A.saltedComputerDigest, 32) != 0)
    {
        LOG("Mismatched saltedComputerDigest. Computor index: %d\n", cid);
        return false;
    }
    return true;
}

std::string indexToAlphabet(int index){
    std::string result = "";
    result += char('A' + (index/26));
    result += char('A' + (index%26));
    return result;
}

void getUniqueVotes(std::vector<Tick>& votes, std::vector<Tick>& uniqueVote, std::vector<std::vector<int>>& voteIndices, int N,
                    bool verifySalt = false,
                    BroadcastComputors* pBC = nullptr,
                    const long long prevResourceDigest = 0,
                    const uint8_t* prevSpectrumDigest = nullptr,
                    const uint8_t* prevUniverseDigest = nullptr,
                    const uint8_t* prevComputerDigest = nullptr)
{
    if (votes.size() == 0) return;
    if (verifySalt)
    {
        std::vector<Tick> new_votes;
        LOG("Performing salt check...\n");
        bool all_passed = true;
        for (int i = 0; i < N; i++)
        {
            if (!verifyVoteWithSalt(votes[i], *pBC, prevResourceDigest, prevSpectrumDigest, prevUniverseDigest, prevComputerDigest))
            {
                LOG("Vote %d failed to pass salt check\n", i);
                dumpQuorumTick(votes[i]);
                all_passed = false;
            }
            else
            {
                new_votes.push_back(votes[i]);
            }
        }
        if (all_passed)
        {
            LOG("ALL votes PASSED salts check\n");
        }
        else
        {
            votes = new_votes;
        }
    }
    uniqueVote.resize(0);
    uniqueVote.push_back(votes[0]);
    voteIndices.resize(1);
    voteIndices[0].push_back(votes[0].computorIndex);
    for (int i = 1; i < votes.size(); i++)
    {
        int vote_indice = -1;
        for (int j = 0; j < uniqueVote.size(); j++)
        {
            if (compareVote(votes[i], uniqueVote[j]))
            {
                vote_indice = j;
                break;
            }
        }
        if (vote_indice != -1)
        {
            voteIndices[vote_indice].push_back(votes[i].computorIndex);
        }
        else
        {
            uniqueVote.push_back(votes[i]);
            voteIndices.resize(voteIndices.size() + 1);
            int M = voteIndices.size() -1;
            voteIndices[M].resize(0);
            voteIndices[M].push_back(votes[i].computorIndex);
        }
    }
}

void getQuorumTick(const char* nodeIp, const int nodePort, uint32_t requestedTick, const char* compFileName)
{
    auto qc = std::make_shared<QubicConnection>(nodeIp, nodePort);
    BroadcastComputors bc;
    {
        FILE* f = fopen(compFileName, "rb");
        if (fread(&bc, 1, sizeof(BroadcastComputors), f) != sizeof(BroadcastComputors))
        {
            LOG("Failed to read comp list\n");
            fclose(f);
            return;
        }
        fclose(f);
    }

    static struct
    {
        RequestResponseHeader header;
        RequestedQuorumTick rqt;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestedQuorumTick::type); // REQUEST_TICK_DATA
    packet.rqt.tick = requestedTick;
    memset(packet.rqt.voteFlags, 0, (676 + 7) / 8);
    qc->sendData(reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
    auto votes = qc->getLatestVectorPacketAs<Tick>();
    LOG("Received %d quorum tick #%u (votes)\n", votes.size(), requestedTick);

    packet.rqt.tick = requestedTick+1;
    memset(packet.rqt.voteFlags, 0, (676 + 7) / 8);
    qc->sendData(reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
    auto votes_next = qc->getLatestVectorPacketAs<Tick>();
    LOG("Received %d quorum tick #%u (votes)\n", votes_next.size(), requestedTick+1);

    int N = votes.size();
    if (N == 0)
    {
        return;
    }

    for (int i = 0; i < N; i++)
    {
        uint8_t digest[64] = {0};
        votes[i].computorIndex ^= Tick::type();
        KangarooTwelve((uint8_t*)&votes[i], sizeof(Tick) - SIGNATURE_SIZE, digest, 32);
        votes[i].computorIndex ^= Tick::type();
        int comp_index = votes[i].computorIndex;
        if (!verify(bc.computors.publicKeys[comp_index], digest, votes[i].signature))
        {
            LOG("Signature of vote %d is not correct\n", i);
            dumpQuorumTick(votes[i]);
            return;
        }
    }
    std::vector<Tick> uniqueVote, uniqueVoteNext;
    std::vector<std::vector<int>> voteIndices, voteIndicesNext;
    getUniqueVotes(votes_next, uniqueVoteNext, voteIndicesNext, N);
    if (votes_next.size() < 451)
    {
        printf("Failed to get votes for tick %d, this will not perform salt check\n", requestedTick+1);
        getUniqueVotes(votes, uniqueVote, voteIndices, N);
    }
    else
    {
        int max_id = 0;
        for (int i = 1; i < uniqueVote.size(); i++)
        {
            if (voteIndicesNext[max_id].size() < voteIndicesNext[i].size())
            {
                max_id = i;
            }
        }
        auto vote_next = uniqueVoteNext[max_id];
        getUniqueVotes(votes, uniqueVote, voteIndices, N, true, &bc,
                       vote_next.prevResourceTestingDigest,
                       vote_next.prevSpectrumDigest,
                       vote_next.prevUniverseDigest,
                       vote_next.prevComputerDigest);
    }

    LOG("Number of unique votes: %d\n", uniqueVote.size());
    for (int i = 0; i < uniqueVote.size(); i++)
    {
        LOG("Vote #%d (voted by %d computors ID) ", i, voteIndices[i].size());
        const bool dumpComputorIndex = false;
        dumpQuorumTick(uniqueVote[i], dumpComputorIndex);
        LOG("Voted by: ");
        std::sort(voteIndices[i].begin(), voteIndices[i].end());
        for (int j = 0; j < voteIndices[i].size(); j++)
        {
            int index = voteIndices[i][j];
            auto alphabet = indexToAlphabet(index);
            if (j < voteIndices[i].size() - 1)
            {
                LOG("%d(%s), ", index, alphabet.c_str());
            }
            else
            {
                LOG("%d(%s)\n", index, alphabet.c_str());
            }
        }
    }
}

void getTickDataToFile(const char* nodeIp, const int nodePort, uint32_t requestedTick, const char* fileName)
{
    auto qc = std::make_shared<QubicConnection>(nodeIp, nodePort);
    uint32_t currenTick = getTickNumberFromNode(qc);
    TickData td;
    getTickData(nodeIp, nodePort, requestedTick, td);
    if (td.epoch == 0)
    {
        LOG("Tick %u is empty\n", requestedTick);
        return;
    }
    int numTx = 0;
    uint8_t all_zero[32] = {0};
    for (int i = 0; i < NUMBER_OF_TRANSACTIONS_PER_TICK; i++)
    {
        if (memcmp(all_zero, td.transactionDigests[i], 32) != 0) numTx++;
    }
    std::vector<Transaction> txs;
    std::vector<extraDataStruct> extraData;
    std::vector<SignatureStruct> signatures;
    getTickTransactions(qc.get(), requestedTick, numTx, txs, nullptr, &extraData, &signatures);

    FILE* f = fopen(fileName, "wb");
    fwrite(&td, 1, sizeof(TickData), f);
    for (int i = 0; i < txs.size(); i++)
    {
        fwrite(&txs[i], 1, sizeof(Transaction), f);
        int extraDataSize = txs[i].inputSize;
        if (extraDataSize != 0)
        {
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
    LOG("List of transactions on tickData (correct order):\n");
    for (int i = 0; i < NUMBER_OF_TRANSACTIONS_PER_TICK; i++)
    {
        if (memcmp(all_zero, td.transactionDigests[i], 32) != 0)
        {
            numTx++;
            char digestHex[65];
            byteToHex(td.transactionDigests[i], digestHex, 32);
            LOG("%s\n", digestHex);
        }
    }
    std::vector<uint8_t> vDigests;
    vDigests.resize(32*numTx);
    for (int i = 0; i < numTx; i++)
    {
        Transaction tx;
        fread(&tx, 1, sizeof(Transaction), f);
        int extraDataSize = tx.inputSize;
        if (extraData != nullptr)
        {
            extraDataStruct eds;
            if (extraDataSize != 0)
            {
                fread(extraDataBuffer, 1, extraDataSize, f);
                eds.vecU8.resize(extraDataSize);
                memcpy(eds.vecU8.data(), extraDataBuffer, extraDataSize);
            }
            extraData->push_back(eds);
        }

        fread(signatureBuffer, 1, SIGNATURE_SIZE, f);
        if (signatures != nullptr)
        {
            SignatureStruct sig;
            memcpy(sig.sig, signatureBuffer, SIGNATURE_SIZE);
            signatures->push_back(sig);
        }
        {
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
            memcpy(vDigests.data() + i * 32, digest, 32);
        }
        if (txHashes != nullptr)
        {
            TxhashStruct tx_hash;
            getTxHashFromDigest(digest, txHashBuffer);
            memcpy(tx_hash.hash, txHashBuffer, 60);
            txHashes->push_back(tx_hash);
        }
        txs.push_back(tx);
    }

    // put in correct order by tickdata
    std::vector<Transaction> _txs; _txs.resize(numTx);
    std::vector<extraDataStruct> _extraData;
    std::vector<SignatureStruct> _signatures;
    std::vector<TxhashStruct> _txHashes;
    if (extraData != nullptr) _extraData.resize(numTx);
    if (signatures != nullptr) _signatures.resize(numTx);
    if (txHashes != nullptr) _txHashes.resize(numTx);
    for (int i = 0; i < numTx; i++)
    {
        for (int j = 0; j < numTx; j++)
        {
            // guaranteed by the protocol, if any duplicated here it's a bug on core side
            if (memcmp(vDigests.data() + j * 32, td.transactionDigests[i], 32) == 0)
            {
                _txs[i] = txs[j];
                if (extraData != nullptr) _extraData[i] =  (*extraData)[j];
                if (signatures != nullptr) _signatures[i] = (*signatures)[j];
                if (txHashes != nullptr) _txHashes[i] = (*txHashes)[j];
            }
        }
    }
    txs = _txs;
    if (extraData != nullptr) (*extraData) = _extraData;
    if (signatures != nullptr) (*signatures) = _signatures;
    if (txHashes != nullptr) (*txHashes) = _txHashes;
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
    if (bc.computors.epoch != td.epoch)
    {
        LOG("Computor list epoch (%u) and tick data epoch (%u) are not matched\n", bc.computors.epoch, td.epoch);
    }
    KangarooTwelve((uint8_t*)&td, sizeof(TickData), digest, 32);
    {
        char tddigest[61]={0};
        getIdentityFromPublicKey(digest, tddigest, true);
        LOG("Tickdata: %s\n", tddigest);
    }
    int computorIndex = td.computorIndex;
    td.computorIndex ^= BROADCAST_FUTURE_TICK_DATA;
    KangarooTwelve(reinterpret_cast<const uint8_t *>(&td),
                   sizeof(TickData) - SIGNATURE_SIZE,
                   digest,
                   32);
    uint8_t* computorOfThisTick = bc.computors.publicKeys[computorIndex];
    if (verify(computorOfThisTick, digest, td.signature))
    {
        char computorID[61] = {0};
        getIdentityFromPublicKey(computorOfThisTick, computorID, false);
        LOG("Tick is VERIFIED (signed by correct computor).\n");
        LOG("Comp ID: %s\n", computorID);
    }
    else
    {
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
        }
        else
        {
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
        if (memcmp(txHashes[i].hash, txHash, 60) == 0)
        {
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
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData(rawPacket, rawPacketSize);
    LOG("Sent %d bytes\n", rawPacketSize);
    RequestResponseHeader header;
    uint8_t* headerPtr = (uint8_t*)&header;
    qc->receiveData(headerPtr, sizeof(RequestResponseHeader));
    std::vector<uint8_t> buffer;
    if (header.size() > sizeof(RequestResponseHeader))
    {
        unsigned long long remainingSize = header.size() - sizeof(RequestResponseHeader);
        buffer.resize(remainingSize);
        qc->receiveData(buffer.data(), remainingSize);
    }
    LOG("Received %d bytes\n", header.size());
    for (int i = 0; i < sizeof(RequestResponseHeader); ++i)
    {
        LOG("%02x", headerPtr[i]);
    }
    for (int i = 0; i < buffer.size(); ++i)
    {
        LOG("%02x", buffer[i]);
    }
    LOG("\n");
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
    uint64_t curTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    SpecialCommand response;
    try
    {
        response = qc->receivePacketWithHeaderAs<SpecialCommand>();
    }
    catch (std::logic_error& e)
    {
        memset(&response, 0, sizeof(SpecialCommand));
        if (command != SPECIAL_COMMAND_REFRESH_PEER_LIST)
        {
            LOG("%s\n", e.what());
            return;
        }
    }

    if (response.everIncreasingNonceAndCommandType == packet.cmd.everIncreasingNonceAndCommandType)
    {
        LOG("Node received special command\n");
    } 
    else
    {
        if (command != SPECIAL_COMMAND_REFRESH_PEER_LIST)
        {
            LOG("Failed to send special command\n");
        } 
        else 
        {
            LOG("Sent special command\n"); // the connection is refreshed right after this command, no way to verify remotely
        }
    }
}

void toggleMainAux(const char* nodeIp, const int nodePort, const char* seed,
                   int command, std::string mode0, std::string mode1)
{
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t subseed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};

    struct {
        RequestResponseHeader header;
        SpecialCommandToggleMainModeResquestAndResponse cmd;
        uint8_t signature[64];
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(PROCESS_SPECIAL_COMMAND);
    uint64_t curTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t commandByte = (uint64_t)(command) << 56;
    packet.cmd.everIncreasingNonceAndCommandType = commandByte | curTime;
    uint8_t flag = 0;
    if (mode0 == "MAIN") flag |= 1;
    if (mode1 == "MAIN") flag |= 2;
    packet.cmd.mainModeFlag = flag;
    memset(packet.cmd.padding, 0, 7);

    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    KangarooTwelve((unsigned char*)&packet.cmd,
                   sizeof(packet.cmd),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    SpecialCommandToggleMainModeResquestAndResponse response;
    try
    {
        response = qc->receivePacketWithHeaderAs<SpecialCommandToggleMainModeResquestAndResponse>();
    }
    catch (std::logic_error& e) 
    {
        memset(&response, 0, sizeof(SpecialCommandToggleMainModeResquestAndResponse));
    }

    if (response.everIncreasingNonceAndCommandType == packet.cmd.everIncreasingNonceAndCommandType)
    {
        if (response.mainModeFlag == packet.cmd.mainModeFlag)
        {
            LOG("Successfully set MAINAUX flag\n");
        } 
        else 
        {
            LOG("The packet is successfully sent but failed set MAINAUX flag\n");
        }
    }
    else
    {
        LOG("Failed set MAINAUX flag\n");
    }
}

void setSolutionThreshold(const char* nodeIp, const int nodePort, const char* seed,
                          int command, int epoch, int threshold)
{
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t subseed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};

    struct {
        RequestResponseHeader header;
        SpecialCommandSetSolutionThresholdResquestAndResponse cmd;
        uint8_t signature[64];
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(PROCESS_SPECIAL_COMMAND);
    uint64_t curTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t commandByte = (uint64_t)(command) << 56;
    packet.cmd.everIncreasingNonceAndCommandType = commandByte | curTime;
    packet.cmd.epoch = epoch;
    packet.cmd.threshold = threshold;

    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    KangarooTwelve((unsigned char*)&packet.cmd,
                   sizeof(packet.cmd),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    SpecialCommandSetSolutionThresholdResquestAndResponse response;
    try
    {
        response = qc->receivePacketWithHeaderAs<SpecialCommandSetSolutionThresholdResquestAndResponse>();
    }
    catch (std::logic_error& e) 
    {
        memset(&response, 0, sizeof(SpecialCommandSetSolutionThresholdResquestAndResponse));
    }

    if (response.everIncreasingNonceAndCommandType == packet.cmd.everIncreasingNonceAndCommandType)
    {
        if (response.epoch == packet.cmd.epoch && response.threshold == packet.cmd.threshold)
        {
            LOG("Successfully set solution threshold\n");
        } 
        else 
        {
            LOG("The packet is successfully sent but failed set solution threshold\n");
        }
    } 
    else
    {
        LOG("Failed set solution threshold\n");
    }
}

void logTime(const UtcTime& time)
{
    LOG("%u-%02u-%02u %02u:%02u:%02u.%09u", time.year, time.month, time.day, time.hour, time.minute, time.second, time.nanosecond);
}

UtcTime convertTime(std::chrono::system_clock::time_point time)
{
    using namespace std;
    using namespace std::chrono;

    time_t tt = system_clock::to_time_t(time);
    tm utc_tm = *gmtime(&tt);
    UtcTime utcTime;
    utcTime.year = utc_tm.tm_year + 1900;
    utcTime.month = utc_tm.tm_mon + 1;
    utcTime.day = utc_tm.tm_mday;
    utcTime.hour = utc_tm.tm_hour;
    utcTime.minute = utc_tm.tm_min;
    utcTime.second = utc_tm.tm_sec;

    // get nanoseconds
    typedef duration<int, ratio_multiply<hours::period, ratio<24> >::type> days;
    system_clock::duration tp = time.time_since_epoch();
    days d = duration_cast<days>(tp);
    tp -= d;
    hours h = duration_cast<hours>(tp);
    tp -= h;
    minutes m = duration_cast<minutes>(tp);
    tp -= m;
    seconds s = duration_cast<seconds>(tp);
    tp -= s;
    utcTime.nanosecond = duration_cast<nanoseconds>(tp).count();

    return utcTime;
}

void syncTime(const char* nodeIp, const int nodePort, const char* seed)
{
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t subseed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    LOG("---------------------------------------------------------------------------------\n");
    LOG("This sets the node clock to roughly be in sync with the local clock.\n");
    LOG("CAUTION: MAKE SURE THAT YOUR LOCAL CLOCK IS SET CORRECTLY, FOR EXAMPLE USING NTP.\n");
    LOG("---------------------------------------------------------------------------------\n\n");

    auto qc = make_qc(nodeIp, nodePort);

    using namespace std::chrono;
    // get time from node and measure round trip time
    unsigned long long roundTripTimeNanosec = 0;
    unsigned long long finalizePacketTimeNanosec = 0;
    {
        LOG("Querying node time ...\n\n");

        struct {
            RequestResponseHeader header;
            SpecialCommand cmd;
            uint8_t signature[64];
        } queryTimeMsg;
        queryTimeMsg.header.setSize(sizeof(queryTimeMsg));
        queryTimeMsg.header.randomizeDejavu();
        queryTimeMsg.header.setType(PROCESS_SPECIAL_COMMAND);
        uint64_t curTime = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        uint64_t commandByte = (uint64_t)(SPECIAL_COMMAND_QUERY_TIME) << 56;
        queryTimeMsg.cmd.everIncreasingNonceAndCommandType = commandByte | curTime;
        LOG("Current unix time (in us) used for nonce (query time): %llu\n\n", curTime);

        // we need to measure the time it takes to finalize the packet because these steps are also executed after the sendTime packet is created later
        auto finalizePacketStartTime = steady_clock::now();

        KangarooTwelve((unsigned char*)&queryTimeMsg.cmd,
                       sizeof(queryTimeMsg.cmd),
                       digest,
                       32);
        sign(subseed, sourcePublicKey, digest, signature);
        memcpy(queryTimeMsg.signature, signature, 64);

        auto startTime = steady_clock::now();

        qc->sendData((uint8_t*)&queryTimeMsg, queryTimeMsg.header.size());
        
        SpecialCommandSendTime response;
        try
        {
            response = qc->receivePacketWithHeaderAs<SpecialCommandSendTime>();
        }
        catch (std::logic_error& e) 
        {
            memset(&response, 0, sizeof(SpecialCommandSendTime));
        }        
        
        auto endTime = steady_clock::now();
        auto nowLocal = system_clock::now();

        if ((response.everIncreasingNonceAndCommandType & 0xFFFFFFFFFFFFFF) != (queryTimeMsg.cmd.everIncreasingNonceAndCommandType & 0xFFFFFFFFFFFFFF)) 
        {
            LOG("Failed to query node time!\n");
            return;
        }

        finalizePacketTimeNanosec = duration<unsigned long long, std::nano>(startTime - finalizePacketStartTime).count();
        roundTripTimeNanosec = duration<unsigned long long, std::nano>(endTime - startTime).count();
        LOG("Clock status before sync:\n");
        LOG("\tNode time (UTC):  "); logTime(response.utcTime); LOG("  -  round trip time %llu ms\n", roundTripTimeNanosec / 1000000);
        LOG("\tLocal time (UTC): "); logTime(convertTime(nowLocal)); LOG("\n\n");
    }

    if (roundTripTimeNanosec > 3000000000llu)
    {
        LOG("Round trip time is too large. Sync skipped, because it would be very inaccurate!");
        return;
    }

    // set node clock to local UTC time + finalize packet time + half round trip time (not very accurate but simple and sufficient for requirements of 5 seconds)
    {
        struct {
            RequestResponseHeader header;
            SpecialCommandSendTime cmd;
            uint8_t signature[64];
        } sendTimeMsg;
        sendTimeMsg.header.setSize(sizeof(sendTimeMsg));
        sendTimeMsg.header.randomizeDejavu();
        sendTimeMsg.header.setType(PROCESS_SPECIAL_COMMAND);
        uint64_t curTime = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        uint64_t commandByte = (uint64_t)(SPECIAL_COMMAND_SEND_TIME) << 56;
        sendTimeMsg.cmd.everIncreasingNonceAndCommandType = commandByte | curTime;
        LOG("Current unix time (in us) used for nonce (send time): %llu\n\n", curTime);

        auto finalizePacketTime = duration_cast<system_clock::duration>(nanoseconds(finalizePacketTimeNanosec));
        auto halfRoudTripTime = duration_cast<system_clock::duration>(nanoseconds(roundTripTimeNanosec / 2));
        auto now = system_clock::now();
        UtcTime timeToSet = convertTime(now + finalizePacketTime + halfRoudTripTime);
        sendTimeMsg.cmd.utcTime = timeToSet;
        LOG("Setting node time to "); logTime(timeToSet); LOG(" ...\n\n");

        auto finalizePacketStartTime = steady_clock::now();

        KangarooTwelve((unsigned char*)&sendTimeMsg.cmd,
                       sizeof(sendTimeMsg.cmd),
                       digest,
                       32);
        sign(subseed, sourcePublicKey, digest, signature);
        memcpy(sendTimeMsg.signature, signature, 64);

        auto startTime = steady_clock::now();
        
        qc->sendData((uint8_t*)&sendTimeMsg, sendTimeMsg.header.size());

        SpecialCommandSendTime response;
        try
        {
            response = qc->receivePacketWithHeaderAs<SpecialCommandSendTime>();
        }
        catch (std::logic_error& e) 
        {
            memset(&response, 0, sizeof(SpecialCommandSendTime));
        }

        auto endTime = steady_clock::now();
        auto nowLocal = system_clock::now();

        if (response.everIncreasingNonceAndCommandType != sendTimeMsg.cmd.everIncreasingNonceAndCommandType) 
        {
            LOG("Failed to set node time!\n");
            return;
        }

        finalizePacketTimeNanosec = duration<unsigned long long, std::nano>(startTime - finalizePacketStartTime).count();
        roundTripTimeNanosec = duration<unsigned long long, std::nano>(endTime - startTime).count();
        LOG("Clock status after sync:\n");
        LOG("\tNode time (UTC):  "); logTime(response.utcTime); LOG("  -  round trip time %llu ms\n", roundTripTimeNanosec / 1000000);
        LOG("\tLocal time (UTC): "); logTime(convertTime(nowLocal)); LOG("\n\n");
    }
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
    if (verify(arbPubkey, digest, result.computors.signature))
    {
        LOG("Computor list is VERIFIED (signed by ARBITRATOR)\n");
    } 
    else 
    {
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
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<BroadcastComputors>();
        return true;
    }
    catch (std::logic_error& e) 
    {
        memset(&result, 0, sizeof(BroadcastComputors));
        return false;
    }
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
    if (verify(arbPubkey, digest, bc.computors.signature))
    {
        LOG("Computor list is VERIFIED (signed by ARBITRATOR)\n");
    } 
    else 
    {
        LOG("Computor list is NOT verified\n");
    }
    FILE* f = fopen(fileName, "wb");
    fwrite(&bc, 1, sizeof(BroadcastComputors), f);
    fclose(f);
}

std::vector<std::string> _getNodeIpList(const char* nodeIp, const int nodePort)
{
    std::vector<std::string> result;
    QCPtr qc;
    try
    {
        qc = make_qc(nodeIp, nodePort);
    } 
    catch (std::logic_error& e)
    {
        return result;
    }

    struct {
        RequestResponseHeader header;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    std::vector<uint8_t> buffer;
    qc->getHandshakeData(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    if (recvByte == 0)
    {
        return result;
    }
    auto epp = (ExchangePublicPeers*)(data);
    for (int i = 0; i < 4; i++)
    {
        if (epp->peers[i][0] == 0 && epp->peers[i][1] == 0 && epp->peers[i][2] == 0 && epp->peers[i][3] == 0) continue;
        std::string new_ip = std::to_string(epp->peers[i][0]) + "." + std::to_string(epp->peers[i][1]) + "." + std::to_string(epp->peers[i][2]) + "." + std::to_string(epp->peers[i][3]);
        result.push_back(new_ip);
    }
    return result;
}

void getNodeIpList(const char* nodeIp, const int nodePort)
{
    LOG("Fetching node ip list from %s\n", nodeIp);
    std::vector<std::string> result = _getNodeIpList(nodeIp, nodePort);
    int count = 0;
    for (int i = 0; i < result.size() && count++ < 4; i++)
    {
        std::vector<std::string> new_result = _getNodeIpList(result[i].c_str(), nodePort);
        result.insert(result.end(), new_result.begin(), new_result.end());
    }
    std::sort(result.begin(), result.end());
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());
    for (auto s : result)
    {
        LOG("%s\n", s.c_str());
    }
}

static bool isEmptyEntity(const Entity& e){
    bool is_pubkey_zero = true;
    for (int i = 0; i < 32; i++)
    {
        if (e.publicKey[i] != 0)
        {
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
    for (int i = 0; i < SPECTRUM_CAPACITY; i++)
    {
        if (!isEmptyEntity(spectrum[i]))
        {
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

// only print ownership
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
    for (int i = 0; i < ASSETS_CAPACITY; i++)
    {
        if (asset[i].varStruct.ownership.type == OWNERSHIP)
        {
            memset(buffer, 0, 128);
            getIdentityFromPublicKey(asset[i].varStruct.ownership.publicKey, buffer, false);
            std::string id = buffer;
            std::string asset_name = "null";
            std::string issuerID = "null";
            size_t issue_index = asset[i].varStruct.ownership.issuanceIndex;
            {
                // get asset name
                memset(buffer, 0, 128);
                memcpy(buffer, asset[issue_index].varStruct.issuance.name, 7);
                asset_name = buffer;
            }
            {
                // get issuer
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
        if (asset[i].varStruct.ownership.type == POSSESSION)
        {
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
                // get asset name
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
        if (asset[i].varStruct.ownership.type == ISSUANCE)
        {
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
                // get asset name
                memset(buffer, 0, 128);
                memcpy(buffer, asset[i].varStruct.issuance.name, 7);
                asset_name = buffer;
                memset(buffer, 0, 128);
                getIdentityFromPublicKey(asset[i].varStruct.issuance.publicKey, buffer, false);
                issuerID = buffer;
            }
            // std::string header ="Index,Type,ID,OwnerIndex,ContractIndex,AssetName,AssetIssuer,Amount\n";
            std::string line = str_index + ",ISSUANCE," + id + "," + str_owner_index + "," +
                               str_contract_index + "," + asset_name + "," + issuerID + "," + str_amount + "\n";
            fwrite(line.c_str(), 1, line.size(), f);
        }
    }
    free(asset);
    fclose(f);
}

void sendSpecialCommandGetMiningScoreRanking(const char* nodeIp, const int nodePort, const char* seed, int command)
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
    uint64_t curTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t commandByte = (uint64_t)(SPECIAL_COMMAND_GET_MINING_SCORE_RANKING) << 56;
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
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    SpecialCommandGetMiningScoreRanking response;

    int headerSize = sizeof(RequestResponseHeader) + 8 + 4; // header and everIncreasingNonceAndCommandType and numberOfRankings
    std::vector<uint8_t> buffer;
    buffer.resize(headerSize);
    qc->receiveData(buffer.data(), headerSize);
    int contentSize = 0;
    {
        uint8_t* data = buffer.data() + sizeof(RequestResponseHeader);
        response.everIncreasingNonceAndCommandType = *((unsigned long long*)data);
        response.numRankings = *((unsigned int*)(data+8));
        contentSize = response.numRankings * 36; // 32 pubkey + 4 bytes score
    }
    if (response.everIncreasingNonceAndCommandType != packet.cmd.everIncreasingNonceAndCommandType)
    {
        LOG("Failed to get mining score ranking!\n");
        return;
    }
    if (contentSize < 676 * 36)
    {
        LOG("Malformed content size\n");
        return;
    }
    buffer.resize(contentSize);
    qc->receiveAllDataOrThrowException(buffer.data(), contentSize);
    uint8_t* data = buffer.data();
    // Get data out
    unsigned char* ptr = data;

    // Get the number of miners
    LOG("Total miner: %u\n", response.numRankings);
    response.rankings.resize(response.numRankings);

    // Get detail ranking score of miners
    unsigned long long total_score = 0;
    if (response.numRankings > 0)
    {
        memcpy(response.rankings.data(), ptr, response.numRankings * sizeof(SpecialCommandGetMiningScoreRanking::ScoreEntry));
        LOG("%-8s%-64s%s\n", "ArrayIndex", "Identity", "Score");
        for (unsigned int i = 0; i < response.numRankings ; i++)
        {
            SpecialCommandGetMiningScoreRanking::ScoreEntry miner = response.rankings[i];
            char publicIdentity[128] = {0};
            getIdentityFromPublicKey(miner.minerPublicKey, publicIdentity, false);
            LOG("%-8u%-64s%u\n", i + 1, publicIdentity, miner.minerScore);
            total_score += miner.minerScore;
        }
        LOG("WARNING: This is raw miner score data from the node and is NOT the final ranking list.\n");
        LOG("Check processTick function in core node to have the latest ranking logic\n");
    }
    else
    {
        LOG("No updated yet.\n");
    }
    LOG("Total score: %llu\n", total_score);
}

unsigned int extract10Bit(const unsigned char* data, unsigned int idx)
{
    //TODO: simplify this
    unsigned int byte0 = data[idx + (idx >> 2)];
    unsigned int byte1 = data[idx + (idx >> 2) + 1];
    unsigned int last_bit0 = 8 - (idx & 3) * 2;
    unsigned int first_bit1 = 10 - last_bit0;
    unsigned int res = (byte0 & ((1 << last_bit0) - 1)) << first_bit1;
    res |= (byte1 >> (8 - first_bit1));
    return res;
}

void getVoteCounterTransaction(const char* nodeIp, const int nodePort, unsigned int requestedTick, const char* compFileName)
{
    BroadcastComputors bc;
    {
        FILE* f = fopen(compFileName, "rb");
        if (fread(&bc, 1, sizeof(BroadcastComputors), f) != sizeof(BroadcastComputors))
        {
            LOG("Failed to read comp list\n");
            fclose(f);
            return;
        }
        fclose(f);
    }
    auto qc = make_qc(nodeIp, nodePort);
    std::vector<Transaction> txs;
    std::vector<TxhashStruct> txHashesFromTick;
    std::vector<extraDataStruct> extraData;
    std::vector<SignatureStruct> signatureStruct;
    getTickTransactions(qc.get(), requestedTick, 1024, txs, &txHashesFromTick, &extraData, &signatureStruct);
    unsigned int votes[676];
    int nTx = txs.size();
    LOG("Finding in %d transactions", nTx);
    for (int i = 0; i < nTx; i++)
    {
        if (extraData[i].vecU8.size() == 848)
        {
            int comp_idx = requestedTick % 676;
            if (memcmp(txs[i].sourcePublicKey, bc.computors.publicKeys[comp_idx], 32) == 0)
            {
                uint8_t* data = extraData[i].vecU8.data();
                uint32_t sum = 0;
                for (int j = 0; j < 676; j++)
                {
                    votes[j] = extract10Bit(data, j);
                    sum += votes[j];
                    auto alphabet = indexToAlphabet(j);
                    LOG("%s: %u\n", alphabet.c_str(), votes[j]);
                }
                if (sum < 676*451)
                {
                    LOG("Invalid sum votes: %u\n", sum);
                }
                if (votes[comp_idx] != 0)
                {
                    LOG("Invalid comp votes\n");
                }
            }
        }
    }
}
