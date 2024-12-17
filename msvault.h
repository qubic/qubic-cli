#pragma once

#include "structs.h"
#include <cstdint>

#define MSVAULT_CONTRACT_INDEX 11

struct MsVaultRegisterVault_input {
    uint16_t vaultType;
    uint8_t vaultName[32];
    // owners: 32 owners, each 32-byte
    uint8_t owners[32 * 32]; // owners[i*32 ... i*32+31]
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
    uint16_t numberOfVaults;
    uint64_t vaultIDs[1024];
    uint8_t vaultNames[1024][32];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetReleaseStatus_input {
    uint64_t vaultID;
};
struct MsVaultGetReleaseStatus_output {
    uint64_t amounts[32];
    uint8_t destinations[32][32];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetBalanceOf_input {
    uint64_t vaultID;
};
struct MsVaultGetBalanceOf_output {
    int64_t balance;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetVaultName_input {
    uint64_t vaultID;
};
struct MsVaultGetVaultName_output {
    uint8_t vaultName[32];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MsVaultGetRevenueInfo_input {};
struct MsVaultGetRevenueInfo_output {
    uint32_t numberOfActiveVaults;
    uint64_t totalRevenue;
    uint64_t totalDistributedToShareholders;
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
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

void msvaultRegisterVault(const char* nodeIp, int nodePort, const char* seed,
    uint16_t vaultType, const uint8_t vaultName[32],
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
