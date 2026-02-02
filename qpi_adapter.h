#pragma once

#if !defined(NDEBUG)
#define NDEBUG
#endif

// For GCC/Clang, provide implementations of MSVC intrinsics used in qpi.h
#if !defined(_MSC_VER)
    #include <cstdint>

    // Signed 64-bit multiply returning low 64 bits and high 64 bits
    inline long long int _mul128(long long int a, long long int b, long long int* high) {
        __int128 result = static_cast<__int128>(a) * static_cast<__int128>(b);
        *high = static_cast<long long int>(result >> 64);
        return static_cast<long long int>(result);
    }

    // Unsigned 64-bit multiply returning low 64 bits and high 64 bits
    inline long long unsigned int _umul128(long long unsigned int a, long long unsigned int b, long long unsigned int* high) {
        unsigned __int128 result = static_cast<unsigned __int128>(a) * static_cast<unsigned __int128>(b);
        *high = static_cast<long long unsigned int>(result >> 64);
        return static_cast<long long unsigned int>(result);
    }
#endif

#include "core/src/contract_core/pre_qpi_def.h"
#include "core/src/contracts/qpi.h"
#include "core/src/oracle_core/oracle_interfaces_def.h"

#include <string>
#include <vector>

// define static functions declared by qpi.h
static void* __acquireScratchpad(unsigned long long size, bool initZero) { return nullptr; }
static void __releaseScratchpad(void* ptr) {}
void QPI::AssetIssuanceIterator::begin(const QPI::AssetIssuanceSelect&) {}
QPI::uint64 QPI::AssetIssuanceIterator::assetName() const { return 0; }
QPI::id QPI::AssetIssuanceIterator::issuer() const { return QPI::id::zero(); }
void QPI::AssetOwnershipIterator::begin(const QPI::Asset&, const QPI::AssetOwnershipSelect&) {}
void QPI::AssetPossessionIterator::begin(const QPI::Asset&, const QPI::AssetOwnershipSelect&, const QPI::AssetPossessionSelect&) {}
template <typename T1, typename T2> void QPI::copyMemory(T1&, const T2&) {}


// define additional functions implemented by qpi:adapter.cpp
std::string toString(const QPI::id& id);
std::string toString(const QPI::DateAndTime& dt);
std::string oracleQueryToString(uint32_t interfaceIndex, const std::vector<uint8_t>& query);
std::string oracleReplyToString(uint32_t interfaceIndex, const std::vector<uint8_t>& reply);
