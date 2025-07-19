#include "testUtils.h"
#include "nodeUtils.h"
#include "connection.h"
#include "logger.h"
#include "keyUtils.h"
#include "K12AndKeyUtil.h"
#include "walletUtils.h"

#include <vector>
#include <array>

// Change this index when new contracts are added
#define TESTEXA_CONTRACT_INDEX 13
#define TESTEXA_ADDRESS "NAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMAML"
#define TESTEXB_CONTRACT_INDEX 14
#define TESTEXC_CONTRACT_INDEX 15

// TESTEXA FUNCTIONS
constexpr uint8_t TESTEXA_QUERY_QPI_FUNCTIONS = 1;
constexpr uint8_t TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_BEGIN_TICK = 2;
constexpr uint8_t TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_END_TICK = 3;
constexpr uint8_t TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_USER_PROC = 4;

// TESTEXA PROCEDURES
constexpr uint8_t TESTEXA_ISSUE_ASSET = 1;
constexpr uint8_t TESTEXA_TRANSFER_SHARE_OWNERSHIP_AND_POSSESSION = 2;
constexpr uint8_t TESTEXA_TRANSFER_SHARE_MANAGEMENT_RIGHTS = 3;
constexpr uint8_t TESTEXA_SET_PRE_RELEASE_SHARES_OUTPUT = 4;
constexpr uint8_t TESTEXA_SET_PRE_ACQUIRE_SHARES_OUTPUT = 5;
constexpr uint8_t TESTEXA_ACQUIRE_SHARE_MANAGEMENT_RIGHTS = 6;
constexpr uint8_t TESTEXA_QUERY_QPI_FUNCTIONS_TO_STATE = 7;

// TESTEXB/C FUNCTIONS
constexpr uint8_t TESTEXBC_INCOMING_TRANSFER_AMOUNTS = 20;

// TESTEXB/C PROCEDURES
constexpr uint8_t TESTEXBC_QPI_BID_IN_IPO = 30;


template <typename T>
bool checkMatchAndLog(const T& first, const T& second, const std::string& name)
{
    if (first == second)
        return true;
    else
    {
        LOG("\t\t- %s: differs! (%u vs. %u)\n", name.c_str(), first, second);
        return false;
    }
}

bool checkMatchAndLog(const uint8_t* first, const uint8_t* second, uint32_t size, const std::string& name)
{
    bool matches = true;
    uint32_t i;
    for (i = 0; i < size; ++i)
        if (first[i] != second[i])
        {
            matches = false;
            break;
        }
    if (!matches)
        LOG("\t\t- %s: differs in byte %d! (%u vs. %u)\n", name.c_str(), i, first[i], second[i]);

    return matches;
}

bool qpiFunctionsOutputMatches(const QpiFunctionsOutput& first, const QpiFunctionsOutput& second, bool includeInvocatorAndOriginator = true)
{
    bool matches = true;
    matches &= checkMatchAndLog(first.arbitrator, second.arbitrator, 32, "arbitrator");
    matches &= checkMatchAndLog(first.computor0, second.computor0, 32, "computor0");
    if (includeInvocatorAndOriginator)
    {
        matches &= checkMatchAndLog(first.invocator, second.invocator, 32, "invocator");
        matches &= checkMatchAndLog(first.originator, second.originator, 32, "originator");
    }
    matches &= checkMatchAndLog(first.invocationReward, second.invocationReward, "invocationReward");
    matches &= checkMatchAndLog(first.numberOfTickTransactions, second.numberOfTickTransactions, "numberOfTickTransactions");
    matches &= checkMatchAndLog(first.tick, second.tick, "tick");
    matches &= checkMatchAndLog(first.epoch, second.epoch, "epoch");
    matches &= checkMatchAndLog(first.millisecond, second.millisecond, "millisecond");
    matches &= checkMatchAndLog(first.year, second.year, "year");
    matches &= checkMatchAndLog(first.month, second.month, "month");
    matches &= checkMatchAndLog(first.day, second.day, "day");
    matches &= checkMatchAndLog(first.hour, second.hour, "hour");
    matches &= checkMatchAndLog(first.minute, second.minute, "minute");
    matches &= checkMatchAndLog(first.second, second.second, "second");
    matches &= checkMatchAndLog(first.dayOfWeek, second.dayOfWeek, "dayOfWeek");
    return matches;
}

bool qpiFunctionsOutputMatchesTickData(const QpiFunctionsOutput& qpiOutput, const TickData& tickData, const TickData& prevTickData)
{
    bool matches = true;
    matches &= checkMatchAndLog(qpiOutput.tick, tickData.tick, "tick");
    matches &= checkMatchAndLog(qpiOutput.epoch, tickData.epoch, "epoch");
    matches &= checkMatchAndLog(qpiOutput.millisecond, prevTickData.millisecond, "millisecond");
    matches &= checkMatchAndLog(qpiOutput.year, prevTickData.year, "year");
    matches &= checkMatchAndLog(qpiOutput.month, prevTickData.month, "month");
    matches &= checkMatchAndLog(qpiOutput.day, prevTickData.day, "day");
    matches &= checkMatchAndLog(qpiOutput.hour, prevTickData.hour, "hour");
    matches &= checkMatchAndLog(qpiOutput.minute, prevTickData.minute, "minute");
    matches &= checkMatchAndLog(qpiOutput.second, prevTickData.second, "second");

    if (tickData.epoch == 0) // empty tick
    {
        matches &= (qpiOutput.numberOfTickTransactions == -1);
    }
    else
    {
        int numTxsTickData = 0;
        for (int tx = 0; tx < NUMBER_OF_TRANSACTIONS_PER_TICK; ++tx)
        {
            for (int b = 0; b < 32; ++b)
            {
                if (tickData.transactionDigests[tx][b] != 0)
                {
                    numTxsTickData++;
                    break;
                }
            }
        }
        matches &= (numTxsTickData == qpiOutput.numberOfTickTransactions);
    }

    return matches;
}

bool qpiFunctionsOutputMatchesTick(const QpiFunctionsOutput& qpiOutput, const Tick& tickData)
{
    bool matches = true;
    matches &= checkMatchAndLog(qpiOutput.tick, tickData.tick, "tick");
    matches &= checkMatchAndLog(qpiOutput.epoch, tickData.epoch, "epoch");
    matches &= checkMatchAndLog(qpiOutput.millisecond, tickData.millisecond, "millisecond");
    matches &= checkMatchAndLog(qpiOutput.year, tickData.year, "year");
    matches &= checkMatchAndLog(qpiOutput.month, tickData.month, "month");
    matches &= checkMatchAndLog(qpiOutput.day, tickData.day, "day");
    matches &= checkMatchAndLog(qpiOutput.hour, tickData.hour, "hour");
    matches &= checkMatchAndLog(qpiOutput.minute, tickData.minute, "minute");
    matches &= checkMatchAndLog(qpiOutput.second, tickData.second, "second");
    return matches;
}

std::vector<std::array<char, 128>> queryQpiFunctionsOutputToState(QCPtr qc, const char* seed, uint32_t firstScheduledTick, uint32_t numTicks)
{
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getPublicKeyFromIdentity(TESTEXA_ADDRESS, destPublicKey);

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 0;
    packet.transaction.inputType = TESTEXA_QUERY_QPI_FUNCTIONS_TO_STATE;
    packet.transaction.inputSize = 0;
    // set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    std::vector<std::array<char, 128>> txHashes(numTicks);
    for (uint32_t tickOffset = 0; tickOffset < numTicks; ++tickOffset)
    {
        packet.transaction.tick = firstScheduledTick + tickOffset;
        // sign the packet
        KangarooTwelve((unsigned char*)&packet.transaction,
            sizeof(Transaction),
            digest,
            32);
        sign(subSeed, sourcePublicKey, digest, signature);
        memcpy(packet.sig, signature, SIGNATURE_SIZE);

        qc->sendData((uint8_t*)&packet, packet.header.size());

        KangarooTwelve((unsigned char*)&packet.transaction,
            sizeof(Transaction) + SIGNATURE_SIZE,
            digest,
            32); // recompute digest for txhash
        getTxHashFromDigest(digest, txHashes[tickOffset].data());
    }
    return txHashes;
}

QpiFunctionsOutput getQpiFunctionsOutput(QCPtr qc, uint32_t requestedTick, unsigned short inputType)
{
    QpiFunctionsOutput output;
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        uint32_t tick;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(uint32_t);
    packet.rcf.contractIndex = TESTEXA_CONTRACT_INDEX;
    packet.tick = requestedTick;
    packet.rcf.inputType = inputType;

    qc->sendData((uint8_t*)&packet, packet.header.size());
    try
    {
        output = qc->receivePacketWithHeaderAs<QpiFunctionsOutput>();
    }
    catch (std::logic_error& e)
    {
        LOG("%s\n", e.what());
        memset(&output, 0, sizeof(output));
    }
    return output;
}

static void queryAndMatchQpiFunctionsOutput(QCPtr qc, uint32_t firstQueriedTick, uint32_t lastQueriedTick, bool useUserProc)
{
    constexpr int numRetries = 3;
    int retryCtr = 0;
    std::unique_ptr<TickData> prevTickData = nullptr;
    TickData tickData;
    for (uint32_t requestedTick = firstQueriedTick; requestedTick <= lastQueriedTick; ++requestedTick)
    {
        qc->resolveConnection();
        // request output of qpi functions that were saved in the TESTEXA SC
        QpiFunctionsOutput beginTickOutput = getQpiFunctionsOutput(qc, requestedTick, TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_BEGIN_TICK);
        QpiFunctionsOutput endTickOutput = getQpiFunctionsOutput(qc, requestedTick, TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_END_TICK);
        QpiFunctionsOutput userProcOutput;
        if (useUserProc) 
            userProcOutput = getQpiFunctionsOutput(qc, requestedTick, TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_USER_PROC);

        LOG("Tick %u\n", requestedTick);
        if (beginTickOutput.tick == requestedTick)
        {
            LOG("\tComparing BEGIN_TICK and END_TICK qpi functions output\n");
            if (endTickOutput.tick == requestedTick)
            {
                if (qpiFunctionsOutputMatches(beginTickOutput, endTickOutput))
                    LOG("\t\t-> matches\n");
            }
            else
                LOG("\t\tfailed to get END_TICK qpi functions output\n");
            if (useUserProc)
            {
                LOG("\tComparing BEGIN_TICK and USER_PROC qpi functions output\n");
                if (userProcOutput.tick == requestedTick)
                {
                    if (qpiFunctionsOutputMatches(beginTickOutput, userProcOutput, false))
                        LOG("\t\t-> matches\n");
                }
                else
                    LOG("\t\tfailed to get USER_PROC qpi functions output\n");
            }
        }
        else
        {
            LOG("\tfailed to get BEGIN_TICK qpi functions output\n");
            continue;
        }

        // also get TickData for comparison 
        // ! the time stamps in TickData are offset because the TickData for tick N is generated at tick N-2 whereas the votes are generated at N-1
        // ! we need to match the time stamps from the qpi functions output to the time stamp in prevTickData
        if (!prevTickData || prevTickData->epoch == 0)
        {
            if (!prevTickData) prevTickData = std::make_unique<TickData>();
            retryCtr = numRetries;
            while (!getTickData(qc, requestedTick - 1, *prevTickData))
            {
                if (retryCtr == 0)
                    break;
                qc->resolveConnection();
                retryCtr--;
            }
        }
        retryCtr = numRetries;
        while (!getTickData(qc, requestedTick, tickData))
        {
            if (retryCtr == 0)
                break;
            qc->resolveConnection();
            retryCtr--;
        }
        LOG("\tComparing BEGIN_TICK qpi functions output and TickData\n");
        if (tickData.tick == requestedTick && prevTickData->tick == requestedTick - 1)
        {
            if (qpiFunctionsOutputMatchesTickData(beginTickOutput, tickData, *prevTickData))
                LOG("\t\t-> matches\n");
        }
        else
        {
            LOG("\t\tfailed to get TickData or tick is empty\n");
        }

        // get quorum tick votes for comparison
        qc->resolveConnection();
        static struct
        {
            RequestResponseHeader header;
            RequestedQuorumTick rqt;
        } packetQT;
        packetQT.header.setSize(sizeof(packetQT));
        packetQT.header.randomizeDejavu();
        packetQT.header.setType(RequestedQuorumTick::type);
        packetQT.rqt.tick = requestedTick;
        memset(packetQT.rqt.voteFlags, 0, (676 + 7) / 8);
        qc->sendData(reinterpret_cast<uint8_t*>(&packetQT), sizeof(packetQT));
        auto votes = qc->getLatestVectorPacketAs<Tick>();
        LOG("\tComparing BEGIN_TICK qpi functions output and quorum tick votes\n");
        LOG("\t\tReceived %d quorum tick votes for comparison\n", votes.size());
        int voteMatchCtr = 0;
        for (int v = 0; v < votes.size(); ++v)
        {
            if (qpiFunctionsOutputMatchesTick(beginTickOutput, votes[v]))
                voteMatchCtr++;
        }
        LOG("\t\tBEGIN_TICK qpi functions output matches %d/%d votes\n", voteMatchCtr, votes.size());

        // save TickData as prevTickData for next loop iteration
        *prevTickData = tickData;
    }
}

void testQpiFunctionsOutput(const char* nodeIp, const int nodePort, const char* seed, uint32_t scheduledTickOffset)
{
    // get current tick
    auto qc = make_qc(nodeIp, nodePort);
    uint32_t currentTick = getTickNumberFromNode(qc);

    // send tx to query qpi functions in user procedure for numTicks ticks starting from currentTick + scheduledTickOffset
    constexpr uint32_t numTicks = 15; // nodes currently save info from last 16 ticks but we miss one due to delay in matching
    uint32_t firstScheduledTick = currentTick + scheduledTickOffset;
    LOG("Sending txs to save qpi functions output to contract state... ");
    std::vector<std::array<char, 128>> txHashes = queryQpiFunctionsOutputToState(qc, seed, firstScheduledTick, numTicks);
    LOG("Done.\n");

    // wait until network reached last queried tick
    uint32_t lastQueriedTick = firstScheduledTick + numTicks - 1;
    LOG("Waiting for network to reach last queried tick %u, currently at %u...", lastQueriedTick, currentTick);
    while (currentTick <= lastQueriedTick)
    {
        uint32_t tick = getTickNumberFromNode(qc);
        if (tick == 0)
        {
            qc->resolveConnection();
            continue;
        }
        if (tick != currentTick)
        {
            currentTick = tick;
            if (currentTick > firstScheduledTick)
            {
                bool txIncluded = checkTxOnTick(qc, txHashes[currentTick - firstScheduledTick - 1].data(), currentTick - 1, false);
            }
            else
                LOG("\n");
            LOG("\tTick %u ", currentTick);
        }
        Q_SLEEP(1000);
    }
    LOG("\nDone.\n");

    queryAndMatchQpiFunctionsOutput(qc, firstScheduledTick, lastQueriedTick, true);
}

void testQpiFunctionsOutputPast(const char* nodeIp, const int nodePort)
{
    // get current tick
    auto qc = make_qc(nodeIp, nodePort);
    uint32_t currentTick = getTickNumberFromNode(qc);

    queryAndMatchQpiFunctionsOutput(qc, currentTick - 15, currentTick - 1, false);
}

static unsigned int getTestContractIndexBC(const char* contractStr)
{
    if (strcasecmp(contractStr, "B") == 0)
    {
        return TESTEXB_CONTRACT_INDEX;
    }
    else if (strcasecmp(contractStr, "C") == 0)
    {
        return TESTEXC_CONTRACT_INDEX;
    }
    else
    {
        LOG("Unsupported test contract %s! Either pass B or C!\n", contractStr);
        exit(1);
    }
}

void testGetIncomingTransferAmounts(
    const char* nodeIp, const int nodePort,
    const char* contractToQuery)
{
    unsigned int contractIndex = getTestContractIndexBC(contractToQuery);

    struct IncomingTransferAmounts_output
    {
        int64_t standardTransactionAmount;
        int64_t procedureTransactionAmount;
        int64_t qpiTransferAmount;
        int64_t qpiDistributeDividendsAmount;
        int64_t revenueDonationAmount;
        int64_t ipoBidRefundAmount;
    };

    IncomingTransferAmounts_output output;
    if (!runContractFunction(nodeIp, nodePort, contractIndex, TESTEXBC_INCOMING_TRANSFER_AMOUNTS, nullptr, 0, &output, sizeof(output)))
    {
        LOG("Failed to get incoming transfer amounts!");
        exit(1);
    }

    LOG("incoming standardTransactionAmount:  %lld\n", output.standardTransactionAmount);
    LOG("incoming procedureTransactionAmount: %lld\n", output.procedureTransactionAmount);
    LOG("incoming qpiTransferAmount: %lld\n", output.qpiTransferAmount);
    LOG("incoming qpiDistributeDividendsAmount: %lld\n", output.qpiDistributeDividendsAmount);
    LOG("incoming revenueDonationAmount: %lld\n", output.revenueDonationAmount);
    LOG("incoming ipoBidRefundAmount: %lld\n", output.ipoBidRefundAmount);
}

void testBidInIpoThroughContract(
    const char* nodeIp, const int nodePort,
    const char* seed,
    const char* contractToBidThrough,
    uint32_t contractIndexToBidFor,
    uint64_t pricePerShare,
    uint16_t numberOfShares,
    uint32_t scheduledTickOffset)
{
    unsigned int contractIndexToBidThrough = getTestContractIndexBC(contractToBidThrough);

    struct QpiBidInIpo_input
    {
        uint32_t ipoContractIndex;
        int64_t pricePerShare;
        uint16_t numberOfShares;
    } input;
    memset(&input, 0, sizeof(input));
    input.ipoContractIndex = contractIndexToBidFor;
    input.pricePerShare = (int64_t)pricePerShare;
    input.numberOfShares = numberOfShares;

    makeContractTransaction(nodeIp, nodePort, seed, contractIndexToBidThrough,
        TESTEXBC_QPI_BID_IN_IPO, 0, sizeof(input), &input, scheduledTickOffset);
}
