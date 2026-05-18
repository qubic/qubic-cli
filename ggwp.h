#pragma once

#include "structs.h"

// ─── Contract constants ────────────────────────────────────────
#define WP_CONTRACT_INDEX 27

// Query function numbers
#define WP_GET_STATUS           1
#define WP_GET_HOLDER_INFO      2
#define WP_GET_CLAN_MEMBER_INFO 3
#define WP_GET_SHAREHOLDER_INFO 4
#define WP_GET_STAKING_INFO     5
#define WP_GET_EXCLUDE_ADDRESSES 6
#define WP_GET_DISTRIBUTION_PREVIEW 7

// Procedure IDs
#define WP_PROC_DEPOSIT_REVENUE         1
#define WP_PROC_ADD_CLAN_MEMBER         2
#define WP_PROC_REMOVE_CLAN_MEMBER      3
#define WP_PROC_SET_CLAN_RANK           4
#define WP_PROC_SET_ADMIN               5
#define WP_PROC_SET_EXCLUDE_ADDRESS     6
#define WP_PROC_STAKE                   7
#define WP_PROC_REQUEST_UNSTAKE         8
#define WP_PROC_FINALIZE_UNSTAKE        9
#define WP_PROC_DEPOSIT_STAKING_REWARDS 10
#define WP_PROC_CLAIM_STAKING_REWARDS   11

// ─── Input/Output structs ──────────────────────────────────────

// fn 1 – GetStatus
struct WPGetStatus_output
{
    uint64_t holderCount;
    uint64_t totalTokensSnapshot;
    uint64_t shareholderCount;
    uint64_t totalSharesSnapshot;
    uint64_t clanMemberCount;
    uint64_t clanWeightedTotal;
    uint64_t pendingRevenue;
    uint64_t reinvestmentFund;
    uint64_t totalDistributed;
    uint64_t totalDeposited;
    uint64_t lastPayoutTick;
    uint64_t lastDistributionEpoch;
    uint8_t  adminAddress[32];
};

// fn 2 – GetHolderInfo
struct WPGetHolderInfo_input
{
    uint8_t holderAddress[32];
};
struct WPGetHolderInfo_output
{
    uint64_t tokenBalance;
    uint32_t isHolder;
};

// fn 4 – GetShareholderInfo
struct WPGetShareholderInfo_input
{
    uint8_t shareholderAddress[32];
};
struct WPGetShareholderInfo_output
{
    uint64_t shares;
    uint32_t isShareholder;
};

// fn 3 – GetClanMemberInfo
struct WPGetClanMemberInfo_input
{
    uint8_t memberAddress[32];
};
struct WPGetClanMemberInfo_output
{
    uint64_t rank;
    uint32_t isMember;
};

// proc 1 – DepositRevenue (no input, amount via invocationReward)

// proc 2 – AddClanMember
struct WPAddClanMember_input
{
    uint8_t memberAddress[32];
    uint64_t rank;
};

// proc 3 – RemoveClanMember
struct WPRemoveClanMember_input
{
    uint8_t memberAddress[32];
};

// proc 4 – SetClanRank
struct WPSetClanRank_input
{
    uint8_t memberAddress[32];
    uint64_t rank;
};

// proc 5 – SetAdmin
struct WPSetAdmin_input
{
    uint8_t newAdmin[32];
};

// proc 6 – SetExcludeAddress
struct WPSetExcludeAddress_input
{
    uint64_t slot;
    uint8_t address[32];
};

// fn 5 – GetStakingInfo
struct WPGetStakingInfo_input
{
    uint8_t stakerAddress[32];
};
struct WPGetStakingInfo_output
{
    uint64_t stakedAmount;
    uint64_t pendingRewards;
    uint64_t unstakeAmount;
    uint64_t unstakeEpoch;
    uint64_t totalStaked;
    uint64_t stakingRewardPool;
    uint32_t isStaker;
};

// fn 6 - GetExcludeAddresses
struct WPGetExcludeAddresses_output
{
    uint8_t address1[32];
    uint8_t address2[32];
};

// proc 7 – Stake
struct WPStake_input
{
    uint64_t numberOfShares;
};

// proc 8 – RequestUnstake
struct WPRequestUnstake_input
{
    uint64_t numberOfShares;
};

// proc 9 – FinalizeUnstake (no input)

// proc 10 – DepositStakingRewards
struct WPDepositStakingRewards_input
{
    uint64_t numberOfShares;
};

// proc 11 – ClaimStakingRewards (no input)

// ─── Function declarations ─────────────────────────────────────
void wpStatus(const char* nodeIp, int nodePort);
void wpHolderInfo(const char* nodeIp, int nodePort, const char* address);
void wpClanMemberInfo(const char* nodeIp, int nodePort, const char* address);
void wpShareholderInfo(const char* nodeIp, int nodePort, const char* address);
void wpDepositRevenue(const char* nodeIp, int nodePort, const char* seed,
                      uint64_t amount, uint32_t scheduledTickOffset);
void wpAddClanMember(const char* nodeIp, int nodePort, const char* seed,
                     const char* memberAddress, uint64_t rank, uint32_t scheduledTickOffset);
void wpRemoveClanMember(const char* nodeIp, int nodePort, const char* seed,
                        const char* memberAddress, uint32_t scheduledTickOffset);
void wpSetClanRank(const char* nodeIp, int nodePort, const char* seed,
                   const char* memberAddress, uint64_t rank, uint32_t scheduledTickOffset);
void wpSetAdmin(const char* nodeIp, int nodePort, const char* seed,
                const char* newAdmin, uint32_t scheduledTickOffset);
void wpSetExcludeAddress(const char* nodeIp, int nodePort, const char* seed,
                         uint64_t slot, const char* address, uint32_t scheduledTickOffset);

// ─── Staking ───────────────────────────────────────────────────
void wpStakingInfo(const char* nodeIp, int nodePort, const char* address);
void wpStake(const char* nodeIp, int nodePort, const char* seed,
             uint64_t numberOfShares, uint32_t scheduledTickOffset);
void wpRequestUnstake(const char* nodeIp, int nodePort, const char* seed,
                      uint64_t numberOfShares, uint32_t scheduledTickOffset);
void wpFinalizeUnstake(const char* nodeIp, int nodePort, const char* seed,
                       uint32_t scheduledTickOffset);
void wpDepositStakingRewards(const char* nodeIp, int nodePort, const char* seed,
                             uint64_t numberOfShares, uint32_t scheduledTickOffset);
void wpClaimStakingRewards(const char* nodeIp, int nodePort, const char* seed,
                           uint32_t scheduledTickOffset);
struct WPGetDistributionPreview_input { uint64_t amount; };
struct WPGetDistributionPreview_output {
    uint64_t holderShare;
    uint64_t shareholderShare;
    uint64_t clanShare;
    uint64_t reinvestShare;
};

void wpDistPreview(const char* nodeIp, int nodePort, uint64_t amount);
void wpExcludeAddresses(const char* nodeIp, int nodePort);
