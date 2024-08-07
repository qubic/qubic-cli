#pragma once

void qearnLock(const char* nodeIp, int nodePort, const char* seed, long long lock_amount, uint32_t scheduledTickOffset);
void qearnUnlock(const char* nodeIp, int nodePort, const char* seed, long long unlock_amount, uint32_t locked_epoch, uint32_t scheduledTickOffset);
void qearnGetInforPerEpoch(const char* nodeIp, int nodePort, uint32_t epoch);
void qearnGetUserLockedInfor(const char* nodeIp, int nodePort);