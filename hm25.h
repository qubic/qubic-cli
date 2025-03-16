#pragma once

#include "structs.h"
#include <cstdint>

#define HM25_CONTRACT_INDEX 12
void hm25Echo(const char* nodeIp, int nodePort, char* seed, uint64_t amount, uint32_t scheduledTickOffset);
void hm25Burn(const char* nodeIp, int nodePort, char* seed, uint64_t amount, uint32_t scheduledTickOffset);
void hm25GetStats(const char* nodeIp, int nodePort);