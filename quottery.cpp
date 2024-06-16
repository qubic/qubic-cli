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

constexpr int QUOTTERY_CONTRACT_ID = 2;
enum quotteryViewId{
    fee = 1,
    betInfo = 2,
    betDetail = 3,
    activeBet = 4,
    activeBetByCreator = 5
};
enum quotteryFuncId{
    issue = 1,
    join = 2,
    cancelBet = 3,
    publishResult = 4
};


void quotteryGetBasicInfo(const char* nodeIp, const int nodePort, qtryBasicInfo_output& result){
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = quotteryViewId::fee;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
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
            auto info = (qtryBasicInfo_output *)(data + ptr + sizeof(RequestResponseHeader));
            result = *info;
        }
        ptr+= header->size();
    }

}

void quotteryPrintBasicInfo(const char* nodeIp, const int nodePort){
    qtryBasicInfo_output result;
    memset(&result, 1, sizeof(qtryBasicInfo_output));
    quotteryGetBasicInfo(nodeIp, nodePort, result);
    LOG("Fee per slot per day: %llu qu\n", result.feePerSlotPerDay);
    LOG("Minimum amount of qus per bet slot: %llu qu\n", result.minBetSlotAmount);
    LOG("Game operator Fee: %.2f%%\n", result.gameOperatorFee/100.0);
    LOG("Shareholders fee: %.2f%%\n", result.shareholderFee/100.0);
    LOG("Burn fee: %.2f%%\n", result.burnFee/100.0);
    LOG("================\n");
    LOG("Number of issued bet: %lld\n", result.nIssuedBet);
    LOG("moneyFlow: %lld\n", result.moneyFlow);
    LOG("moneyFlow through issueBet: %lld\n", result.moneyFlowThroughIssueBet);
    LOG("moneyFlow through joinBet: %lld\n", result.moneyFlowThroughJoinBet);
    LOG("moneyFlow through finalizeBet: %lld\n", result.moneyFlowThroughFinalizeBet);
    LOG("================\n");
    LOG("earned amount for shareholders: %lld\n", result.earnedAmountForShareHolder);
    LOG("earned amount for winners: %lld\n", result.earnedAmountForBetWinner);
    LOG("distributed amount: %lld\n", result.distributedAmount);
    LOG("burned amount: %lld\n", result.burnedAmount);
    char buf[64] = {0};
    getIdentityFromPublicKey(result.gameOperator, buf, false);
    LOG("Game operator ID: %s\n", buf);
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
    auto qc = make_qc(nodeIp, nodePort);
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
    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
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
        promptStdin("Enter amount of qus per bet slot", buff, 16);
        packet.ibi.amountPerSlot = std::atoi(buff);
    }
    {
        promptStdin("Enter max number of bet slot per option", buff, 16);
        packet.ibi.maxBetSlotPerOption = std::atoi(buff);
    }

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    {
        qtryBasicInfo_output quotteryBasicInfo;
        quotteryGetBasicInfo(nodeIp, nodePort, quotteryBasicInfo);
        std::time_t now = time(0);
        std::tm *gmtm = gmtime(&now);
        uint8_t year = gmtm->tm_year % 100;
        uint8_t month = gmtm->tm_mon;
        uint8_t day = gmtm->tm_mday;
        uint8_t curDate[4] = {year, month, day, 0};
        uint64_t diffday = diffDate(curDate, packet.ibi.endDate) + 1;
        packet.transaction.amount = packet.ibi.maxBetSlotPerOption * packet.ibi.numberOfOption * quotteryBasicInfo.feePerSlotPerDay * diffday;
    }


    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = quotteryFuncId::issue;
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
    auto qc = make_qc(nodeIp, nodePort);
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
    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
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
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = quotteryFuncId::join;
    packet.transaction.inputSize = sizeof(QuotteryjoinBet_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(QuotteryjoinBet_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
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
void quotteryGetBetInfo(const char* nodeIp, const int nodePort, int betId, getBetInfo_output& result){
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        getBetInfo_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(getBetInfo_input);
    packet.rcf.inputType = quotteryViewId::betInfo;
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

}
void quotteryPrintBetInfo(const char* nodeIp, const int nodePort, int betId){

    getBetInfo_output result;
    memset(&result, 0, sizeof(getBetInfo_output));
    LOG("Getting betId #%d info...\n", betId);
    quotteryGetBetInfo(nodeIp, nodePort, betId, result);
    if (isArrayZero((uint8_t*)&result, sizeof(getBetInfo_output))){
        LOG("Failed to get\n");
        return;
    }
    if (result.betId == -1){
        LOG("BetId #%d doesn't exist\n", betId);
        return;
    }
    char buf[128] = {0};
    LOG("Bet Id: %u\n", result.betId); //    uint32_t betId;
    LOG("Number of options: %u\n", result.nOption); //    uint8_t nOption;      // options number
    getIdentityFromPublicKey(result.creator, buf, false);
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
        LOG("OpenDate: %02u-%02u-%02u 00:00:00\n", result.openDate[0], result.openDate[1], result.openDate[2]);
        LOG("CloseDate: %02u-%02u-%02u 23:59:59\n", result.closeDate[0], result.closeDate[1], result.closeDate[2]);
        LOG("EndDate: %02u-%02u-%02u 23:59:59\n", result.endDate[0], result.endDate[1], result.endDate[2]);
    }
    LOG("Oracle IDs\n");
    int nOP = 0;
    for (int i = 0; i < 8; i++){
        if (!isZeroPubkey(result.oracleProviderId+i*32)){
            nOP++;
            memset(buf, 0 , 128);
            getIdentityFromPublicKey(result.oracleProviderId+i*32, buf, false);
            uint32_t fee_u32 = result.oracleFees[i];
            double fee = (fee_u32)/100.0;
            LOG("%s\tFee: %.2f%%\n", buf, fee);
        }
    }
    LOG("Current votes result:\n");
    for (int i = 0; i < 8; i++)
    {
        if (result.betResultWonOption[i] != -1 && result.betResultOPId[i] != -1)
        {
            LOG("OP #%d: voted for option #%d\n", result.betResultOPId[i], result.betResultWonOption[i]);
        }
    }
}

//  getBetOptionDetail 3
void quotteryGetBetOptionDetail(const char* nodeIp, const int nodePort, uint32_t betId, uint32_t betOption, getBetOptionDetail_output& result){
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        getBetOptionDetail_input bo_inp;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(getBetOptionDetail_input);
    packet.rcf.inputType = quotteryViewId::betDetail;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
    packet.bo_inp.betId = betId;
    packet.bo_inp.betOption = betOption;
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
            auto oup = (getBetOptionDetail_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *oup;
        }
        ptr+= header->size();
    }

}
// showing which ID bet for an option
void quotteryPrintBetOptionDetail(const char* nodeIp, const int nodePort, uint32_t betId, uint32_t betOption){
    getBetOptionDetail_output result;
    memset(&result, 0, sizeof(getBetOptionDetail_output));
    quotteryGetBetOptionDetail(nodeIp, nodePort, betId, betOption, result);
    if (isArrayZero((uint8_t*)&result, sizeof(getBetOptionDetail_output))){
        LOG("Failed to get\n");
        return;
    }
    LOG("List of IDs bet option #%d on betID %d\n", betOption, betId);
    char buf[128] = {0};
    for (int i = 0; i < 1024; i++){
        if (!isZeroPubkey(result.bettor + i*32)){
            memset(buf, 0, 128);
            getIdentityFromPublicKey(result.bettor + i * 32, buf, false);
            LOG("%s\n", buf);
        }
    }
}
//getActiveBet 4
void quotteryGetActiveBet(const char* nodeIp, const int nodePort, getActiveBet_output& result){
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = quotteryViewId::activeBet;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
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
            auto oup = (getActiveBet_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *oup;
        }
        ptr+= header->size();
    }

}
// showing which ID bet for an option
void quotteryPrintActiveBet(const char* nodeIp, const int nodePort){
    getActiveBet_output result;
    memset(&result, 0, sizeof(getActiveBet_output));
    quotteryGetActiveBet(nodeIp, nodePort, result);
    LOG("List of active bet (%d):\n", result.count);
    for (int i = 0; i < result.count; i++){
        LOG("%u, ", result.betId[i]);
    }
    LOG("\n");
}
//getBetByCreator 5
//getActiveBet 4
void quotteryGetActiveBetByCreator(const char* nodeIp, const int nodePort, getActiveBetByCreator_output& result, const uint8_t* creator){
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        getActiveBetByCreator_input abi;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(getActiveBetByCreator_input);
    packet.rcf.inputType = quotteryViewId::activeBetByCreator;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
    memcpy(packet.abi.creator, creator, 32);
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
            auto oup = (getActiveBetByCreator_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *oup;
        }
        ptr+= header->size();
    }

}

void quotteryPrintActiveBetByCreator(const char* nodeIp, const int nodePort, const char* identity){
    uint8_t creatorPubkey[32] = {0};
    getPublicKeyFromIdentity(identity, creatorPubkey);
    getActiveBetByCreator_output result;
    memset(&result, 0, sizeof(getActiveBet_output));
    quotteryGetActiveBetByCreator(nodeIp, nodePort, result, creatorPubkey);
    LOG("List of active bet (%d):\n", result.count);
    for (int i = 0; i < result.count; i++){
        LOG("%u, ", result.betId[i]);
    }
    LOG("\n");
}

void quotteryCancelBet(const char* nodeIp, const int nodePort, const char* seed, const uint32_t betId, const uint32_t scheduledTickOffset){
    auto qc = make_qc(nodeIp, nodePort);
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
    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        cancelBet_input cbi;
        unsigned char signature[64];
    } packet;
    packet.cbi.betId = betId;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 0;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = quotteryFuncId::cancelBet;
    packet.transaction.inputSize = sizeof(cancelBet_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(cancelBet_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(cancelBet_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Cancel bet tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}
void quotteryPublishResult(const char* nodeIp, const int nodePort, const char* seed, const uint32_t betId, const uint32_t winOption, const uint32_t scheduledTickOffset){
    auto qc = make_qc(nodeIp, nodePort);
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
    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        publishResult_input pri;
        unsigned char signature[64];
    } packet;
    packet.pri.betId = betId;
    packet.pri.winOption = winOption;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 0;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = quotteryFuncId::publishResult;
    packet.transaction.inputSize = sizeof(publishResult_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(publishResult_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(publishResult_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Publishing result tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}
