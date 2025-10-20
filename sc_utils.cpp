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

static ValueType parseType(const std::string& type) {
    if (type == "sint8")  return SINT8;
    if (type == "uint8")  return UINT8;
    if (type == "sint16") return SINT16;
    if (type == "uint16") return UINT16;
    if (type == "sint32") return SINT32;
    if (type == "uint32") return UINT32;
    if (type == "sint64") return SINT64;
    if (type == "uint64") return UINT64;
    return UNKNOWN;
}

std::string getIndentString(int indent = 0) {
    std::string indentStr = "";
    for (int i = 0; i < indent; i++) {
        indentStr += "    ";
    }

    return indentStr;
}

void standardizeScFormat(std::string &input, bool removeStruct = false) {
    trimStr(input);
    // Remove all [] characters and {} if removeStruct is true
    input.erase(std::remove_if(input.begin(), input.end(), [removeStruct](char c) {
        return (c == '[' || c == ']') || (removeStruct && (c == '{' || c == '}'));
    }), input.end());
}

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

unsigned long long ContractPrimitive::getSize() {
    return supportedDataTypes[this->type].first;
}

unsigned long long ContractPrimitive::getAlignment() {
    return supportedDataTypes[this->type].second;
}

void ContractPrimitive::dumpIntoBuffer(void *buffer) {
    if (this->type == "id") {
        uint8_t pubkey[32] = {};
        getPublicKeyFromIdentity(this->value.c_str(), pubkey);
        memcpy(buffer, pubkey, 32);
    } else {
        u_int64_t value = std::stoull(this->value.c_str());
        memcpy(buffer, &value, this->getSize());
    }
}

ContractPrimitive ContractPrimitive::fromBuffer(const uint8_t *buffer, std::string type) {
    if (type == "id") {
        uint8_t id[61] = {};
        getIdentityFromPublicKey(buffer, (char*)id, false);
        return {type, std::string(reinterpret_cast<char*>(id))};
    } else {
        switch (parseType(type)) {
            case SINT8: {
                int8_t v; memcpy(&v, buffer, sizeof(v));
                return {type, std::to_string(v)};
            }
            case UINT8: {
                uint8_t v; memcpy(&v, buffer, sizeof(v));
                return {type, std::to_string(v)};
            }
            case SINT16: {
                int16_t v; memcpy(&v, buffer, sizeof(v));
                return {type, std::to_string(v)};
            }
            case UINT16: {
                uint16_t v; memcpy(&v, buffer, sizeof(v));
                return {type, std::to_string(v)};
            }
            case SINT32: {
                int32_t v; memcpy(&v, buffer, sizeof(v));
                return {type, std::to_string(v)};
            }
            case UINT32: {
                uint32_t v; memcpy(&v, buffer, sizeof(v));
                return {type, std::to_string(v)};
            }
            case SINT64: {
                int64_t v; memcpy(&v, buffer, sizeof(v));
                return {type, std::to_string(v)};
            }
            case UINT64: {
                uint64_t v; memcpy(&v, buffer, sizeof(v));
                return {type, std::to_string(v)};
            }
            default:
                throw std::runtime_error("Unsupported contract type");
        }
    }
}

void ContractPrimitive::print(int ident) {
    std::string indent = "";
    for (int i = 0; i < ident; i++) {
        indent += "    ";
    }
    LOG("%s%s", indent.c_str(), this->value.c_str());
}

bool ContractPrimitive::isEmpty() {
    return this->value == "0" || this->value == "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB";
}

std::string ContractPrimitive::toString() {
    return this->value + this->type;
}

std::string ContractObject::toString() {
    std::string output;
    for (int i = 0; i < this->numberOfFields; i++) {
        if (this->primitive.find(i) != this->primitive.end()) {
            output += this->primitive[i].toString();
        } else if (this->object.find(i) != this->object.end()) {
            std::string objectStr = this->object[i].toString();
            if (this->object[i].isArray) {
                output += "[" + objectStr + "]";
            } else {
                output += "{" + objectStr + "}";
            }
        }

        if (i + 1 < this->numberOfFields) {
            output += ",";
        }
    }

    int additionalFields = std::max(0, this->arrayMaxSize - this->numberOfFields);
    if (additionalFields > 0 && this->numberOfFields > 0) {
        output += ",";
    }
    for (int i = 0; i < additionalFields; i++) {
        if (this->primitive.find(0) != this->primitive.end()) {
            output += this->primitive[0].toString();
        } else if (this->object.find(0) != this->object.end()) {
            std::string objectStr = this->object[0].toString();
            if (this->object[0].isArray) {
                output += "[" + objectStr + "]";
            } else {
                output += "{" + objectStr + "}";
            }
        }
        if (i + 1 < additionalFields) {
            output += ",";
        }
    }

    return output;
}

unsigned long long ContractObject::getSize() {
    unsigned long long totalSize = 0;
    int additionalFields = std::max(0, this->arrayMaxSize - this->numberOfFields);

    for (int i = 0; i < this->numberOfFields; i++) {
        unsigned long long alignment = 1;
        unsigned long long size = 0;
        if (this->primitive.find(i) != this->primitive.end()) {
            alignment = this->primitive[i].getAlignment();
            size = this->primitive[i].getSize();
        } else if (this->object.find(i) != this->object.end()) {
            alignment = this->object[i].getAlignment();
            size = this->object[i].getSize();
        }
        // Align the current size
        if (totalSize % alignment != 0) {
            totalSize += (alignment - (totalSize % alignment));
        }
        totalSize += size;
    }

    // Add padding for array
    for (int i = 0; i < additionalFields; i++) {
        unsigned long long alignment = 1;
        unsigned long long size = 0;
        if (this->primitive.find(0) != this->primitive.end()) {
            alignment = this->primitive[0].getAlignment();
            size = this->primitive[0].getSize();
        } else if (this->object.find(0) != this->object.end()) {
            alignment = this->object[0].getAlignment();
            size = this->object[0].getSize();
        }
        // Align the current size
        if (totalSize % alignment != 0) {
            totalSize += (alignment - (totalSize % alignment));
        }
        totalSize += size;
    }

    return totalSize;
}

unsigned long long ContractObject::getAlignment() {
    unsigned long long largestAlignment = 1;
    for (int i = 0; i < this->numberOfFields; i++) {
        unsigned long long alignment = 1;
        if (this->primitive.find(i) != this->primitive.end()) {
            alignment = this->primitive[i].getAlignment();
        } else if (this->object.find(i) != this->object.end()) {
            alignment = this->object[i].getAlignment();
        }
        if (alignment > largestAlignment) {
            largestAlignment = alignment;
        }
    }
    return largestAlignment;
}

void ContractObject::dumpIntoBuffer(void *buffer) {
    uint8_t *internalBuffer = new uint8_t[MAX_INPUT_SIZE];
    unsigned long long currentBufferPos = 0;
    for (int i = 0; i < this->numberOfFields; i++) {
        unsigned long long alignment = 1;
        unsigned long long size = 0;
        if (this->primitive.find(i) != this->primitive.end()) {
            alignment = this->primitive[i].getAlignment();
            size = this->primitive[i].getSize();
        } else if (this->object.find(i) != this->object.end()) {
            alignment = this->object[i].getAlignment();
            size = this->object[i].getSize();
        }
        // Align the current buffer pointer
        if (currentBufferPos % alignment != 0) {
            currentBufferPos += (alignment - (currentBufferPos % alignment));
        }
        if (this->primitive.find(i) != this->primitive.end()) {
            this->primitive[i].dumpIntoBuffer(internalBuffer + currentBufferPos);
        } else if (this->object.find(i) != this->object.end()) {
            this->object[i].dumpIntoBuffer(internalBuffer + currentBufferPos);
        }

        currentBufferPos += size;
    }

    // Add padding for array
    int additionalFields = std::max(0, this->arrayMaxSize - this->numberOfFields);
    for (int i = 0; i < additionalFields; i++) {
        unsigned long long alignment = 1;
        unsigned long long size = 0;
        if (this->primitive.find(0) != this->primitive.end()) {
            alignment = this->primitive[0].getAlignment();
            size = this->primitive[0].getSize();
        } else if (this->object.find(0) != this->object.end()) {
            alignment = this->object[0].getAlignment();
            size = this->object[0].getSize();
        }
        // Align the current buffer pointer
        if (currentBufferPos % alignment != 0) {
            currentBufferPos += (alignment - (currentBufferPos % alignment));
        }
        // Add zero padding
        memset(internalBuffer + currentBufferPos, 0, size);
        currentBufferPos += size;
    }

    if (currentBufferPos != this->getSize()) {
        LOG("FATAL: Something went wrong when dump contract object into buffer copied size: %d expected size: %d \n", currentBufferPos, this->getSize());
    }

    memcpy(buffer, internalBuffer, currentBufferPos);
}

ContractObject ContractObject::fromBuffer(void *buffer, const char *format, bool isRoot) {
    ContractObject contractObject = buildContractObject(format, isRoot);
    unsigned long long currentBufferPos = 0;
    for (int i = 0; i < contractObject.numberOfFields; i++) {
        unsigned long long alignment = 1;
        unsigned long long size = 0;
        if (contractObject.primitive.find(i) != contractObject.primitive.end()) {
            alignment = contractObject.primitive[i].getAlignment();
            size = contractObject.primitive[i].getSize();
        } else if (contractObject.object.find(i) != contractObject.object.end()) {
            alignment = contractObject.object[i].getAlignment();
            size = contractObject.object[i].getSize();
        }
        // Align the current buffer pointer
        if (currentBufferPos % alignment != 0) {
            currentBufferPos += (alignment - (currentBufferPos % alignment));
        }
        if (contractObject.primitive.find(i) != contractObject.primitive.end()) {
            contractObject.primitive[i] = ContractPrimitive::fromBuffer((uint8_t*)buffer + currentBufferPos, contractObject.primitive[i].type);
        } else if (contractObject.object.find(i) != contractObject.object.end()) {
            bool isArray = contractObject.object[i].isArray;
            int arrayMaxSize = contractObject.object[i].arrayMaxSize;
            contractObject.object[i] = ContractObject::fromBuffer((uint8_t*)buffer + currentBufferPos, contractObject.object[i].toString().c_str());
            contractObject.object[i].isArray = isArray;
            contractObject.object[i].arrayMaxSize = arrayMaxSize;
        }

        currentBufferPos += size;
    }

    // Handle max array size case
    int additionalFields = std::max(0, contractObject.arrayMaxSize - contractObject.numberOfFields);
    for (int i = 0; i < additionalFields; i++) {
        unsigned long long alignment = 1;
        unsigned long long size = 0;
        if (contractObject.primitive.find(0) != contractObject.primitive.end()) {
            alignment = contractObject.primitive[0].getAlignment();
            size = contractObject.primitive[0].getSize();
        } else if (contractObject.object.find(0) != contractObject.object.end()) {
            alignment = contractObject.object[0].getAlignment();
            size = contractObject.object[0].getSize();
        }
        // Align the current buffer pointer
        if (currentBufferPos % alignment != 0) {
            currentBufferPos += (alignment - (currentBufferPos % alignment));
        }
        if (contractObject.primitive.find(0) != contractObject.primitive.end()) {
            contractObject.primitive[i + contractObject.numberOfFields] = ContractPrimitive::fromBuffer((uint8_t*)buffer + currentBufferPos, contractObject.primitive[0].type);
        } else if (contractObject.object.find(0) != contractObject.object.end()) {
            bool isArray = contractObject.object[0].isArray;
            int arrayMaxSize = contractObject.object[0].arrayMaxSize;
            contractObject.object[i + contractObject.numberOfFields] = ContractObject::fromBuffer((uint8_t*)buffer + currentBufferPos, contractObject.object[0].toString().c_str());
            contractObject.object[i + contractObject.numberOfFields].isArray = isArray;
            contractObject.object[i + contractObject.numberOfFields].arrayMaxSize = arrayMaxSize;
        }

        currentBufferPos += size;
    }

    return contractObject;
}

void ContractObject::print(int indent, bool hasNext) {
    if (this->isEmpty()) {
        LOG("%s(empty)%s\n", getIndentString(indent - 1).c_str(), hasNext ? "," : "");
        return;
    }

    if (this->isArray) {
        LOG("%s[\n", getIndentString(indent - 1).c_str());
    } else {
        LOG("%s{\n", getIndentString(indent - 1).c_str());
    }

    int totalFields = this->numberOfFields + std::max(0, this->arrayMaxSize - this->numberOfFields);
    for (int i = 0; i < totalFields; i++) {
        if (this->primitive.find(i) != this->primitive.end()) {
            this->primitive[i].print(indent);
            if (i+1 < numberOfFields) {
                LOG(",\n");
            } else {
                LOG("\n");
            }
        } else if (this->object.find(i) != this->object.end()) {
            bool isHasNext = (i + 1 < totalFields);
            bool isArray = this->object[i].isArray;
            if (isArray) {
                this->object[i].print(indent + 1, isHasNext);
            } else {
                this->object[i].print(indent + 1, isHasNext);
            }
        }
    }

    if (this->isArray) {
        LOG("%s]%s\n", getIndentString(indent - 1).c_str(), hasNext ? "," : "");
    } else {
        LOG("%s}%s\n", getIndentString(indent - 1).c_str(), hasNext ? "," : "");
    }
}

bool ContractObject::isEmpty() {
    for (int i = 0; i < this->numberOfFields; i++) {
        if (this->primitive.find(i) != this->primitive.end()) {
            if (!this->primitive[i].isEmpty()) {
                return false;
            }
        } else if (this->object.find(i) != this->object.end()) {
            if (!this->object[i].isEmpty()) {
                return false;
            }
        }
    }

    return true;
}

ContractObject buildContractObject(const char *format, bool isRoot = false) {
    // Handle empty format
    if (!format) return {};
    if (std::string(format).empty()) return {};
    // Preprocessing step
    // 1. If the format starts with { and ends with } and this a root object remove them
    std::string formatStr = format;
    if (isRoot) {
        trimStr(formatStr);
        formatStr = unwrapString(formatStr, '{', '}');
    }
    // Main processing step
    std::vector<std::string> formatSplitted = splitString(formatStr, ",");
    ContractObject contractObject = {};
    int openBracketCount = 0;
    int closeBracketCount = 0;
    int openSquareBracketCount = 0;
    int closeSquareBracketCount = 0;
    int currentFieldIndex = 0;
    bool isInObject = false;
    bool isInArray = false;
    std::string currentNestedObject;
    std::string currentArrayObject;
    for (auto &fmt: formatSplitted) {
        if (fmt.empty()) continue;
        trimStr(fmt);

        if (fmt.front() == '{' && !isInArray && !isInObject) {
            isInObject = true;
        } else if (fmt.front() == '[' && !isInArray && !isInObject) {
            isInArray = true;
        }

        // Process nested object
        if (!isInArray && (isIncudedInStr(fmt, "{") || isIncudedInStr(fmt, "}"))) {
            if (!isInArray && !isInObject) {
                isInObject = true;
            }
            int newOpenBracketCount = std::count(fmt.begin(), fmt.end(), '{');
            int newCloseBracketCount = std::count(fmt.begin(), fmt.end(), '}');
            openBracketCount += newOpenBracketCount;
            closeBracketCount += newCloseBracketCount;

            if (openBracketCount == closeBracketCount) {
                currentNestedObject += fmt;
                openBracketCount = 0;
                closeBracketCount = 0;
                // Process the nested object
                currentNestedObject = unwrapString(currentNestedObject, '{', '}');
                ContractObject nestedObject = buildContractObject(currentNestedObject.c_str());
                contractObject.object[currentFieldIndex] = nestedObject;
                currentFieldIndex++;
                currentNestedObject = "";
                isInObject = false;
                continue;
            } else {
                currentNestedObject += fmt + ",";
                continue;
            }
        } else if (!currentNestedObject.empty()) {
            currentNestedObject += fmt + ",";
            continue;
        }

        // If there is no current nested object, process array of object
        if (!isInObject && (isIncudedInStr(fmt, "[") || isIncudedInStr(fmt, "]"))) {
            if (!isInArray && !isInObject) {
                isInArray = true;
            }
            int newOpenSquareBracketCount = std::count(fmt.begin(), fmt.end(), '[');
            int newCloseSquareBracketCount = std::count(fmt.begin(), fmt.end(), ']');
            openSquareBracketCount += newOpenSquareBracketCount;
            closeSquareBracketCount += newCloseSquareBracketCount;

            if (openSquareBracketCount == closeSquareBracketCount) {
                currentArrayObject += fmt;
                // We now should get something like [x;...] (x is optional)
                // Extract the x which is the max size of the array
                int arrayMaxSize = 0;
                if (isIncudedInStr(currentArrayObject, ";") && currentArrayObject.find_first_of(';') < currentArrayObject.find_first_of(',')) {
                    // There is x
                    std::string xStr = currentArrayObject.substr(1, currentArrayObject.find_first_of(';') - 1);
                    trimStr(xStr);
                    arrayMaxSize = std::stoi(xStr);

                    // Remove the x;
                    while (currentArrayObject[1] != ';') {
                        currentArrayObject.erase(1, 1);
                    }
                    currentArrayObject.erase(1, 1); // remove the ';'
                }
                openSquareBracketCount = 0;
                closeSquareBracketCount = 0;
                // Process the array of object
                currentArrayObject = unwrapString(currentArrayObject, '[', ']');
                ContractObject arrayObject = buildContractObject(currentArrayObject.c_str());
                arrayObject.isArray = true;
                arrayObject.arrayMaxSize = arrayMaxSize;
                contractObject.object[currentFieldIndex] = arrayObject;
                currentFieldIndex++;
                currentArrayObject = "";
                isInArray = false;
                continue;
            } else {
                currentArrayObject += fmt + ",";
                continue;
            }
        } else if (!currentArrayObject.empty()) {
            currentArrayObject += fmt + ",";
            continue;
        }

        // If there is no current nested object or array of object, process primitive
        if (currentNestedObject.empty() && currentArrayObject.empty()) {
            standardizeScFormat(fmt);
            bool isFound = false;
            for (auto &dataType: supportedDataTypes) {
                if (isIncudedInStr(fmt, dataType.first)) {
                    std::string plainData = fmt.substr(0, fmt.size() - dataType.first.size());
                    contractObject.primitive[currentFieldIndex] = {dataType.first, plainData};
                    currentFieldIndex++;
                    isFound = true;
                    break;
                }
            }

            if (!isFound) {
                throw std::runtime_error("Unsupported contract data type in format: " + fmt);
            }
        }
    }

    contractObject.numberOfFields = currentFieldIndex;
    return contractObject;
}