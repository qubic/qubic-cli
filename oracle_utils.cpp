//#include <cstdint>
//#include <cstring>
#include <cstdlib>
//
//#include "structs.h"
//#include "wallet_utils.h"
//#include "key_utils.h"
#include "oracle_utils.h"
//#include "connection.h"
#include "logger.h"
#include "utils.h"

//#include "node_utils.h"
//#include "k12_and_key_utils.h"
//#include "utils.h"
//#include "sanity_check.h"

void printGetOracleQueryHelpAndExit()
{
    LOG("qubic-cli [...] -getoraclequery [QUERY_ID]\n");
    LOG("    Print the oracle query, metadata, and reply if available.\n");
    LOG("qubic-cli [...] -getoraclequery pending\n");
    LOG("    Print the query IDs for all pending queries.\n");
    LOG("qubic-cli [...] -getoraclequery pending+\n");
    LOG("    Print the oracle query, metadata, and reply if available for each query ID received by pending.\n");
    exit(1);
}

void processGetOracleQuery(const char* nodeIp, const int nodePort, const char* requestType)
{
    if (strcasecmp(requestType, "") == 0)
        printGetOracleQueryHelpAndExit();
}
