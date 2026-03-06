#include "contracts.h"
#include "utils.h"
#include "logger.h"
#include "defines.h"

extern int g_nodePort;

uint32_t getContractIndex(const char* str, bool enableTestContracts)
{
    uint32_t idx = 0;
    if (strcasecmp(str, "QX") == 0)
        idx = 1;
    else if (strcasecmp(str, "QUOTTERY") == 0 || strcasecmp(str, "QTRY") == 0)
        idx = 2;
    else if (strcasecmp(str, "RANDOM") == 0)
        idx = 3;
    else if (strcasecmp(str, "QUTIL") == 0)
        idx = 4;
    else if (strcasecmp(str, "MLM") == 0)
        idx = 5;
    else if (strcasecmp(str, "GQMPROP") == 0)
        idx = 6;
    else if (strcasecmp(str, "SWATCH") == 0)
        idx = 7;
    else if (strcasecmp(str, "CCF") == 0)
        idx = 8;
    else if (strcasecmp(str, "QEARN") == 0)
        idx = 9;
    else if (strcasecmp(str, "QVAULT") == 0)
        idx = 10;
    else if (strcasecmp(str, "MSVAULT") == 0)
        idx = 11;
    else if (strcasecmp(str, "QBAY") == 0)
        idx = 12;
    else if (strcasecmp(str, "QSWAP") == 0)
        idx = 13;
    else if (strcasecmp(str, "NOST") == 0)
        idx = 14;
    else if (strcasecmp(str, "QDRAW") == 0)
        idx = 15;
    else if (strcasecmp(str, "RL") == 0)
        idx = 16;
    else if (strcasecmp(str, "QBOND") == 0)
        idx = 17;
    else if (strcasecmp(str, "QIP") == 0)
        idx = 18;
    else if (strcasecmp(str, "QRAFFLE") == 0)
        idx = 19;
    else if (strcasecmp(str, "QRWA") == 0)
        idx = 20;
    else if (strcasecmp(str, "QRP") == 0)
        idx = 21;
    else if (strcasecmp(str, "QTF") == 0)
        idx = 22;
    else if (strcasecmp(str, "QDUEL") == 0)
        idx = 23;
    else if (strcasecmp(str, "PULSE") == 0)
        idx = 24;
    else
    {
        unsigned int contractCount = CONTRACT_COUNT;
        if (enableTestContracts)
        {
            if (strcasecmp(str, "TESTEXA") == 0)
                return TESTEXA_CONTRACT_INDEX;
            else if (strcasecmp(str, "TESTEXB") == 0)
                return TESTEXB_CONTRACT_INDEX;
            else if (strcasecmp(str, "TESTEXC") == 0)
                return TESTEXC_CONTRACT_INDEX;
            else if (strcasecmp(str, "TESTEXD") == 0)
                return TESTEXD_CONTRACT_INDEX;

            contractCount += 4; // + 4 to make contracts TestExampleA-D accessible via contract index number
        }
        if (sscanf(str, "%u", &idx) != 1 || idx == 0 || (g_nodePort == DEFAULT_NODE_PORT && (idx > contractCount)))
        {
            LOG("Contract \"%s\" is unknown!\n", str);
            exit(1);
        }
    }
    return idx;
}

const char* getContractName(uint32_t contractIndex, bool enableTestContracts)
{
    switch (contractIndex)
    {
    case 1: return "QX";
    case 2: return "QUOTTERY";
    case 3: return "RANDOM";
    case 4: return "QUTIL";
    case 5: return "MLM";
    case 6: return "GQMPROP";
    case 7: return "SWATCH";
    case 8: return "CCF";
    case 9: return "QEARN";
    case 10: return "QVAULT";
    case 11: return "MSVAULT";
    case 12: return "QBAY";
    case 13: return "QSWAP";
    case 14: return "NOST";
    case 15: return "QDRAW";
    case 16: return "RL";
    case 17: return "QBOND";
    case 18: return "QIP";
    case 19: return "QRAFFLE";
    case 20: return "QRWA";
    case 21: return "QRP";
    case 22: return "QTF";
    case 23: return "QDUEL";
    case 24: return "PULSE";
    default:
        if (enableTestContracts)
        {
            if (contractIndex == TESTEXA_CONTRACT_INDEX)
                return "TESTEXA";
            if (contractIndex == TESTEXB_CONTRACT_INDEX)
                return "TESTEXB";
            if (contractIndex == TESTEXC_CONTRACT_INDEX)
                return "TESTEXC";
            if (contractIndex == TESTEXD_CONTRACT_INDEX)
                return "TESTEXD";
        }
        return nullptr;
    }
}
