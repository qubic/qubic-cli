#include <cstring>
#include <cstdio>
#include <ctime>
#include "stdint.h"
#include "quottery.h"
#include "prompt.h"
#include "structs.h"
#include "keyUtils.h"
#include "nodeUtils.h"
#include "K12AndKeyUtil.h"
#include "connection.h"
#include "logger.h"
#include "walletUtils.h"


void quotteryGetFees(const char* nodeIp, const int nodePort, QuotteryFees_output& result){
    auto qc = new QubicConnection(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = 1;
    packet.rcf.contractIndex = 2;
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            auto fees = (QuotteryFees_output *)(data + ptr + sizeof(RequestResponseHeader));
            result = *fees;
        }
        ptr+= header->size();
    }
    delete qc;
}

void quotteryPrintBetFees(const char* nodeIp, const int nodePort){
    QuotteryFees_output result;
    memset(&result, 1, sizeof(QuotteryFees_output));
    quotteryGetFees(nodeIp, nodePort, result);
    LOG("feePerSlotPerDay: %llu\n", result.feePerSlotPerDay);
    LOG("gameOperatorFee: %llu\n", result.gameOperatorFee);
    LOG("feePerSlotPerDay: %llu\n", result.shareholderFee);
    LOG("feePerSlotPerDay: %llu\n", result.minBetSlotAmount);
}

static int accumulatedDay(int month)
{
    int res = 0;
    switch (month)
    {
        case 1: res = 0; break;
        case 2: res = 31; break;
        case 3: res = 59; break;
        case 4: res = 90; break;
        case 5: res = 120; break;
        case 6: res = 151; break;
        case 7: res = 181; break;
        case 8: res = 212; break;
        case 9: res = 243; break;
        case 10:res = 273; break;
        case 11:res = 304; break;
        case 12:res = 334; break;;
    }
    return res;
}

// return diff in number of day, A must be smaller than or equal B to have valid value
static uint64_t diffDate(uint8_t A[4], uint8_t B[4]) {
    int baseYear = A[0];
    uint64_t dayA = accumulatedDay(A[1]) + A[2];
    uint64_t dayB = (B[0]-baseYear)*365ULL + accumulatedDay(B[1]) + B[2];

    // handling leap-year: only store last 2 digits of year here, don't care about mod 100 & mod 400 case
    for (int i = baseYear; i < B[0]; i++) {
        if (i % 4 == 0) {
            dayB++;
        }
    }
    if (B[0] % 4 == 0 && B[1] > 2) dayB++;
    return dayB-dayA;
}

void quotteryIssueBet(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset){
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subseed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char publicIdentity[128] = {0};
    char txHash[128] = {0};
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    const bool isLowerCase = false;
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    ((uint64_t*)destPublicKey)[0] = 2;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        QuotteryissueBet_input ibi;
        unsigned char signature[64];
    } packet;
    memset(&packet.ibi, 0, sizeof(QuotteryissueBet_input));

    char buff[128] = {0};
    promptStdin("Enter bet description (32 chars)", buff, 32);
    memcpy(packet.ibi.betDesc, buff, 32);
    promptStdin("Enter number of options (valid [2-8])", buff, 1);
    packet.ibi.numberOfOption = buff[0] - 48;
    for (int i = 0; i < packet.ibi.numberOfOption; i++){
        char buff2[128] = {0};
        sprintf(buff2, "Enter option #%d description (32 chars)", i);
        promptStdin(buff2, buff, 32);
        memcpy(packet.ibi.optionDesc + i*32, buff, 32);
    }
    promptStdin("Enter number of oracle provider (valid [1-8])", buff, 1);
    int numberOP = buff[0] - 48;
    for (int i = 0; i < numberOP; i++){
        char buff2[128] = {0};
        uint8_t buf3[32] = {0};
        sprintf(buff2, "Enter oracle provider #%d ID (60 chars)", i);
        promptStdin(buff2, buff, 60);
        getPublicKeyFromIdentity(buff, buf3);
        memcpy(packet.ibi.oracleProviderId + i * 32, buf3, 32);
    }
    for (int i = 0; i < numberOP; i++){
        char buff2[128] = {0};
        sprintf(buff2, "Enter fee for oracle provider #%d ID [4 digits number, format ABCD (meaning AB.CD%%)]", i);
        promptStdin(buff2, buff, 4);
        uint32_t op_fee = std::atoi(buff);
        packet.ibi.oracleFees[i] = op_fee;
    }
    {
        promptStdin("Enter bet close date (stop receiving bet date) (Format: YY-MM-DD)", buff, 8);
        uint8_t year = (buff[0]-48)*10 + (buff[1]-48);
        uint8_t month = (buff[3]-48)*10 + (buff[4]-48);
        uint8_t day = (buff[6]-48)*10 + (buff[7]-48);
        packet.ibi.closeDate[0] = year;
        packet.ibi.closeDate[1] = month;
        packet.ibi.closeDate[2] = day;
        packet.ibi.closeDate[3] = 0;
    }
    {
        promptStdin("Enter bet end date (finalize bet date) (Format: YY-MM-DD)", buff, 8);
        uint8_t year = (buff[0]-48)*10 + (buff[1]-48);
        uint8_t month = (buff[3]-48)*10 + (buff[4]-48);
        uint8_t day = (buff[6]-48)*10 + (buff[7]-48);
        packet.ibi.endDate[0] = year;
        packet.ibi.endDate[1] = month;
        packet.ibi.endDate[2] = day;
        packet.ibi.endDate[3] = 0;
    }
    {
        promptStdin("Enter amount per bet slot", buff, 16);
        packet.ibi.amountPerSlot = std::atoi(buff);
    }
    {
        promptStdin("Enter max bet slot per option", buff, 16);
        packet.ibi.maxBetSlotPerOption = std::atoi(buff);
    }

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    {
        QuotteryFees_output quotteryFee;
        quotteryGetFees(nodeIp, nodePort, quotteryFee);
        std::time_t now = time(0);
        std::tm *gmtm = gmtime(&now);
        uint8_t year = gmtm->tm_year % 100;
        uint8_t month = gmtm->tm_mon;
        uint8_t day = gmtm->tm_mday;
        uint8_t curDate[4] = {year, month, day, 0};
        uint64_t diffday = diffDate(curDate, packet.ibi.endDate) + 1;
        packet.transaction.amount = packet.ibi.maxBetSlotPerOption * packet.ibi.numberOfOption * quotteryFee.feePerSlotPerDay * diffday;
    }


    uint32_t currentTick = getTickNumberFromNode(nodeIp, nodePort);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = 1;
    packet.transaction.inputSize = sizeof(QuotteryissueBet_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(QuotteryissueBet_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    auto qc = new QubicConnection(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(QuotteryissueBet_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Bet creation has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void quotteryJoinBet(const char* nodeIp, int nodePort, const char* seed, uint32_t betId, int numberOfBetSlot, uint64_t amountPerSlot, uint8_t option, uint32_t scheduledTickOffset){
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subseed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char publicIdentity[128] = {0};
    char txHash[128] = {0};
    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    const bool isLowerCase = false;
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    ((uint64_t*)destPublicKey)[0] = 2;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        QuotteryjoinBet_input jbi;
        unsigned char signature[64];
    } packet;
    memset(&packet.jbi, 0, sizeof(QuotteryjoinBet_input));
    packet.jbi.betId = betId;
    packet.jbi.numberOfSlot = numberOfBetSlot;
    packet.jbi.option = option;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = amountPerSlot*numberOfBetSlot;
    uint32_t currentTick = getTickNumberFromNode(nodeIp, nodePort);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = 2;
    packet.transaction.inputSize = sizeof(QuotteryjoinBet_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(QuotteryjoinBet_input),
                   digest,
                   32);
    printf("size of joinBet_input %d\n", sizeof(QuotteryjoinBet_input));
    printf("size of transaction %d\n", sizeof(Transaction));
    printf("size of packet %d\n", sizeof(packet));
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    auto qc = new QubicConnection(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(QuotteryjoinBet_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Joining bet tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

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
void quotteryGetBetInfo(const char* nodeIp, const int nodePort, int betId, getBetInfo_output& result){
    auto qc = new QubicConnection(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        getBetInfo_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 4;
    packet.rcf.inputType = 2;
    packet.rcf.contractIndex = 2;
    packet.input.betId = betId;
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            auto fees = (getBetInfo_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *fees;
        }
        ptr+= header->size();
    }
    delete qc;
}
static bool isZeroPubkey(uint8_t* pubkey){
    for (int i = 0; i < 32; i++){
        if (pubkey[i] != 0){
            return false;
        }
    }
    return true;
}
void quotteryPrintBetInfo(const char* nodeIp, const int nodePort, int betId){

    getBetInfo_output result;
    memset(&result, 1, sizeof(getBetInfo_output));
    quotteryGetBetInfo(nodeIp, nodePort, betId, result);
    char buf[128] = {0};
    LOG("Bet Id: %u\n", result.betId); //    uint32_t betId;
    LOG("Number of options: %u\n", result.nOption); //    uint8_t nOption;      // options number
    byteToHex(result.creator, buf, 32);
    LOG("Creator: %s\n", buf);
    {
        memset(buf, 0 , 128);
        memcpy(buf, result.betDesc, 32);
        LOG("Bet descriptions: %s\n", buf);
    }
    for (int i = 0; i < result.nOption; i++){
        memset(buf, 0 , 128);
        memcpy(buf, result.optionDesc + i * 32, 32);
        LOG("Option #%d: %s\n", i, buf);
    }
    {
        LOG("Current state:\n");
        for (int i = 0; i < result.nOption; i++){
            LOG("Option #%d: %d | ", i, result.currentBetState[i]);
        }
        LOG("\n");
    }
    {
        LOG("Minimum bet amount: %llu\n", result.minBetAmount);
        LOG("Maximum slot per option: %llu\n", result.maxBetSlotPerOption);
        LOG("OpenDate: %u-%u-%u 00:00:00\n", result.openDate[0], result.openDate[1], result.openDate[2]);
        LOG("CloseDate: %u-%u-%u 23:59:59\n", result.closeDate[0], result.closeDate[1], result.closeDate[2]);
        LOG("EndDate: %u-%u-%u 23:59:59\n", result.endDate[0], result.endDate[1], result.endDate[2]);
    }
    LOG("Oracle IDs\n");
    for (int i = 0; i < 8; i++){
        if (!isZeroPubkey(result.oracleProviderId+i*32)){
            memset(buf, 0 , 128);
            byteToHex(result.oracleProviderId+i*32, buf, 32);
            uint32_t fee_u32 = result.oracleFees[i];
            double fee = (fee_u32)/100.0;
            LOG("%s\tFee: %.2f%%\n", buf, fee);
        }
    }
}