#pragma once

#include "structs.h"

struct QpiFunctionsOutput
{
    uint8_t arbitrator[32];
    uint8_t computor0[32];
    uint8_t invocator[32];
    uint8_t originator[32];
    int64_t invocationReward;
    int32_t numberOfTickTransactions;
    uint32_t tick;
    uint16_t epoch;
    uint16_t millisecond;
    uint8_t year;   // [0..99] (0 = 2000, 1 = 2001, ..., 99 = 2099)
    uint8_t month;  // [1..12]
    uint8_t day;    // [1..31]
    uint8_t hour;   // [0..23]
    uint8_t minute; // [0..59]
    uint8_t second;
    uint8_t dayOfWeek;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

void testQpiFunctionsOutput(const char* nodeIp, const int nodePort, const char* seed, uint32_t scheduledTickOffset);
void testQpiFunctionsOutputPast(const char* nodeIp, const int nodePort);
void testGetIncomingTransferAmounts(
    const char* nodeIp, const int nodePort,
    const char* contractToQuery);
void testBidInIpoThroughContract(
    const char* nodeIp, const int nodePort,
    const char* seed,
    const char* contractToBidThrough,
    uint32_t contractIndexToBidFor,
    uint64_t pricePerShare,
    uint16_t numberOfShares,
    uint32_t scheduledTickOffset);
