#pragma once

#include "structs.h"
#include "connection.h"

struct VanityAddress {
  char seed[55 + 1];
  char identity[60 + 1];
};

void printWalletInfo(const char* seed);
void printBalance(const char* publicIdentity, const char* nodeIp, int nodePort);
void makeStandardTransaction(const char* nodeIp, int nodePort, const char* seed,
                             const char* targetIdentity, const uint64_t amount, uint32_t scheduledTickOffset,
                             int waitUntilFinish);

void makeStandardTransactionInTick(const char* nodeIp, int nodePort, const char* seed,
                             const char* targetIdentity, const uint64_t amount, uint32_t txTick,
                             int waitUntilFinish);

void makeCustomTransaction(const char* nodeIp, int nodePort,
                           const char* seed,
                           const char* targetIdentity,
                           uint16_t txType,
                           uint64_t amount,
                           int extraDataSize,
                           const uint8_t* extraData,
                           uint32_t scheduledTickOffset);
void makeContractTransaction(const char* nodeIp, int nodePort,
                             const char* seed,
                             uint64_t contractIndex,
                             uint16_t txType,
                             uint64_t amount,
                             int extraDataSize,
                             const void* extraData,
                             uint32_t scheduledTickOffset,
                             QCPtr* qcPtr = nullptr);
bool runContractFunction(const char* nodeIp, int nodePort,
    unsigned int contractIndex,
    unsigned short funcNumber,
    void* inputPtr,
    size_t inputSize,
    void* outputPtr,
    size_t outputSize,
    QCPtr* qcPtr = nullptr);
bool runContractFunction(const char* nodeIp, int nodePort,
    unsigned int contractIndex,
    unsigned short funcNumber,
    const char* formatInput,
    const char* formatOutput);
void invokeContractProcedure(const char* nodeIp, int nodePort,
    const char* seed,
    uint64_t contractIndex,
    uint16_t txType,
    uint64_t amount,
    const char* formatInput,
    uint32_t scheduledTickOffset);

void printReceipt(Transaction& tx, const char* txHash = nullptr, const uint8_t* extraData = nullptr, int moneyFlew = -1);
bool verifyTx(Transaction& tx, const uint8_t* extraData, const uint8_t* signature);
void makeIPOBid(const char* nodeIp, int nodePort,
                const char* seed,
                uint32_t contractIndex,
                uint64_t pricePerShare,
                uint16_t numberOfShare,
                uint32_t scheduledTickOffset);
void printIPOStatus(const char* nodeIp, int nodePort, uint32_t contractIndex);
void printActiveIPOs(const char* nodeIp, int nodePort);
VanityAddress generateVanityAddress(const char* pattern, unsigned int vanityGenerationThreads, bool isSufix = false);
