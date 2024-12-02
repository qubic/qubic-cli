#pragma once

#include "structs.h"

// SC structs

struct QVaultGetData_input
{
    uint32_t t;
};
struct QVaultGetData_output
{
    uint64_t numberOfBannedAddress;
    uint32_t computor_permille;
    uint32_t QCAPHolder_permille;
    uint32_t reinvesting_permille;
    uint32_t dev_permille;
    uint8_t AUTH_ADDRESS1[32];
    uint8_t AUTH_ADDRESS2[32];
    uint8_t AUTH_ADDRESS3[32];
    uint8_t Reinvesting_address[32];
    uint8_t admin_address[32];
    uint8_t newAuthAddress1[32];
    uint8_t newAuthAddress2[32];
    uint8_t newAuthAddress3[32];
    uint8_t newReinvesting_address1[32];
    uint8_t newReinvesting_address2[32];
    uint8_t newReinvesting_address3[32];
    uint8_t newAdmin_address1[32];
    uint8_t newAdmin_address2[32];
    uint8_t newAdmin_address3[32];
    uint8_t bannedAddress1[32];
    uint8_t bannedAddress2[32];
    uint8_t bannedAddress3[32];
    uint8_t unbannedAddress1[32];
    uint8_t unbannedAddress2[32];
    uint8_t unbannedAddress3[32];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

void submitAuthAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
void changeAuthAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t numberOfChangedAddress);
void submitFees(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t newQCAPHolder_fee, uint32_t newreinvesting_fee, uint32_t newdev_fee);
void changeFees(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t newQCAPHolder_fee, uint32_t newreinvesting_fee, uint32_t newdev_fee);
void submitReinvestingAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
void changeReinvestingAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
void submitAdminAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
void changeAdminAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
void getData(const char* nodeIp, int nodePort);
void submitBannedAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
void saveBannedAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
void submitUnbannedannedAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
void saveUnbannedAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity);
