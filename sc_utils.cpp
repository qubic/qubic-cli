#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>
#include <map>

#include "sc_utils.h"
#include "key_utils.h"
#include "structs.h"
#include "qx_struct.h"
#include "connection.h"
#include "logger.h"

// {data type, {size in byte, alignment in byte}}
std::map<std::string, std::pair<unsigned long long, unsigned long long> > supportedDataTypes = {
    {"sint8", {1, 1}},
    {"uint8", {1, 1}},
    {"sint16", {2, 2}},
    {"uint16", {2, 2}},
    {"sint32", {4, 4}},
    {"uint32", {4, 4}},
    {"sint64", {8, 8}},
    {"uint64", {8, 8}},
    {"id", {32, 8}},
};

void dumpQxContractToCSV(const char *input, const char *output) {
    std::cout << "Dumping QX contract file " << input << std::endl;

    // Check the file size
    FILE* f;
    {
        f = fopen(input, "rb"); // Open the file in binary mode
        if (f == nullptr)
        {
            std::cout << "Can not open file! Exit. " << output << std::endl;
            return;
        }

        fseek(f, 0, SEEK_END);
        unsigned long fileSize = (unsigned long)ftell(f);
        fclose(f);

        if (fileSize != sizeof(QX))
        {
            std::cout << "File size is different from QX state size! " << fileSize << " . Expected " << sizeof(QX) << std::endl;
            return;
        }
    }

    std::shared_ptr<QX> qxState = std::make_shared<QX>();

    f = fopen(input, "rb");
    if (fread(qxState.get(), 1, sizeof(QX), f) != sizeof(QX))
    {
        LOG("Failed to read QX state\n");
        fclose(f);
        return;
    }
    fclose(f);

    f = fopen(output, "w");
    {
       std::string header ="EarnedAmount,DistributedAmount,BurnedAmount,AssetIssuanceFee,TransferFee,TradeFee,Entity,Issuer,AssetName,NumberOfShares,Price\n";
       fwrite(header.c_str(), 1, header.size(), f);
    }
    std::string stringLine = std::to_string(qxState->_earnedAmount)
                             + "," + std::to_string(qxState->_distributedAmount)
                             + "," + std::to_string(qxState->_burnedAmount)
                             + "," + std::to_string(qxState->_assetIssuanceFee)
                             + "," + std::to_string(qxState->_transferFee)
                             + "," + std::to_string(qxState->_tradeFee);

    int64_t elementIdx = 0;
    uint8_t povID[32];
    char buffer[128];
    char assetNameBuffer[8];
    for (uint64_t elementIdx = 0; elementIdx < qxState->_entityOrders.population(); elementIdx++)
    {
        if (elementIdx > 0)
        {
            stringLine = ",,,,,";
        }
        // Write data
        // Entity
        qxState->_entityOrders.pov(elementIdx, povID);

        memset(buffer, 0, 128);
        getIdentityFromPublicKey(povID, buffer, false);
        stringLine = stringLine + "," + buffer;

        // Extract data
        auto entityOrder = qxState->_entityOrders.element(elementIdx);

        // Issuer
        memset(buffer, 0, 128);
        getIdentityFromPublicKey(entityOrder.issuer, buffer, false);
        stringLine = stringLine + "," + buffer;

        // Asset name
        memcpy(assetNameBuffer, (char*)&entityOrder.assetName, 8);
        std::string assetName = assetNameBuffer;
        stringLine = stringLine + "," + assetName;

        // Number of share
        stringLine = stringLine + "," + std::to_string(entityOrder.numberOfShares);

        // Price
        int64_t price = qxState->_entityOrders.priority(elementIdx);
        stringLine = stringLine + "," + std::to_string(price);

        stringLine += "\n";

        fwrite(stringLine.c_str(), 1, stringLine.size(), f);
    }

    fclose(f);

    std::cout << "File is written into " << output << std::endl;
}

void dumpQtryContractToCSV(const char* input, const char* output)
{
    std::cout << "Dumping Qtry contract file " << input << std::endl;
}

void dumpContractToCSV(const char* input, uint32_t contractId, const char* output)
{
    // Checking the contract type
    std::string fileName = input;
    auto extensionLocation = fileName.rfind('.');
    switch (contractId)
    {
    case SCType::SC_TYPE_QX :
        dumpQxContractToCSV(input, output);
        break;
    default:
        std::cout << "Unsupported contract id: " << contractId << std::endl;
        break;
    }
}

void standardizeScFormat(std::string &input, bool removeStruct = false) {
    trimStr(input);
    // Remove all [] characters and {} if removeStruct is true
    input.erase(std::remove_if(input.begin(), input.end(), [removeStruct](char c) {
        return (c == '[' || c == ']') || (removeStruct && (c == '{' || c == '}'));
    }), input.end());
}

ContractDataInfo getContractInputFormatInfo(const char *format) {
    if (!format) return {0, 0, {}};
    if (std::string(format).empty()) return {0, 0, {}};
    std::vector<std::string> formatSplitted = splitString(format, ",");
    std::map<int, unsigned long long> fieldOffsets; // key: field index, value: offset in bytes
    int totalSize = 0;
    int largestAlignment = 1;
    int openBracketCount = 0;
    int closeBracketCount = 0;
    int fieldIndex = 0;
    std::string currentNestedObject;
    for (auto &fmt: formatSplitted) {
        int alignment = 1;
        if (fmt.empty()) continue;
        standardizeScFormat(fmt);

        if (isIncudedInStr(fmt, "{") || isIncudedInStr(fmt, "}")) {
            int newOpenBracketCount = std::count(fmt.begin(), fmt.end(), '{');
            int newCloseBracketCount = std::count(fmt.begin(), fmt.end(), '}');
            openBracketCount += newOpenBracketCount;
            closeBracketCount += newCloseBracketCount;

            if (openBracketCount == closeBracketCount && openBracketCount > 0) {
                currentNestedObject += fmt;
                openBracketCount = 0;
                closeBracketCount = 0;
                // Process the nested object
                currentNestedObject = currentNestedObject.substr(1, currentNestedObject.size() - 2); // remove {}
                ContractDataInfo contractData = getContractInputFormatInfo(currentNestedObject.c_str());
                // Align the nested object
                if (totalSize % contractData.alignment != 0) {
                    totalSize += (contractData.alignment - (totalSize % contractData.alignment));
                }
                // Adjust field offsets for the nested object
                for (auto &offset: contractData.fieldOffsets) {
                    fieldOffsets[fieldIndex + offset.first] = totalSize + offset.second;
                }
                totalSize += contractData.size;
                alignment = contractData.alignment;
                fieldIndex += (int) contractData.fieldOffsets.size();
                currentNestedObject = "";
                goto check_alignment;
            } else {
                currentNestedObject += fmt + ",";
                continue;
            }
        } else if (!currentNestedObject.empty()) {
            currentNestedObject += fmt + ",";
            continue;
        }

        for (auto &dataType: supportedDataTypes) {
            if (isIncudedInStr(fmt, dataType.first)) {
                if (totalSize % dataType.second.second != 0) {
                    totalSize += (dataType.second.second - (totalSize % dataType.second.second));
                }

                fieldOffsets[fieldIndex] = totalSize;
                fieldIndex++;
                totalSize += dataType.second.first; // size
                alignment = dataType.second.second;
                break;
            }
        }

        check_alignment:
        if (alignment > largestAlignment) largestAlignment = alignment;
    }

    if (totalSize % largestAlignment != 0) {
        totalSize += (largestAlignment - (totalSize % largestAlignment));
    }

    return {totalSize, largestAlignment, fieldOffsets};
}

int packContractInputData(const char *format, void *output) {
    if (!output || !format) return 0;
    if (std::string(format).empty()) return 0;

    ContractDataInfo info = getContractInputFormatInfo(format);
    uint8_t *buffer = new uint8_t[MAX_INPUT_SIZE];
    memset(buffer, 0, MAX_INPUT_SIZE);

    int currentIndex = 0;
    std::string currentNestedObject;
    std::vector<std::string> formatSplitted = splitString(format, ",");
    for (auto &fmt: formatSplitted) {
        if (fmt.empty()) continue;
        standardizeScFormat(fmt, true);

        for (auto &dataType: supportedDataTypes) {
            if (isIncudedInStr(fmt, dataType.first)) {
                std::string plainData = fmt.substr(0, fmt.size() - dataType.first.size());

                unsigned long long offsetInBuffer = info.fieldOffsets[currentIndex];
                unsigned long long dataTypeSize = dataType.second.first;

                if (dataType.first == "id") {
                    uint8_t pubkey[32];
                    memset(pubkey, 0, 32);
                    getPublicKeyFromIdentity(plainData.c_str(), pubkey);
                    memcpy(buffer + offsetInBuffer, pubkey, 32);
                } else {
                    uint64_t value = std::stoull(plainData);
                    memcpy(buffer + offsetInBuffer, &value, dataTypeSize);
                }
                break;
            }
        }

        currentIndex++;
    }

    memcpy(output, buffer, info.size);
    delete[] buffer;
    return info.size;
}

void printContractFormatData(const char *format, const void *input) {
    static std::map<std::string, std::string> formatMap = {
        {"sint8", "d"},
        {"uint8", "u"},
        {"sint16", "d"},
        {"uint16", "u"},
        {"sint32", "d"},
        {"uint32", "u"},
        {"sint64", "lld"},
        {"uint64", "llu"},
        {"id", "s"},
    };

    if (!input || !format) return;
    if (std::string(format).empty()) return;

    ContractDataInfo info = getContractInputFormatInfo(format);
    std::string formatInputString = format;
    std::string formatStringTemplate = format;
    auto *buffer = new uint8_t[MAX_INPUT_SIZE];
    memset(buffer, 0, MAX_INPUT_SIZE);

    for (auto dataType: supportedDataTypes) {
        if (isIncudedInStr(formatStringTemplate, dataType.first)) {
            size_t pos = 0;
            while ((pos = formatStringTemplate.find(dataType.first, pos)) != std::string::npos) {
                formatStringTemplate.replace(pos, dataType.first.length(), "%" + formatMap[dataType.first]);
                pos += 1; // Move past the replaced character
            }
        }
    }

    size_t currentDilimiterPos = 0;
    size_t previousDilimiterPos = 0;
    int currentIndex = 0;
    for (auto &fmt: splitString(formatInputString, ",")) {
        if (fmt.empty()) continue;
        standardizeScFormat(fmt, true);

        // Check if the data type is supported
        if (supportedDataTypes.find(fmt) == supportedDataTypes.end()) {
            LOG("FATAL: Unsupported data type %s\n", fmt.c_str());
            delete[] buffer;
        }

        previousDilimiterPos = currentDilimiterPos;
        currentDilimiterPos = formatStringTemplate.find(',', previousDilimiterPos + 1);
        std::string formatStrForThisPart = formatStringTemplate.substr(previousDilimiterPos,
                                                                    currentDilimiterPos != std::string::npos ?
                                                                       currentDilimiterPos - previousDilimiterPos :
                                                                       formatStringTemplate.size() - previousDilimiterPos);
        unsigned long long offsetInBuffer = info.fieldOffsets[currentIndex];
        if (fmt == "id") {
            uint8_t identity[61];
            memset(identity, 0, 61);
            getIdentityFromPublicKey((uint8_t *)input + offsetInBuffer, (char *) identity, false);
            printf(formatStrForThisPart.c_str(), identity);
        } else {
            uint64_t data = 0;
            memcpy(&data, (uint8_t *)input + offsetInBuffer, supportedDataTypes[fmt].first);
            printf(formatStrForThisPart.c_str(), data);
        }

        currentIndex++;
    }
}
