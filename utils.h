#pragma once
#include <random>
#include <cstring>
#include <string>
#include <vector>

extern char* g_printToScreen;

static const char B64_TABLE[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void byteToHex(const uint8_t* byte, char* hex, const int sizeInByte)
{
    for (int i = 0; i < sizeInByte; i++)
    {
        snprintf(hex+i*2, 3, "%02x", byte[i]);
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
static uint32_t getRand32()
{
    uint32_t r;
    rand32(&r);
    return r;
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
#ifdef _MSC_VER
    char* wStr = _strdup(str);
#else
    char* wStr = strdup(str);
#endif
    
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


static void trimStr( std::string& str) {
    while (!str.empty() && isspace(str.front())) {
        str.erase(str.begin());
    }
    while (!str.empty() && isspace(str.back())) {
        str.pop_back();
    }
}

static bool isIncudedInStr(const std::string& str, const std::string& pattern) {
    return str.find(pattern) != std::string::npos;
}
#ifdef _MSC_VER
static inline int strcasecmp(const char* s1, const char* s2)
{
    return _stricmp(s1, s2);
}
#endif

static std::string unwrapString(const std::string& str, char wrapperCharFront, char wrapperCharBack) {
    if (str.size() >= 2 && str.front() == wrapperCharFront && str.back() == wrapperCharBack) {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

static inline std::string base64_encode(const std::vector<uint8_t> &in) {
    std::string out;
    int val = 0, valb = -6;

    for (uint8_t c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(B64_TABLE[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(B64_TABLE[((val << 8) >> (valb + 8)) & 0x3F]);

    while (out.size() % 4)
        out.push_back('=');

    return out;
}

static std::string base64_encode(uint8_t *data, size_t length) {
    return base64_encode(std::vector<uint8_t>(data, data + length));
}

static std::vector<uint8_t> base64_decode(const std::string &in) {
    static int T[256];
    static bool init = false;

    if (!init) {
        for (int i = 0; i < 256; i++) T[i] = -1;
        for (int i = 0; i < 64; i++) T[(unsigned char)B64_TABLE[i]] = i;
        init = true;
    }

    std::vector<uint8_t> out;
    int val = 0, valb = -8;

    for (unsigned char c : in) {
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;

        if (valb >= 0) {
            out.push_back(uint8_t((val >> valb) & 0xFF));
            valb -= 8;
        }
    }

    return out;
}

static void printBytes(uint8_t* data, size_t length, std::string type = "base64") {\
    printf("---------------- %s ----------------\n", type.c_str());
   if (type == "base64") {
       std::string encoded = base64_encode(data, length);
       printf("%s\n", encoded.c_str());
    } else if (type == "hex") {
         for (size_t i = 0; i < length; i++) {
              printf("%02x", data[i]);
         }
         printf("\n");
    } else {
         printf("Unsupported print type: %s\n", type.c_str());
    }
}