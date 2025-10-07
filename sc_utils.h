#pragma once

#include <cstdint>
#include <map>


struct ContractDataInfo {
    int size;
    int alignment;
    std::map<int, unsigned long long> fieldOffsets; // key: field index, value: offset in bytes
};

void dumpContractToCSV(const char* input, uint32_t contractId, const char* output);
// Get size of the struct represented by format string (include padding for alignment)
ContractDataInfo getContractInputFormatInfo(const char* format);
// Create struct buffer from format string and output it to output pointer, return the size of the struct (including padding for alignment)
int packContractInputData(const char* format, void* output);
void printContractFormatData(const char* format, const void* input);