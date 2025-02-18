#pragma once

#include <random>
#include <cstring>
#include <string>
#include <vector>

static void byteToHex(const uint8_t* byte, char* hex, const int sizeInByte)
{
    for (int i = 0; i < sizeInByte; i++)
    {
        sprintf(hex+i*2, "%02x", byte[i]);
    }
}
static void hexToByte(const char* hex, uint8_t* byte, const int sizeInByte)
{
    for (int i = 0; i < sizeInByte; i++)
    {
        sscanf(hex+i*2, "%2hhx", &byte[i]);
    }
}

static void rand32(uint32_t* r)
{
    std::random_device rd;
    static thread_local std::mt19937 generator(rd());
    std::uniform_int_distribution<uint32_t> distribution(0,UINT32_MAX);
    *r = distribution(generator);
}

static void rand64(uint64_t* r)
{
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<uint64_t> distribution(0,UINT32_MAX);
    *r = distribution(generator);
}

static inline std::string strtok2string(char* s, const char* delimiter)
{
    const char* res = strtok(s, delimiter);
    return (res) ? res : "";
}

static inline std::vector<std::string> splitString(const char* str, const char* delimiter)
{
    std::vector<std::string> vec;
    char* wStr = strdup(str);
    const char* res = strtok(wStr, delimiter);
    while (res)
    {
        vec.push_back(res);
        res = strtok(nullptr, delimiter);
    }
    free(wStr);
    return vec;
}

static inline std::vector<std::string> splitString(const std::string& str, const char* delimiter)
{
    return splitString(str.c_str(), delimiter);
}
