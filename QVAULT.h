#pragma once

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
