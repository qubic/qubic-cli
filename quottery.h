#pragma once

#include "structs.h"

// SC structs
struct QuotteryjoinBet_input
{
    uint32_t betId;
    int numberOfSlot;
    uint32_t option;
    uint32_t _placeHolder;
};

struct QuotteryissueBet_input
{
    uint8_t betDesc[32];
    uint8_t optionDesc[32*8];
    uint8_t oracleProviderId[32*8];
    uint32_t oracleFees[8];
    uint32_t closeDate;
    uint32_t endDate;
    uint64_t amountPerSlot;
    uint32_t maxBetSlotPerOption;
    uint32_t numberOfOption;
};

struct qtryBasicInfo_output
{
    uint64_t feePerSlotPerHour; // Amount of qus
    uint64_t gameOperatorFee; // 4 digit number ABCD means AB.CD% | 1234 is 12.34%
    uint64_t shareholderFee; // 4 digit number ABCD means AB.CD% | 1234 is 12.34%
    uint64_t minBetSlotAmount; // amount of qus
    uint64_t burnFee; // percentage
    uint64_t nIssuedBet; // number of issued bet
    uint64_t moneyFlow;
    uint64_t moneyFlowThroughIssueBet;
    uint64_t moneyFlowThroughJoinBet;
    uint64_t moneyFlowThroughFinalizeBet;
    uint64_t earnedAmountForShareHolder;
    uint64_t paidAmountForShareHolder;
    uint64_t earnedAmountForBetWinner;
    uint64_t distributedAmount;
    uint64_t burnedAmount;
    uint8_t gameOperator[32];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct getBetInfo_input
{
    uint32_t betId;
};
struct getBetInfo_output
{
    // meta data info
    uint32_t betId;
    uint32_t nOption;      // options number
    uint8_t creator[32];
    uint8_t betDesc[32];      // 32 bytes
    uint8_t optionDesc[8*32];  // 8x(32)=256bytes
    uint8_t oracleProviderId[8*32]; // 256x8=2048bytes
    uint32_t oracleFees[8];   // 4x8 = 32 bytes

    uint32_t openDate;     // creation date, start to receive bet
    uint32_t closeDate;    // stop receiving bet date
    uint32_t endDate;       // result date
    // Amounts and numbers
    uint64_t minBetAmount;
    uint32_t maxBetSlotPerOption;
    uint32_t currentBetState[8]; // how many bet slots have been filled on each option
    char betResultWonOption[8];
    char betResultOPId[8];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }    
};

struct getBetOptionDetail_input
{
    uint32_t betId;
    uint32_t betOption;
};
struct getBetOptionDetail_output
{
    uint8_t bettor[32*1024];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct getActiveBet_output
{
    uint32_t count;
    uint32_t betId[1024];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct getActiveBetByCreator_input
{
    uint8_t creator[32];
};
struct getActiveBetByCreator_output
{
    uint32_t count;
    uint32_t betId[1024];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    } 
};

struct publishResult_input
{
    uint32_t betId;
    uint32_t winOption;
};

#pragma pack(push, 1)
struct cancelBet_input
{
    uint32_t betId;
};
#pragma pack(pop)

void quotteryIssueBet(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset);
void quotteryPrintBetInfo(const char* nodeIp, const int nodePort, int betId);
void quotteryPrintBasicInfo(const char* nodeIp, const int nodePort);
void quotteryJoinBet(const char* nodeIp, int nodePort, const char* seed, uint32_t betId, int numberOfBetSlot, uint64_t amountPerSlot, uint8_t option, uint32_t scheduledTickOffset);
void quotteryPrintBetOptionDetail(const char* nodeIp, const int nodePort, uint32_t betId, uint32_t betOption);
void quotteryPrintActiveBet(const char* nodeIp, const int nodePort);
void quotteryPrintActiveBetByCreator(const char* nodeIp, const int nodePort, const char* identity);
void quotteryCancelBet(const char* nodeIp, const int nodePort, const char* seed, const uint32_t betId, const uint32_t scheduledTickOffset);
void quotteryPublishResult(const char* nodeIp, const int nodePort, const char* seed, const uint32_t betId, const uint32_t winOption, const uint32_t scheduledTickOffset);

// Get the basic information of quottery
void quotteryGetBasicInfo(const char* nodeIp, const int nodePort, qtryBasicInfo_output& result);

// Core function
void quotteryGetActiveBet(const char* nodeIp, const int nodePort, getActiveBet_output& result);

// Get detail information of a bet ID
void quotteryGetBetInfo(
    const char* nodeIp,
    const int nodePort,
    int betId,
    getBetInfo_output& result);

void unpackQuotteryDate(uint8_t& _year, uint8_t& _month, uint8_t& _day, uint8_t& _hour, uint8_t& _minute, uint8_t& _second, uint32_t data);
