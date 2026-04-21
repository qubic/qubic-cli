#pragma once

#include "structs.h"

// ─── Contract constants ────────────────────────────────────────
#define QRWA_CONTRACT_INDEX 20

// Query function numbers (inputType in RequestContractFunction)
#define QRWA_GET_GOV_PARAMS           1
#define QRWA_GET_GOV_POLL             2
#define QRWA_GET_TREASURY_BALANCE     4
#define QRWA_GET_DIVIDEND_BALANCES    5
#define QRWA_GET_TOTAL_DISTRIBUTED    6
#define QRWA_GET_CONTRACT_ADDRESSES  12
#define QRWA_GET_ACTIVE_GOV_POLL_IDS  8
#define QRWA_GET_GENERAL_ASSET_BAL    9
#define QRWA_GET_GENERAL_ASSETS      10
#define QRWA_GET_PAYOUTS_QMINE       11
#define QRWA_GET_PAYOUTS_QRWA        13
#define QRWA_GET_PAYOUTS_DEDICATED   14
#define QRWA_GET_SC_DIVIDEND_TRACKING 15
#define QRWA_GET_PAYOUTS_POOL_D      16

// Procedure IDs (inputType in InvokeContractProcedure)
#define QRWA_PROC_DONATE_TO_TREASURY    3
#define QRWA_PROC_VOTE_GOV_PARAMS       4
#define QRWA_PROC_DEPOSIT_GENERAL_ASSET 7
#define QRWA_PROC_REVOKE_ASSET_MGMT    8
#define QRWA_PROC_SET_POOL_A_ADDRESS    9
#define QRWA_PROC_SET_POOL_D_ADDRESS   10

constexpr uint64_t QRWA_PAYOUT_RING_SIZE = 16384;
constexpr uint64_t QRWA_PAYOUT_PAGE_SIZE = 512;
constexpr uint64_t QRWA_MAX_ASSETS = 1024;
constexpr uint64_t QRWA_MAX_GOV_POLLS = 64;

// Payout types
constexpr uint8_t QRWA_PAYOUT_TYPE_QMINE_HOLDER   = 0;
constexpr uint8_t QRWA_PAYOUT_TYPE_QMINE_DEV      = 1;
constexpr uint8_t QRWA_PAYOUT_TYPE_QRWA_HOLDER    = 2;
constexpr uint8_t QRWA_PAYOUT_TYPE_DEDICATED_QRWA = 3;
constexpr uint8_t QRWA_PAYOUT_TYPE_POOL_D_QRWA    = 4;

// ─── Input/Output structs ──────────────────────────────────────

struct QRWAPayoutEntry
{
    uint8_t  recipient[32];
    uint64_t amount;
    uint64_t qmineHolding;
    uint64_t qrwaHolding;
    uint32_t tick;
    uint16_t epoch;
    uint8_t  payoutType;
    uint8_t  _pad0;
};

// fn 1
struct QRWAGovParams
{
    uint8_t  mAdminAddress[32];
    uint8_t  electricityAddress[32];
    uint8_t  maintenanceAddress[32];
    uint8_t  reinvestmentAddress[32];
    uint8_t  qmineDevAddress[32];
    uint64_t electricityPercent;
    uint64_t maintenancePercent;
    uint64_t reinvestmentPercent;
};
struct QRWAGetGovParams_output
{
    QRWAGovParams params;
};

// fn 2
struct QRWAGovProposal
{
    uint64_t proposalId;
    uint64_t status;   // 0=Empty, 1=Active, 2=Passed, 3=Failed
    uint64_t score;
    QRWAGovParams params;
};
struct QRWAGetGovPoll_input
{
    uint64_t proposalId;
};
struct QRWAGetGovPoll_output
{
    QRWAGovProposal proposal;
    uint64_t status; // 0=NotFound, 1=Found
};

// fn 4
struct QRWAGetTreasuryBalance_output
{
    uint64_t balance;
};

// fn 5
struct QRWAGetDividendBalances_output
{
    uint64_t revenuePoolA;
    uint64_t revenuePoolB;
    uint64_t dedicatedRevenuePool;
    uint64_t poolAQmineDividend;
    uint64_t poolAQrwaDividend;
    uint64_t poolBQmineDividend;
    uint64_t poolBQrwaDividend;
    uint64_t poolCQmineDividend;
    uint64_t poolCQrwaDividend;
    uint64_t poolDRevenuePool;
    uint64_t poolDQmineDividend;
    uint64_t poolDQrwaDividend;
};

// fn 6
struct QRWAGetTotalDistributed_output
{
    uint64_t totalPoolADistributed;
    uint64_t totalPoolBDistributed;
    uint64_t totalPoolCDistributed;
    uint64_t totalPoolDDistributed;
    uint64_t payoutTotalQmineBegin;
};

// fn 7
struct QRWAGetContractAddresses_output
{
    uint8_t dedicatedRevenueAddress[32];
    uint8_t poolARevenueAddress[32];
    uint8_t poolDRevenueAddress[32];
    uint8_t fundraisingAddress[32];
    uint8_t exchangeAddress[32];
};

// fn 8
struct QRWAGetActiveGovPollIds_output
{
    uint64_t count;
    uint64_t ids[QRWA_MAX_GOV_POLLS]; // Array<uint64, 64>
};

// fn 9
struct QRWAGetGeneralAssetBalance_input
{
    uint8_t issuer[32];
    uint64_t assetName;
};
struct QRWAGetGeneralAssetBalance_output
{
    uint64_t balance;
    uint64_t status; // 1=found, 0=not found
};

// fn 10
struct QRWAAssetEntry
{
    uint8_t  issuer[32];
    uint64_t assetName;
};
struct QRWAGetGeneralAssets_output
{
    uint64_t count;
    QRWAAssetEntry assets[QRWA_MAX_ASSETS];
    uint64_t balances[QRWA_MAX_ASSETS];
};

// Pagination input (shared layout for fn 11, 13, 14)
struct QRWAGetPayouts_input
{
    uint16_t page; // 0 = newest entries
};

// fn 11
struct QRWAGetPayoutsQmine_output
{
    QRWAPayoutEntry payouts[QRWA_PAYOUT_PAGE_SIZE];
    uint16_t nextIdx;
    uint16_t returnedCount;
    uint16_t page;
    uint16_t totalPages;
};

// fn 13
struct QRWAGetPayoutsQrwa_output
{
    QRWAPayoutEntry payouts[QRWA_PAYOUT_PAGE_SIZE];
    uint16_t nextIdx;
    uint16_t returnedCount;
    uint16_t page;
    uint16_t totalPages;
};

// fn 14
struct QRWAGetPayoutsDedicated_output
{
    QRWAPayoutEntry payouts[QRWA_PAYOUT_PAGE_SIZE];
    uint16_t nextIdx;
    uint16_t returnedCount;
    uint16_t page;
    uint16_t totalPages;
};

// fn 16
struct QRWAGetPayoutsPoolD_output
{
    QRWAPayoutEntry payouts[QRWA_PAYOUT_PAGE_SIZE];
    uint16_t nextIdx;
    uint16_t returnedCount;
    uint16_t page;
    uint16_t totalPages;
};

// fn 15
struct QRWAGetScDividendTracking_output
{
    uint64_t count;
    uint8_t  scContractIds[QRWA_MAX_ASSETS][32];   // Array<id, 1024>
    uint64_t cumulativeDividends[QRWA_MAX_ASSETS];  // Array<uint64, 1024>
};

// ─── Pool selector enum ────────────────────────────────────────
enum QRWAPoolType
{
    QRWA_POOL_A,        // Pool A — Qubic Mining (fn 11)
    QRWA_POOL_B,        // Pool B — SC Assets Revenue (fn 13)
    QRWA_POOL_C,        // Pool C — BTC Mining (fn 14)
    QRWA_POOL_D,        // Pool D — MLM Water (fn 16)
};

// ─── Procedure input/output structs ────────────────────────────

// proc 3
struct QRWADonateToTreasury_input
{
    uint64_t amount;
};
struct QRWADonateToTreasury_output
{
    uint64_t status;
};

// proc 4
struct QRWAVoteGovParams_input
{
    QRWAGovParams proposal;
};
struct QRWAVoteGovParams_output
{
    uint64_t status;
};

// proc 6
struct QRWASetPoolARevenueAddress_input
{
    uint8_t newAddress[32];
};
struct QRWASetPoolARevenueAddress_output
{
    uint64_t status;
};

// proc 10
struct QRWASetPoolDRevenueAddress_input
{
    uint8_t newAddress[32];
};
struct QRWASetPoolDRevenueAddress_output
{
    uint64_t status;
};

// proc 7
struct QRWADepositGeneralAsset_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t amount;
};
struct QRWADepositGeneralAsset_output
{
    uint64_t status;
};

// proc 8
struct QRWARevokeAssetManagementRights_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    int64_t numberOfShares;
};
struct QRWARevokeAssetManagementRights_output
{
    int64_t transferredNumberOfShares;
    uint64_t status;
};

// ─── CLI function declarations ─────────────────────────────────
// epochFilter: -1 = show all, >=0 = filter by epoch
void qrwaPayout(const char* nodeIp, int nodePort, QRWAPoolType pool, int epochFilter = -1);
void qrwaPayoutAddress(const char* nodeIp, int nodePort, const char* identity, int epochFilter = -1);
void qrwaStatus(const char* nodeIp, int nodePort);
void qrwaAssets(const char* nodeIp, int nodePort);
void qrwaGovParams(const char* nodeIp, int nodePort);
void qrwaGovPoll(const char* nodeIp, int nodePort, uint64_t proposalId);
void qrwaGovPollIds(const char* nodeIp, int nodePort);
void qrwaDividends(const char* nodeIp, int nodePort);
void qrwaScDividends(const char* nodeIp, int nodePort);
void qrwaDonateToTreasury(const char* nodeIp, int nodePort, const char* seed, uint64_t amount, uint32_t scheduledTickOffset);
void qrwaVoteGovParams(const char* nodeIp, int nodePort, const char* seed, const QRWAGovParams& proposal, uint32_t scheduledTickOffset);
void qrwaSetPoolARevenueAddress(const char* nodeIp, int nodePort, const char* seed, const char* newAddress, uint32_t scheduledTickOffset);
void qrwaSetPoolDRevenueAddress(const char* nodeIp, int nodePort, const char* seed, const char* newAddress, uint32_t scheduledTickOffset);
void qrwaDepositGeneralAsset(const char* nodeIp, int nodePort, const char* seed, const char* issuerIdentity, const char* assetName, uint64_t amount, uint32_t scheduledTickOffset);
void qrwaRevokeAssetMgmt(const char* nodeIp, int nodePort, const char* seed, const char* issuerIdentity, const char* assetName, int64_t numberOfShares, uint32_t scheduledTickOffset);
