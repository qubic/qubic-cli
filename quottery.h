void quotteryIssueBet(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset);
void quotteryPrintBetInfo(const char* nodeIp, const int nodePort, int betId);
void quotteryPrintBetFees(const char* nodeIp, const int nodePort);
void quotteryJoinBet(const char* nodeIp, int nodePort, const char* seed, uint32_t betId, int numberOfBetSlot, uint64_t amountPerSlot, uint8_t option, uint32_t scheduledTickOffset);

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