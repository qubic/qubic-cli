#include "qubicLogParser.h"
#include <string>
#include <cstring>
#include "keyUtils.h"
#include "logger.h"

#define QU_TRANSFER 0
#define ASSET_ISSUANCE 1
#define ASSET_OWNERSHIP_CHANGE 2
#define ASSET_POSSESSION_CHANGE 3
#define CONTRACT_ERROR_MESSAGE 4
#define CONTRACT_WARNING_MESSAGE 5
#define CONTRACT_INFORMATION_MESSAGE 6
#define CONTRACT_DEBUG_MESSAGE 7
#define CUSTOM_MESSAGE 255
std::string logTypeToString(uint8_t type){
    switch(type){
        case 0:
            return "QU transfer";
        case 1:
            return "Asset issuance";
        case 2:
            return "Asset ownership change";
        case 3:
            return "Asset possession change";
        case 4:
            return "Contract error";
        case 5:
            return "Contract warning";
        case 6:
            return "Contract info";
        case 7:
            return "Contract debug";
        case 255:
            return "Custom msg";
    }
    return "Unknown msg";
}
std::string parseLogToString_type0(uint8_t* ptr){
    char sourceIdentity[61] = {0};
    char destIdentity[61] = {0};;
    uint64_t amount;
    const bool isLowerCase = false;
    getIdentityFromPublicKey(ptr, sourceIdentity, isLowerCase);
    getIdentityFromPublicKey(ptr+32, destIdentity, isLowerCase);
    memcpy(&amount, ptr+64, 8);
    std::string result = "from " + std::string(sourceIdentity) + " to " + std::string(destIdentity) + " " + std::to_string(amount) + "QU.";
    return result;
}
void printQubicLog(uint8_t* logBuffer, int bufferSize){
    if (bufferSize < 16){
        LOG("Buffer size is too small (not enough to contain the header)\n");
        return;
    }
    // basic info
    uint8_t year = *((unsigned char*)(logBuffer + 0));
    uint8_t month = *((unsigned char*)(logBuffer + 1));
    uint8_t day = *((unsigned char*)(logBuffer + 2));
    uint8_t hour = *((unsigned char*)(logBuffer + 3));
    uint8_t minute = *((unsigned char*)(logBuffer + 4));
    uint8_t second = *((unsigned char*)(logBuffer + 5));
    uint16_t epoch = *((unsigned char*)(logBuffer + 6));
    uint32_t tick = *((unsigned int*)(logBuffer + 8));
    uint32_t tmp = *((unsigned int*)(logBuffer + 12));
    uint8_t messageType = tmp >> 24;
    std::string mt = logTypeToString(messageType);
    uint32_t messageSize = (tmp << 8) >> 8;

    logBuffer += 16;
    std::string humanLog = "[Can't parse]";
    switch(messageType){
        case 0:
            if (messageSize == 72){ //TODO: change this to constant
                humanLog = parseLogToString_type0(logBuffer);
            } else {
                LOG("Malfunction buffer size for QU_TRANSFER log\n");
            }
            break;
            //TODO: fill these functions
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 255:
            break;
    }

    // tmp
    LOG("%02d-%02d-%02d %02d:%02d:%02d %u.%03d %s: %s\n", year, month, day, hour, minute, second, tick, epoch, mt.c_str(), humanLog.c_str());
}