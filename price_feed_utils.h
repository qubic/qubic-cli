#pragma once

#include <cstdint>

// fromHour for "the latest hours the node holds".
static constexpr uint32_t BTC_PRICE_SERIES_LATEST = 0xFFFFFFFFu;

// -getbtcpriceseries [FROM_HOUR] [COUNT] [PERIOD_SEC] : print the state of the node's BTC price series
// and 'count' points from 'fromHour' as CSV. periodSec is the length of one step, used for the UTC column.
void printBtcPriceSeries(const char* nodeIp, int nodePort, uint32_t fromHour, uint32_t count, uint32_t periodSec);

// -getbpp9000taskdata [ANCHOR_TICK] : print the mining task data the node uses for a solution anchored at
// 'anchorTick' (0 = the latest completed tick).
void printBpp9000TaskData(const char* nodeIp, int nodePort, uint32_t anchorTick);
