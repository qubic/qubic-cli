#pragma once
void printWalletInfo(const char* seed);
void printBalance(const char* publicIdentity, const char* nodeIp, int nodePort);
void makeStandardTransaction(const char* nodeIp, int nodePort, const char* seed,
                             const char* targetIdentity, const uint64_t amount, uint32_t scheduledTickOffset);
void makeCustomTransaction(const char* nodeIp, int nodePort,
                           const char* seed,
                           const char* targetIdentity,
                           ushort txType,
                           uint64_t amount,
                           int extraDataSize,
                           const uint8_t* extraData,
                           uint32_t scheduledTickOffset);

void printReceipt(Transaction& tx, const char* txHash = nullptr, const uint8_t* extraData = nullptr);
bool verifyTx(Transaction& tx, const uint8_t* extraData, const uint8_t* signature);