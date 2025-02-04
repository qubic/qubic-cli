#pragma once

#include "stdint.h"

void qutilSendToManyV1(const char* nodeIp, int nodePort, const char* seed, const char* payoutListFile, uint32_t scheduledTickOffset);
void qutilBurnQubic(const char* nodeIp, int nodePort, const char* seed, long long amount, uint32_t scheduledTickOffset);