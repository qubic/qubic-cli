#include "testUtils.h"
#include "nodeUtils.h"
#include "connection.h"
#include "logger.h"

// Change this index when new contracts are added
#define TESTEXA_CONTRACT_INDEX 12
#define TESTEXA_ADDRESS "MAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAWLWD"

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

template <typename T>
bool checkMatchAndLog(const T& first, const T& second, const std::string& name)
{
    if (first == second)
        return true;
    else
    {
        LOG("\t- %s: differs! (%u vs. %u)\n", name, first, second);
        return false;
    }
}

bool checkMatchAndLog(const uint8_t* first, const uint8_t* second, uint32_t size, const std::string& name)
{
    bool matches = true;
    int i;
    for (i = 0; i < size; ++i)
        if (first[i] != second[i])
        {
            matches = false;
            break;
        }
    if (!matches)
        LOG("\t- %s: differs in byte %d! (%u vs. %u)\n", name, i, first[i], second[i]);

    return matches;
}

bool qpiFunctionsOutputMatches(const QpiFunctionsOutput& first, const QpiFunctionsOutput& second)
{
    bool matches = true;
    matches &= checkMatchAndLog(first.arbitrator, second.arbitrator, 32, "arbitrator");
    matches &= checkMatchAndLog(first.computor0, second.computor0, 32, "computor0");
    matches &= checkMatchAndLog(first.invocator, second.invocator, 32, "invocator");
    matches &= checkMatchAndLog(first.originator, second.originator, 32, "originator");
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

void testQpiFunctionsBeginAndEndTick(const char* nodeIp, const int nodePort)
{
    // get current tick
    auto qc = make_qc(nodeIp, nodePort);
    uint32_t currentTick = getTickNumberFromNode(qc);

    // request output of qpi functions that were saved at begin and end tick for the last 16 ticks
    for (int tickOffset = 15; tickOffset >= 0; --tickOffset)
    {
        QpiFunctionsOutput beginTickOutput, endTickOutput;
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
        uint32_t requestedTick = currentTick - tickOffset;
        packet.tick = requestedTick;

        packet.rcf.inputType = TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_BEGIN_TICK;
        qc->sendData((uint8_t*)&packet, packet.header.size());
        try
        {
            beginTickOutput = qc->receivePacketWithHeaderAs<QpiFunctionsOutput>();
        }
        catch (std::logic_error& e)
        {
            memset(&beginTickOutput, 0, sizeof(beginTickOutput));
        }
        packet.header.randomizeDejavu();
        packet.rcf.inputType = TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_END_TICK;
        qc->sendData((uint8_t*)&packet, packet.header.size());
        try
        {
            endTickOutput = qc->receivePacketWithHeaderAs<QpiFunctionsOutput>();
        }
        catch (std::logic_error& e)
        {
            memset(&endTickOutput, 0, sizeof(endTickOutput));
        }

        LOG("Tick %u\n", requestedTick);
        if (beginTickOutput.tick == requestedTick && endTickOutput.tick == requestedTick)
        {
            if (qpiFunctionsOutputMatches(beginTickOutput, endTickOutput))
                LOG("\tBEGIN_TICK and END_TICK output matches\n");
        }
        else
        {
            LOG("\tfailed to get qpi functions output\n");
        }
    }
}