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


void testQpiFunctionsBeginAndEndTick(const char* nodeIp, const int nodePort)
{
    // get current tick
    auto qc = make_qc(nodeIp, nodePort);
    uint32_t currentTick = getTickNumberFromNode(qc);

    // request output of qpi functions that were saved at begin and end tick for the last 16 ticks
    for (int tickOffset = 0; tickOffset < 16; ++tickOffset)
    {
        QpiFunctionsOutput beginTickOutput, endTickOutput;
        struct {
            RequestResponseHeader header;
            RequestContractFunction rcf;
            uint32_t tick;
        } packetBegin;
        packetBegin.header.setSize(sizeof(packetBegin));
        packetBegin.header.randomizeDejavu();
        packetBegin.header.setType(RequestContractFunction::type());
        packetBegin.rcf.inputSize = sizeof(uint32_t);
        packetBegin.rcf.contractIndex = TESTEXA_CONTRACT_INDEX;
        uint32_t requestedTick = currentTick - tickOffset;
        packetBegin.tick = requestedTick;

        packetBegin.rcf.inputType = TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_BEGIN_TICK;
        qc->sendData((uint8_t*)&packetBegin, packetBegin.header.size());
        try
        {
            beginTickOutput = qc->receivePacketWithHeaderAs<QpiFunctionsOutput>();
            LOG("beginTickOutput for tick %u received\n", beginTickOutput.tick);
        }
        catch (std::logic_error& e)
        {
            memset(&beginTickOutput, 0, sizeof(beginTickOutput));
        }

        struct {
            RequestResponseHeader header;
            RequestContractFunction rcf;
            uint32_t tick;
        } packetEnd;
        packetEnd.header.setSize(sizeof(packetEnd));
        packetEnd.header.randomizeDejavu();
        packetEnd.header.setType(RequestContractFunction::type());
        packetEnd.rcf.inputSize = sizeof(uint32_t);
        packetEnd.rcf.contractIndex = TESTEXA_CONTRACT_INDEX;
        packetEnd.tick = requestedTick;
        packetEnd.header.randomizeDejavu();
        packetEnd.rcf.inputType = TESTEXA_RETURN_QPI_FUNCTIONS_OUTPUT_END_TICK;
        qc->sendData((uint8_t*)&packetEnd, packetEnd.header.size());
        try
        {
            endTickOutput = qc->receivePacketWithHeaderAs<QpiFunctionsOutput>();
            LOG("endTickOutput for tick %u received\n", endTickOutput.tick);
        }
        catch (std::logic_error& e)
        {
            memset(&endTickOutput, 0, sizeof(endTickOutput));
        }

        LOG("Tick %u\n", requestedTick);
        if (beginTickOutput.tick == requestedTick && endTickOutput.tick == requestedTick)
        {
            LOG("\t- year: ");
            if (beginTickOutput.year == endTickOutput.year)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.year, endTickOutput.year);
            LOG("\t- month: ");
            if (beginTickOutput.month == endTickOutput.month)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.month, endTickOutput.month);
            LOG("\t- day: ");
            if (beginTickOutput.day == endTickOutput.day)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.day, endTickOutput.day);
            LOG("\t- hour: ");
            if (beginTickOutput.hour == endTickOutput.hour)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.hour, endTickOutput.hour);
            LOG("\t- minute: ");
            if (beginTickOutput.minute == endTickOutput.minute)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.minute, endTickOutput.minute);
            LOG("\t- second: ");
            if (beginTickOutput.second == endTickOutput.second)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.second, endTickOutput.second);
            LOG("\t- millisecond: ");
            if (beginTickOutput.millisecond == endTickOutput.millisecond)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.millisecond, endTickOutput.millisecond);
            LOG("\t- dayOfWeek: ");
            if (beginTickOutput.dayOfWeek == endTickOutput.dayOfWeek)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.dayOfWeek, endTickOutput.dayOfWeek);
            LOG("\t- arbitrator: ");
            bool matches = true;
            int i;
            for (i = 0; i < 32; ++i)
                if (beginTickOutput.arbitrator[i] != endTickOutput.arbitrator[i])
                {
                    matches = false;
                    break;
                }
            if (matches)
                LOG("matches\n");
            else
                LOG("differs in byte %d! (beginTick %u, endTick %u)\n", i, beginTickOutput.arbitrator[i], endTickOutput.arbitrator[i]);
            LOG("\t- computor0: ");
            matches = true;
            for (i = 0; i < 32; ++i)
                if (beginTickOutput.computor0[i] != endTickOutput.computor0[i])
                {
                    matches = false;
                    break;
                }
            if (matches)
                LOG("matches\n");
            else
                LOG("differs in byte %d! (beginTick %u, endTick %u)\n", i, beginTickOutput.computor0[i], endTickOutput.computor0[i]);
            LOG("\t- epoch: ");
            if (beginTickOutput.epoch == endTickOutput.epoch)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %u, endTick %u)\n", beginTickOutput.epoch, endTickOutput.epoch);
            LOG("\t- invocationReward: ");
            if (beginTickOutput.invocationReward == endTickOutput.invocationReward)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %lld, endTick %lld)\n", beginTickOutput.invocationReward, endTickOutput.invocationReward);
            LOG("\t- invocator: ");
            matches = true;
            for (i = 0; i < 32; ++i)
                if (beginTickOutput.invocator[i] != endTickOutput.invocator[i])
                {
                    matches = false;
                    break;
                }
            if (matches)
                LOG("matches\n");
            else
                LOG("differs in byte %d! (beginTick %u, endTick %u)\n", i, beginTickOutput.invocator[i], endTickOutput.invocator[i]);
            LOG("\t- numberOfTickTransactions: ");
            if (beginTickOutput.numberOfTickTransactions == endTickOutput.numberOfTickTransactions)
                LOG("matches\n");
            else
                LOG("differs! (beginTick %d, endTick %d)\n", beginTickOutput.numberOfTickTransactions, endTickOutput.numberOfTickTransactions);
            LOG("\t- originator: ");
            matches = true;
            for (i = 0; i < 32; ++i)
                if (beginTickOutput.originator[i] != endTickOutput.originator[i])
                {
                    matches = false;
                    break;
                }
            if (matches)
                LOG("matches\n");
            else
                LOG("differs in byte %d! (beginTick %u, endTick %u)\n", i, beginTickOutput.originator[i], endTickOutput.originator[i]);
        }
        else
        {
            LOG("\tfailed to get qpi functions output\n");
        }
    }
}