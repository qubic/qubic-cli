#pragma once

#include "structs.h"

struct QpiFunctionsOutput
{
	uint8_t year;   // [0..99] (0 = 2000, 1 = 2001, ..., 99 = 2099)
	uint8_t month;  // [1..12]
	uint8_t day;    // [1..31]
	uint8_t hour;   // [0..23]
	uint8_t minute; // [0..59]
	uint8_t second;
	uint16_t millisecond;
	uint8_t dayOfWeek;
	uint8_t arbitrator[32];
	uint8_t computor0[32];
	uint16_t epoch;
	int64_t invocationReward;
	uint8_t invocator[32];
	int32_t numberOfTickTransactions;
	uint8_t originator[32];
	uint32_t tick;

	static constexpr unsigned char type()
	{
		return RespondContractFunction::type();
	}
};

void testQpiFunctionsBeginAndEndTick(const char* nodeIp, const int nodePort);