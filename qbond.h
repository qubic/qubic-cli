#pragma once
#include "structs.h"

struct Stake_input {
    int64_t millions;
};

struct TransferMBond_input {
    uint8_t newOwnerAndPossessor[32];
    int64_t epoch;
    int64_t numberOfMBonds;
};
struct TransferMBond_output {
    int64_t transferredMBonds;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct OrderOperation_input
{
    int64_t epoch;
    int64_t price;
    int64_t numberOfMBonds;
};
struct OrderOperation_output
{
    int64_t mBondsAmount;
};

struct Burn_input
{
    int64_t amount;
};
struct Burn_output
{
    int64_t amount;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct UpdateCFA_input
{
    uint8_t user[32];
    uint64_t operation;
};

struct GetInfoPerEpoch_input {
    int64_t epoch;
};
struct GetInfoPerEpoch_output {
    uint64_t stakersAmount;
    int64_t totalStaked;
    int64_t apy;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetFees_input
{
};
struct GetFees_output
{
    uint64_t stakeFeePercent;
    uint64_t tradeFeePercent;
    uint64_t transferFee;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetEarnedFees_output
{
    uint64_t stakeFees;
    uint64_t tradeFees;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct Order
{
    uint8_t owner[32];
    int64_t epoch;
    int64_t numberOfMBonds;
    int64_t price;
};

struct GetOrders_input
{
    int64_t epoch;
    int64_t asksOffset;
    int64_t bidsOffset;
};
struct GetOrders_output
{
    Order askOrders[256];
    Order bidOrders[256];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetUserOrders_input
{
    uint8_t owner[32];
    int64_t asksOffset;
    int64_t bidsOffset;
};
struct GetUserOrders_output
{
    Order askOrders[256];
    Order bidOrders[256];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct MBondsTable_input
{
};
struct MBondsTable_output
{
    struct TableEntry
    {
        int64_t epoch;
        int64_t totalStakedQBond;
        int64_t totalStakedQEarn;
        uint64_t apy;
    };
    TableEntry entries[512];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetUserMBonds_input
{
    uint8_t owner[32];
};
struct GetUserMBonds_output
{
    int64_t totalMBondsAmount;
    struct MBondEntity
    {
        int64_t epoch;
        int64_t amount;
        uint64_t apy;
    };
    MBondEntity mbonds[256];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetCFA_input
{
};
struct GetCFA_output
{
    uint8_t cfa[1024][32];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

void qbondStake(const char* nodeIp, int nodePort, const char* seed, const int64_t millionsOfQu, const uint32_t scheduledTickOffset);
void qbondTransfer(const char* nodeIp, int nodePort, const char* seed, const char* targetIdentity, const int64_t epoch, const int64_t mbondsAmount, const uint32_t scheduledTickOffset);
void qbondAddAskOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const uint32_t scheduledTickOffset);
void qbondRemoveAskOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const uint32_t scheduledTickOffset);
void qbondAddBidOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const uint32_t scheduledTickOffset);
void qbondRemoveBidOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const uint32_t scheduledTickOffset);
void qbondBurn(const char* nodeIp, int nodePort, const char* seed, const int64_t burnAmount, const uint32_t scheduledTickOffset);
void qbondUpdateCFA(const char* nodeIp, int nodePort, const char* seed, const char* user, const bool operation, const uint32_t scheduledTickOffset);
void qbondGetFees(const char* nodeIp, int nodePort);
void qbondGetEarnedFees(const char* nodeIp, int nodePort);
void qbondGetInfoPerEpoch(const char* nodeIp, int nodePort, const int64_t epoch);
void qbondGetOrders(const char* nodeIp, int nodePort, const int64_t epoch, const int64_t asksOffset, const int64_t bidsOffset);
void qbondGetUserOrders(const char* nodeIp, int nodePort, const char* owner, const int64_t asksOffset, const int64_t bidsOffset);
void qbondGetTable(const char* nodeIp, int nodePort);
void qbondGetUserMBonds(const char* nodeIp, int nodePort, const char* owner);
void qbondGetCFA(const char* nodeIp, int nodePort);

void qbondOperateOrder(const char* nodeIp, int nodePort, const char* seed, const int64_t epoch, const int64_t mbondPrice, const int64_t mbondsAmount, const int64_t fee, const uint16_t inputType, const uint32_t scheduledTickOffset);
void printOrders(const char* ordersType, const Order orders[]);
