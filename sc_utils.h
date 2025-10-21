#pragma once

#include <cstdint>
#include <map>
#include <string>

enum ValueType {
    SINT8,
    UINT8,
    SINT16,
    UINT16,
    SINT32,
    UINT32,
    SINT64,
    UINT64,
    UNKNOWN
};

struct ContractPrimitive {
    std::string type;
    std::string value;

    unsigned long long getSize();
    unsigned long long getAlignment();
    void dumpIntoBuffer(void *buffer);
    static ContractPrimitive fromBuffer(const uint8_t *buffer, std::string type);
    void print(int indent = 0);
    std::string toString();
    bool isEmpty();
};

// A struct respersent data type in a contract, it can be primitive, object or array of object
struct ContractObject {
    std::map<int, ContractPrimitive> primitive;
    std::map<int, ContractObject> object;
    bool isArray;
    int arrayMaxSize;
    int numberOfFields;

    unsigned long long getSize();
    unsigned long long getAlignment();
    void dumpIntoBuffer(void *buffer);
    static ContractObject fromBuffer(void *buffer, const char* format, bool isRoot = false);
    void print(int indent = 1, bool hasNext = false);
    std::string toString();
    bool isEmpty();
};

void dumpContractToCSV(const char* input, uint32_t contractId, const char* output);
ContractObject buildContractObject(const char *format, bool isRoot);