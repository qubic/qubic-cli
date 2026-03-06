#pragma once

void processGetOracleQuery(const char* nodeIp, const int nodePort, const char* requestType, const char* reqParam);
void makeOracleUserQueryTransaction(
    const char* nodeIp, int nodePort, const char* seed,
    const char* oracleInterfaceString,
    const char* queryString,
    const char* timeoutSecondsString,
    uint32_t scheduledTickOffset);
void makePriceOracleContractTransaction(
    const char* nodeIp, int nodePort, const char* seed,
    const char* commandString,
    uint32_t contractIndex,
    const char* queryStringOrSubscriptionId,
    const char* timeSecondsString,
    uint32_t scheduledTickOffset);