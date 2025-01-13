#pragma once

#include "structs.h"

// SC structs

struct QEarnGetLockInfoPerEpoch_input
{
    uint32_t Epoch;                             /* epoch number to get information */
};
struct QEarnGetLockInfoPerEpoch_output
{
    uint64_t LockedAmount;                      /* initial total locked amount in epoch */
    uint64_t BonusAmount;                       /* initial bonus amount in epoch*/
    uint64_t CurrentLockedAmount;               /* total locked amount in epoch. exactly the amount excluding the amount unlocked early*/
    uint64_t CurrentBonusAmount;                /* bonus amount in epoch excluding the early unlocking */
    uint64_t Yield;                             /* Yield calculated by 10000000 multiple*/

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QEarnGetUserLockedInfo_input
{
    uint8_t publicKey[32];
    uint32_t epoch;
};
struct QEarnGetUserLockedInfo_output
{
    uint64_t LockedAmount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QEarnGetStateOfRound_input
{
    uint32_t Epoch;
};
struct QEarnGetStateOfRound_output
{
    uint32_t state;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QEarnGetUserLockStatus_input
{
    uint8_t publicKey[32];
};
struct QEarnGetUserLockStatus_output
{
    uint64_t status;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QEarnGetStatsPerEpoch_input {
    uint32_t epoch;
};

struct QEarnGetStatsPerEpoch_output {

    uint64_t earlyUnlockedAmount;
    uint64_t earlyUnlockedPercent;
    uint64_t totalLockedAmount;
    uint64_t averageAPY;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QEarnGetBurnedAndBoostedStats_input {
    uint32_t t;
};

struct QEarnGetBurnedAndBoostedStats_output {

    uint64_t burnedAmount;
    uint64_t averageBurnedPercent;
    uint64_t boostedAmount;
    uint64_t averageBoostedPercent;
    uint64_t rewardedAmount;
    uint64_t averageRewardedPercent;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QEarnGetBurnedAndBoostedStatsPerEpoch_input {
    uint32_t epoch;
};

struct QEarnGetBurnedAndBoostedStatsPerEpoch_output {

    uint64_t burnedAmount;
    uint64_t burnedPercent;
    uint64_t boostedAmount;
    uint64_t boostedPercent;
    uint64_t rewardedAmount;
    uint64_t rewardedPercent;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }

};

struct QEarnGetEndedStatus_input
{
    uint8_t publicKey[32];
};
struct QEarnGetEndedStatus_output
{
    uint64_t Fully_Unlocked_Amount;
    uint64_t Fully_Rewarded_Amount;
    uint64_t Early_Unlocked_Amount;
    uint64_t Early_Rewarded_Amount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

void qearnLock(const char* nodeIp, int nodePort, const char* seed, long long lock_amount, uint32_t scheduledTickOffset);
void qearnUnlock(const char* nodeIp, int nodePort, const char* seed, long long unlock_amount, uint32_t locked_epoch, uint32_t scheduledTickOffset);
void qearnGetInfoPerEpoch(const char* nodeIp, int nodePort, uint32_t epoch);
void qearnGetUserLockedInfo(const char* nodeIp, int nodePort,char* Identity,  uint32_t epoch);
void qearnGetStateOfRound(const char* nodeIp, int nodePort, uint32_t epoch);
void qearnGetUserLockedStatus(const char* nodeIp, int nodePort,char* Identity);
void qearnGetEndedStatus(const char* nodeIp, int nodePort,char* Identity);
void qearnGetStatsPerEpoch(const char* nodeIp, int nodePort, uint32_t epoch);
void qearnGetBurnedAndBoostedStats(const char* nodeIp, int nodePort);
void qearnGetBurnedAndBoostedStatsPerEpoch(const char* nodeIp, int nodePort, uint32_t epoch);
