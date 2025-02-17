#pragma once

#include "structs.h"
#include <cstdint>

#define MSVAULT_CONTRACT_INDEX 11
#define MSVAULT_MAX_OWNERS 16
#define MSVAULT_MAX_COOWNER 8

struct MsVaultRegisterVault_input {
    uint8_t vaultName[32];
    // owners: 32 owners, each 32-byte
    uint8_t owners[MSVAULT_MAX_OWNERS * 32];
    uint64_t requiredApprovals;
};
struct MsVaultRegisterVault_output {
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultDeposit_input {
    uint64_t vaultID;
};
struct MsVaultDeposit_output {
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultReleaseTo_input {
    uint64_t vaultID;
    uint64_t amount;
    uint8_t destination[32];
};
struct MsVaultReleaseTo_output {
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultResetRelease_input {
    uint64_t vaultID;
};
struct MsVaultResetRelease_output {
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetVaults_input {
    uint8_t publicKey[32];
};
struct MsVaultGetVaults_output {
    uint64_t numberOfVaults;
    uint64_t vaultIDs[MSVAULT_MAX_COOWNER];
    uint8_t vaultNames[MSVAULT_MAX_COOWNER][32];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetReleaseStatus_input {
    uint64_t vaultID;
};
struct MsVaultGetReleaseStatus_output {
    uint64_t status;
    uint64_t amounts[MSVAULT_MAX_OWNERS];
    uint8_t destinations[MSVAULT_MAX_OWNERS][32];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetBalanceOf_input {
    uint64_t vaultID;
};
struct MsVaultGetBalanceOf_output {
    uint64_t status;
    int64_t balance;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetVaultName_input {
    uint64_t vaultID;
};
struct MsVaultGetVaultName_output {
    uint64_t status;
    uint8_t vaultName[32];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetRevenueInfo_input {};
struct MsVaultGetRevenueInfo_output {
    uint64_t numberOfActiveVaults;
    uint64_t totalRevenue;
    uint64_t totalDistributedToShareholders;
    uint64_t burnedAmount;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetFees_input {
};
struct MsVaultGetFees_output {
    uint64_t registeringFee;
    uint64_t releaseFee;
    uint64_t releaseResetFee;
    uint64_t holdingFee;
    uint64_t depositFee; // always 0 for now
    uint64_t burnFee;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetVaultOwners_input {
    uint64_t vaultID;
};
struct MsVaultGetVaultOwners_output {
    uint64_t status;
    uint64_t numberOfOwners;
    uint8_t owners[MSVAULT_MAX_OWNERS][32];  // 16 owners, each 32 bytes
    uint64_t requiredApprovals;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

void msvaultRegisterVault(const char* nodeIp, int nodePort, const char* seed,
    uint64_t requiredApprovals, const uint8_t vaultName[32],
    const char* ownersCommaSeparated,
    uint32_t scheduledTickOffset);

void msvaultDeposit(const char* nodeIp, int nodePort, const char* seed,
    uint64_t vaultID, uint64_t amount, uint32_t scheduledTickOffset);

void msvaultReleaseTo(const char* nodeIp, int nodePort, const char* seed,
    uint64_t vaultID, uint64_t amount, const char* destinationIdentity,
    uint32_t scheduledTickOffset);

void msvaultResetRelease(const char* nodeIp, int nodePort, const char* seed,
    uint64_t vaultID, uint32_t scheduledTickOffset);

void msvaultGetVaults(const char* nodeIp, int nodePort, const char* identity);
void msvaultGetReleaseStatus(const char* nodeIp, int nodePort, uint64_t vaultID);
void msvaultGetBalanceOf(const char* nodeIp, int nodePort, uint64_t vaultID);
void msvaultGetVaultName(const char* nodeIp, int nodePort, uint64_t vaultID);
void msvaultGetRevenueInfo(const char* nodeIp, int nodePort);
void msvaultGetFees(const char* nodeIp, int nodePort);
void msvaultGetVaultOwners(const char* nodeIp, int nodePort, uint64_t vaultID);
