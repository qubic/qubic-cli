#pragma once
//SC structs
struct QuotteryjoinBet_input {
    uint32_t betId;
    int numberOfSlot;
    uint32_t option;
    uint32_t _placeHolder;
};
struct QuotteryissueBet_input {
    uint8_t betDesc[32];
    uint8_t optionDesc[32*8];
    uint8_t oracleProviderId[32*8];
    uint32_t oracleFees[8];
    uint8_t closeDate[4];
    uint8_t endDate[4];
    uint64_t amountPerSlot;
    uint32_t maxBetSlotPerOption;
    uint32_t numberOfOption;
};
struct QuotteryFees_output
{
    uint64_t feePerSlotPerDay; // Amount of qus
    uint64_t gameOperatorFee; // Amount of qus
    uint64_t shareholderFee; // 4 digit number ABCD means AB.CD% | 1234 is 12.34%
    uint64_t minBetSlotAmount; // 4 digit number ABCD means AB.CD% | 1234 is 12.34%
};


struct getBetInfo_input {
    uint32_t betId;
};

struct getBetInfo_output {
    // meta data info
    uint32_t betId;
    uint32_t nOption;      // options number
    uint8_t creator[32];
    uint8_t betDesc[32];      // 32 bytes
    uint8_t optionDesc[8*32];  // 8x(32)=256bytes
    uint8_t oracleProviderId[8*32]; // 256x8=2048bytes
    uint32_t oracleFees[8];   // 4x8 = 32 bytes

    uint8_t openDate[4];     // creation date, start to receive bet
    uint8_t closeDate[4];    // stop receiving bet date
    uint8_t endDate[4];       // result date
    // Amounts and numbers
    uint64_t minBetAmount;
    uint32_t maxBetSlotPerOption;
    uint32_t currentBetState[8]; // how many bet slots have been filled on each option
};

struct getBetOptionDetail_input {
    uint32_t betId;
    uint32_t betOption;
};
struct getBetOptionDetail_output {
    uint8_t bettor[32*1024];
};
struct getActiveBet_output{
    uint32_t count;
    uint32_t betId[1024];
};

struct getActiveBetByCreator_input{
    uint8_t creator[32];
};

struct getActiveBetByCreator_output{
    uint32_t count;
    uint32_t betId[1024];
};
void quotteryIssueBet(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset);
void quotteryPrintBetInfo(const char* nodeIp, const int nodePort, int betId);
void quotteryPrintBetFees(const char* nodeIp, const int nodePort);
void quotteryJoinBet(const char* nodeIp, int nodePort, const char* seed, uint32_t betId, int numberOfBetSlot, uint64_t amountPerSlot, uint8_t option, uint32_t scheduledTickOffset);
void quotteryPrintBetOptionDetail(const char* nodeIp, const int nodePort, uint32_t betId, uint32_t betOption);
void quotteryPrintActiveBet(const char* nodeIp, const int nodePort);
void quotteryPrintActiveBetByCreator(const char* nodeIp, const int nodePort, const char* identity);