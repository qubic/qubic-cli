#include <cinttypes>
#include <cstring>
#include <cstdio>
#include <ctime>

#include "stdint.h"
#include "quottery.h"
#include "prompt.h"
#include "structs.h"
#include "key_utils.h"
#include "node_utils.h"
#include "k12_and_key_utils.h"
#include "connection.h"
#include "logger.h"
#include "wallet_utils.h"
constexpr int QUOTTERY_CONTRACT_ID = 2;
/**
 * @return pack DateAndTime data from year, month, day, hour, minute, second, millisec, microsec to a uint64_t
 * Bit layout: year(18) | month(4) | day(5) | hour(5) | minute(6) | second(6) | millisec(10) | microsec(10)
 */
static void packDateTime(uint32_t _year, uint32_t _month, uint32_t _day, uint32_t _hour, uint32_t _minute, uint32_t _second, uint32_t _millisec, uint32_t _microsec, uint64_t& res)
{
    res = ((uint64_t)_year << 46) | ((uint64_t)_month << 42) | ((uint64_t)_day << 37) | ((uint64_t)_hour << 32)
        | ((uint64_t)_minute << 26) | ((uint64_t)_second << 20) | ((uint64_t)_millisec << 10) | (uint64_t)_microsec;
}

#define DATETIME_GET_YEAR(data) ((data >> 46))
#define DATETIME_GET_MONTH(data) ((data >> 42) & 0b1111)
#define DATETIME_GET_DAY(data) ((data >> 37) & 0b11111)
#define DATETIME_GET_HOUR(data) ((data >> 32) & 0b11111)
#define DATETIME_GET_MINUTE(data) ((data >> 26) & 0b111111)
#define DATETIME_GET_SECOND(data) ((data >> 20) & 0b111111)
#define DATETIME_GET_MILLISEC(data) ((data >> 10) & 0b1111111111)
#define DATETIME_GET_MICROSEC(data) ((data) & 0b1111111111)

/**
* @return unpack DateAndTime from uint64 to year, month, day, hour, minute, second, millisec, microsec
*/
void unpackDateTime(uint32_t& _year, uint8_t& _month, uint8_t& _day, uint8_t& _hour, uint8_t& _minute, uint8_t& _second, uint16_t& _millisec, uint16_t& _microsec, uint64_t data)
{
    _year = DATETIME_GET_YEAR(data); // 18 bits
    _month = DATETIME_GET_MONTH(data); // 4 bits
    _day = DATETIME_GET_DAY(data); // 5 bits
    _hour = DATETIME_GET_HOUR(data); // 5 bits
    _minute = DATETIME_GET_MINUTE(data); // 6 bits
    _second = DATETIME_GET_SECOND(data); // 6 bits
    _millisec = DATETIME_GET_MILLISEC(data); // 10 bits
    _microsec = DATETIME_GET_MICROSEC(data); // 10 bits
}

// QTRY PROCEDURES
#define QTRY_CREATE_EVENT 1
#define QTRY_ADD_ASK_ORDER    2
#define QTRY_REMOVE_ASK_ORDER 3
#define QTRY_ADD_BID_ORDER    4
#define QTRY_REMOVE_BID_ORDER 5
#define QTRY_PUBLISH_RESULT 6
#define QTRY_TRY_FINALIZE_EVENT 7
#define QTRY_DISPUTE 8
#define QTRY_RESOLVE_DISPUTE 9
#define QTRY_USER_CLAIM_REWARD 10
#define QTRY_GO_FORCE_CLAIM_REWARD 11
#define QTRY_TRANSFER_QUSD 12
#define QTRY_TRANSFER_SHARE_MANAGEMENT_RIGHTS 13
#define QTRY_CLEAN_MEMORY 14
#define QTRY_TRANSFER_QTRYGOV 15
#define QTRY_UPDATE_FEE_DISCOUNT_LIST 20
#define QTRY_PROPOSAL_VOTE 100
// QTRY FUNCTIONS
#define QTRY_GET_BASIC 1
#define QTRY_GET_EVENT 2
#define QTRY_GET_ORDERS 3
#define QTRY_GET_ACTIVE_EVENTS 4
#define QTRY_GET_EVENT_BATCH 5
#define QUOTTERY_GET_USER_POSITION 6
#define QTRY_GET_APPROVED_AMOUNT 7
#define QTRY_GET_TOP_PROPOSALS 8

#define QUOTTERY_EO_GET_OPTION(eo)  ((eo) >> 63)
#define QUOTTERY_EO_GET_EVENTID(eo) ((eo) & 0x3FFFFFFFFFFFFFFFULL)

static int64_t getBalanceNumber(QCPtr& qc, const uint8_t* publicKey) {
    RespondedEntity result;
    memset(&result, 0, sizeof(RespondedEntity));
    std::vector<uint8_t> buffer;
    buffer.resize(0);
    uint8_t tmp[1024] = { 0 };
    struct {
        RequestResponseHeader header;
        RequestedEntity req;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_ENTITY);
    memcpy(packet.req.publicKey, publicKey, 32);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    result = qc->receivePacketWithHeaderAs<RespondedEntity>();
    return result.entity.incomingAmount - result.entity.outgoingAmount;
}

void quotteryGetBasicInfo(QCPtr& qc, qtryBasicInfo_output& result)
{
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = QTRY_GET_BASIC;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<qtryBasicInfo_output>();
    }
    catch (std::logic_error)
    {
        memset(&result, 0, sizeof(qtryBasicInfo_output));
    }
}

void quotteryPrintBasicInfo(const char* nodeIp, const int nodePort)
{
    qtryBasicInfo_output result;
    memset(&result, 1, sizeof(qtryBasicInfo_output));
    auto qc = make_qc(nodeIp, nodePort);
    quotteryGetBasicInfo(qc, result);
    LOG("Operation Fee: %.2f%%\n", result.operationFee / 10.0);
    LOG("Shareholders fee: %.2f%%\n", result.shareholderFee / 10.0);
    LOG("Burn fee: %.2f%%\n", result.burnFee / 10.0);
    LOG("================\n");
    LOG("Number of issued events: %" PRIu64 "\n", result.nIssuedEvent);
    LOG("Shareholders revenue: %" PRIu64 "\n", result.shareholdersRevenue);
    LOG("Operation revenue: %" PRIu64 "\n", result.operationRevenue);
    LOG("Burned amount: %" PRIu64 "\n", result.burnedAmount);
    LOG("feePerDay: %" PRIu64 "\n", result.feePerDay);
    LOG("antiSpamAmount: %" PRIu64 "\n", result.antiSpamAmount);
    LOG("depositAmountForDispute: %" PRIu64 "\n", result.depositAmountForDispute);
    char buf[64] = { 0 };
    getIdentityFromPublicKey(result.gameOperator, buf, false);
    LOG("Game operator ID: %s\n", buf);
}

void quotteryGetActiveEvents(const char* nodeIp, int nodePort, getActiveEvent_output& result)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct
    {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet{};

    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = QTRY_GET_ACTIVE_EVENTS;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;

    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<getActiveEvent_output>();
    }
    catch (std::logic_error)
    {
        memset(&result, 0, sizeof(getActiveEvent_output));
    }
}

void quotteryPrintActiveEvents(const char* nodeIp, int nodePort)
{
    getActiveEvent_output result{};
    quotteryGetActiveEvents(nodeIp, nodePort, result);

    if (isArrayZero(reinterpret_cast<uint8_t*>(&result), sizeof(result)))
    {
        LOG("Failed to get recent active events\n");
        return;
    }

    LOG("Recent active event IDs:\n");
    bool hasAny = false;
    for (size_t i = 0; i < QUOTTERY_MAX_CONCURRENT_EVENT; ++i)
    {
        if (result.recentActiveEvent[i] == uint64_t(-1))
            continue;

        LOG("%" PRIu64 "\n", result.recentActiveEvent[i]);
        hasAny = true;
    }

    if (!hasAny)
    {
        LOG("(none)\n");
    }
}

#define QTRY_GET_YEAR(data) ((data >> 26)+24)
#define QTRY_GET_MONTH(data) ((data >> 22) & 0b1111)
#define QTRY_GET_DAY(data) ((data >> 17) & 0b11111)
#define QTRY_GET_HOUR(data) ((data >> 12) & 0b11111)
#define QTRY_GET_MINUTE(data) ((data >> 6) & 0b111111)
#define QTRY_GET_SECOND(data) ((data) & 0b111111)

/**
* @return unpack qtry datetime from uin32 to year, month, day, hour, minute, secon
*/
void unpackQuotteryDate(uint8_t& _year, uint8_t& _month, uint8_t& _day, uint8_t& _hour, uint8_t& _minute, uint8_t& _second, uint32_t data)
{
    _year = QTRY_GET_YEAR(data); // 6 bits
    _month = QTRY_GET_MONTH(data); //4bits
    _day = QTRY_GET_DAY(data); //5bits
    _hour = QTRY_GET_HOUR(data); //5bits
    _minute = QTRY_GET_MINUTE(data); //6bits
    _second = QTRY_GET_SECOND(data); //6bits
}


struct QuotteryCreateEvent_input
{
    uint64_t eid;
    uint64_t openDate; // submitted date
    uint64_t endDate; // stop receiving result from OPs
    uint8_t desc[128];
    uint8_t option0Desc[64];
    uint8_t option1Desc[64];
};

void quotteryCreateEvent(const char* nodeIp, int nodePort, const char* seed,
    const std::string eventDesc,
    const std::string opt0Desc,
    const std::string opt1Desc,
    const std::string endDate,
    uint16_t tagId,
    uint32_t scheduledTickOffset)
{
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subseed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
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
        QuotteryCreateEvent_input cei;
        unsigned char signature[64];
    } packet;
    memset(&packet.cei, 0, sizeof(QuotteryCreateEvent_input));

    // Copy desc text, but cap at 30 bytes to reserve last 2 bytes for tagId
    memcpy(packet.cei.desc, eventDesc.c_str(), std::min(int(eventDesc.size()), 128));
    // Pack tagId as uint16 LE into desc[30:32]
    packet.cei.desc[126] = (uint8_t)(tagId & 0xFF);
    packet.cei.desc[127] = (uint8_t)((tagId >> 8) & 0xFF);
    memcpy(packet.cei.option0Desc, opt0Desc.c_str(), std::min(int(opt0Desc.size()), 64));
    memcpy(packet.cei.option1Desc, opt1Desc.c_str(), std::min(int(opt1Desc.size()), 64));

    {
        auto buff = endDate.data();
        if (strlen(buff) != 19 || buff[4] != '-' || buff[7] != '-' || buff[10] != ' ' || buff[13] != ':' ||
            buff[16] != ':' ||
            !isdigit(buff[0]) || !isdigit(buff[1]) || !isdigit(buff[2]) || !isdigit(buff[3]) ||
            !isdigit(buff[5]) || !isdigit(buff[6]) || !isdigit(buff[8]) || !isdigit(buff[9]) ||
            !isdigit(buff[11]) || !isdigit(buff[12]) || !isdigit(buff[14]) || !isdigit(buff[15]) ||
            !isdigit(buff[17]) || !isdigit(buff[18])) {
            LOG("Error: Invalid date-time format. Please follow the format: YYYY-MM-DD hh:mm:ss\n");
            exit(EXIT_FAILURE);
        }

        uint32_t year = (buff[0] - 48) * 1000 + (buff[1] - 48) * 100 + (buff[2] - 48) * 10 + (buff[3] - 48);
        uint8_t month = (buff[5] - 48) * 10 + (buff[6] - 48);
        uint8_t day = (buff[8] - 48) * 10 + (buff[9] - 48);
        uint8_t hour = (buff[11] - 48) * 10 + (buff[12] - 48);
        uint8_t minute = (buff[14] - 48) * 10 + (buff[15] - 48);
        uint8_t sec = (buff[17] - 48) * 10 + (buff[18] - 48);
        packDateTime(year, month, day, hour, minute, sec, 0, 0, packet.cei.endDate);
    }

    // Note: eid and openDate will be set by the SC
    packet.cei.eid = 0;
    packet.cei.openDate = 0;

    LOG("Crafting transaction...\n");
    LOG("Tag ID: %u\n", tagId);
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    auto qc = make_qc(nodeIp, nodePort);
    LOG("Established connection...\n");
    {
        qtryBasicInfo_output basic{};
        quotteryGetBasicInfo(qc, basic);
        packet.transaction.amount = 0;
    }

    uint32_t currentTick = getTickNumberFromNode(qc);
    LOG("Getting tick info, latest tick is: %u\n", currentTick);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QTRY_CREATE_EVENT;
    packet.transaction.inputSize = sizeof(QuotteryCreateEvent_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(QuotteryCreateEvent_input),
        digest,
        32);
    LOG("Signing tx packet...\n");
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    LOG("Sending data...\n");
    qc->sendData((uint8_t*)&packet, packet.header.size());
    LOG("Sent data...\n");
    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(QuotteryCreateEvent_input) + SIGNATURE_SIZE,
        digest,
        32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Event creation has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void _quotteryGetEventInfo(QCPtr& qc, uint64_t eventId, getEventInfo_output& result) {
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        getEventInfo_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(getEventInfo_input);
    packet.rcf.inputType = QTRY_GET_EVENT;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
    packet.input.eventId = eventId;
    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<getEventInfo_output>();
    }
    catch (std::logic_error)
    {
        memset(&result, 0, sizeof(getEventInfo_output));
        result.resultByGO = -1;
    }
}

void quotteryGetEventInfo(const char* nodeIp, const int nodePort, uint64_t eventId, getEventInfo_output& result)
{
    auto qc = make_qc(nodeIp, nodePort);
    _quotteryGetEventInfo(qc, eventId, result);
}

static void quotteryPrintEventMetaData(const QtryEventInfo& result, uint64_t requestedEventId)
{
    if (result.eid != requestedEventId)
    {
        LOG("EventId #%" PRIu64 " doesn't exist\n", requestedEventId);
        return;
    }

    char buf[128] = { 0 };
    LOG("Event Id: %" PRIu64 "\n", result.eid);
    {
        memset(buf, 0, sizeof(buf));
        memcpy(buf, result.desc, sizeof(result.desc));
        LOG("Event description: %s\n", buf);
    }
    {
        memset(buf, 0, sizeof(buf));
        memcpy(buf, result.option0Desc, sizeof(result.option0Desc));
        LOG("Option 0: %s\n", buf);
    }
    {
        memset(buf, 0, sizeof(buf));
        memcpy(buf, result.option1Desc, sizeof(result.option1Desc));
        LOG("Option 1: %s\n", buf);
    }
    {
        uint32_t year;
        uint8_t month, day, hour, minute, second;
        uint16_t _millisec;
        uint16_t _microsec;
        unpackDateTime(year, month, day, hour, minute, second, _millisec, _microsec, result.openDate);
        LOG("Open date: %04u-%02u-%02u %02u:%02u:%02u\n", year, month, day, hour, minute, second);

        unpackDateTime(year, month, day, hour, minute, second, _millisec, _microsec, result.endDate);
        LOG("End date:   %04u-%02u-%02u %02u:%02u:%02u\n", year, month, day, hour, minute, second);
    }
}

static void quotteryPrintEventInfoRecord(const getEventInfo_output& result, uint64_t requestedEventId)
{
    if (result.qei.eid != requestedEventId)
    {
        LOG("EventId #%" PRIu64 " doesn't exist\n", requestedEventId);
        return;
    }
    quotteryPrintEventMetaData(result.qei, requestedEventId);

    LOG("Result by GO: %" PRId32 "\n", result.resultByGO);
    if (result.resultByGO != -1)
    {
        if (result.publishTickTime == 0xffffffffu) {
            LOG("This event is already finalized and waiting for cleanup\n");
        }
        else {
            LOG("Publish tick time: %" PRIu32 "\n", result.publishTickTime);
        }
    }

    if (!isZeroPubkey(result.disputerInfo.pubkey))
    {
        char disputerId[128] = { 0 };
        getIdentityFromPublicKey(result.disputerInfo.pubkey, disputerId, false);
        LOG("Disputer: %s\n", disputerId);
        LOG("Dispute amount: %" PRIu64 "\n", result.disputerInfo.amount);
        LOG("Computors vote 0: %" PRIu32 "\n", result.computorsVote0);
        LOG("Computors vote 1: %" PRIu32 "\n", result.computorsVote1);
    }
}

void quotteryPrintEventInfo(const char* nodeIp, const int nodePort, uint64_t eventId)
{
    getEventInfo_output result;
    memset(&result, 0, sizeof(getEventInfo_output));
    LOG("Getting eventId #%" PRIu64 " info...\n", eventId);
    quotteryGetEventInfo(nodeIp, nodePort, eventId, result);
    if (isArrayZero((uint8_t*)&result, sizeof(getEventInfo_output)))
    {
        LOG("Failed to get\n");
        return;
    }

    quotteryPrintEventInfoRecord(result, eventId);
}

void quotteryGetEventInfoBatch(const char* nodeIp, int nodePort, const uint64_t* eventIds, GetEventInfoBatch_output& result)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct
    {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetEventInfoBatch_input input;
    } packet{};

    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(GetEventInfoBatch_input);
    packet.rcf.inputType = QTRY_GET_EVENT_BATCH;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;

    memset(&packet.input, 0, sizeof(packet.input));
    for (size_t j = 0; j < 64; ++j)
    {
        packet.input.eventIds[j] = eventIds[j];
    }

    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<GetEventInfoBatch_output>();
    }
    catch (std::logic_error)
    {
        memset(&result, 0, sizeof(GetEventInfoBatch_output));
    }
}

void quotteryPrintEventInfoBatch(const char* nodeIp, int nodePort, const uint64_t* eventIds, size_t count)
{
    if (count == 0)
    {
        LOG("Error: no event ids provided\n");
        return;
    }

    uint64_t paddedEventIds[64] = {};
    for (size_t i = 0; i < count && i < 64; ++i)
    {
        paddedEventIds[i] = eventIds[i];
    }

    GetEventInfoBatch_output result{};
    memset(&result, 2, sizeof(GetEventInfoBatch_output));
    LOG("Getting %zu event(s) info in batch...\n", count);
    quotteryGetEventInfoBatch(nodeIp, nodePort, paddedEventIds, result);

    if (isArrayZero(reinterpret_cast<uint8_t*>(&result), sizeof(result)))
    {
        LOG("Failed to get batch event info\n");
        return;
    }

    for (size_t i = 0; i < count && i < 64; ++i)
    {
        LOG("\n================\n");
        LOG("Requested eventId: %" PRIu64 "\n", paddedEventIds[i]);
        quotteryPrintEventMetaData(result.aqei[i], paddedEventIds[i]);
    }
}

struct qtryOrderAction_input
{
    uint64_t eventId;
    uint64_t option;
    uint64_t amount;
    uint64_t price;
};
template <int functionNumber>
void qtryOrderAction(const char* nodeIp, int nodePort,
    const char* seed,
    uint64_t eventId, uint64_t option, uint64_t amount, int64_t price,
    uint64_t antiSpamAmount,
    uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        qtryOrderAction_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    qtryBasicInfo_output qbi{};
    quotteryGetBasicInfo(qc, qbi);
    antiSpamAmount = qbi.antiSpamAmount;
    packet.transaction.amount = (int64_t)antiSpamAmount;

    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = functionNumber;
    packet.transaction.inputSize = sizeof(qtryOrderAction_input);

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY order action - functionNumber: %d\n", functionNumber);
    LOG("eventId: %" PRIu64 "\n", eventId);
    LOG("option: %" PRIu64 "\n", option);
    LOG("amount: %" PRIu64 "\n", amount);
    LOG("price: %" PRId64 "\n", price);
    LOG("antiSpamAmount: %" PRIu64 "\n", antiSpamAmount);
    LOG("\n-------------------------------------\n\n");

    packet.input.eventId = eventId;
    packet.input.option = option;
    packet.input.amount = amount;
    packet.input.price = price;

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryOrderAction_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryOrderAction_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryOrderAction_input) + SIGNATURE_SIZE,
        digest,
        32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qtryAddToAskOrder(const char* nodeIp, int nodePort, const char* seed,
    uint64_t eventId, uint64_t option, uint64_t amount, int64_t price,
    uint64_t antiSpamAmount, uint32_t scheduledTickOffset)
{
    qtryOrderAction<QTRY_ADD_ASK_ORDER>(nodeIp, nodePort, seed, eventId, option, amount, price, antiSpamAmount, scheduledTickOffset);
}

void qtryAddToBidOrder(const char* nodeIp, int nodePort, const char* seed,
    uint64_t eventId, uint64_t option, uint64_t amount, int64_t price,
    uint64_t antiSpamAmount, uint32_t scheduledTickOffset)
{
    qtryOrderAction<QTRY_ADD_BID_ORDER>(nodeIp, nodePort, seed, eventId, option, amount, price, antiSpamAmount, scheduledTickOffset);
}

void qtryRemoveAskOrder(const char* nodeIp, int nodePort, const char* seed,
    uint64_t eventId, uint64_t option, uint64_t amount, int64_t price,
    uint64_t antiSpamAmount, uint32_t scheduledTickOffset)
{
    qtryOrderAction<QTRY_REMOVE_ASK_ORDER>(nodeIp, nodePort, seed, eventId, option, amount, price, antiSpamAmount, scheduledTickOffset);
}

void qtryRemoveBidOrder(const char* nodeIp, int nodePort, const char* seed,
    uint64_t eventId, uint64_t option, uint64_t amount, int64_t price,
    uint64_t antiSpamAmount, uint32_t scheduledTickOffset)
{
    qtryOrderAction<QTRY_REMOVE_BID_ORDER>(nodeIp, nodePort, seed, eventId, option, amount, price, antiSpamAmount, scheduledTickOffset);
}

void qtryGetOrders(const char* nodeIp, int nodePort,
    uint64_t eventId, uint64_t option, uint64_t isBid, uint64_t offset,
    qtryGetOrders_output& result)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        qtryGetOrders_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(qtryGetOrders_input);
    packet.rcf.inputType = QTRY_GET_ORDERS;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
    packet.input.eventId = eventId;
    packet.input.option = option;
    packet.input.isBid = isBid;
    packet.input.offset = offset;
    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<qtryGetOrders_output>();
    }
    catch (std::logic_error)
    {
        memset(&result, 0, sizeof(qtryGetOrders_output));
    }
}

void quotteryPrintOrders(const char* nodeIp, int nodePort,
    uint64_t eventId, uint64_t option, uint64_t isBid, uint64_t offset)
{
    qtryGetOrders_output result;
    memset(&result, 0, sizeof(qtryGetOrders_output));
    qtryGetOrders(nodeIp, nodePort, eventId, option, isBid, offset, result);

    int N = sizeof(result.orders) / sizeof(result.orders[0]);
    LOG("%s orders for eventId %" PRIu64 " option %" PRIu64 " (offset %" PRIu64 "):\n",
        isBid ? "Bid" : "Ask", eventId, option, offset);
    LOG("Entity\t\t\t\t\t\t\t\tPrice\tAmount\n");
    for (int i = 0; i < N; i++)
    {
        if (isZeroPubkey(result.orders[i].qo.entity))
        {
            break;
        }
        char iden[128] = { 0 };
        getIdentityFromPublicKey(result.orders[i].qo.entity, iden, false);
        LOG("%s\t%" PRId64 "\t%" PRIu64 "\n", iden, result.orders[i].price, result.orders[i].qo.amount);
    }
}

struct getUserPosition_input
{
    uint8_t uid[32];
};

void quotteryGetUserPosition(const char* nodeIp, int nodePort, const char* identity, getUserPosition_output& result)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        getUserPosition_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(getUserPosition_input);
    packet.rcf.inputType = QUOTTERY_GET_USER_POSITION;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
    memset(&packet.input, 0, sizeof(getUserPosition_input));
    getPublicKeyFromIdentity(identity, packet.input.uid);
    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        result = qc->receivePacketWithHeaderAs<getUserPosition_output>();
    }
    catch (std::logic_error)
    {
        memset(&result, 0, sizeof(getUserPosition_output));
    }
}

void quotteryPrintUserPosition(const char* nodeIp, int nodePort, const char* identity)
{
    getUserPosition_output result;
    memset(&result, 0, sizeof(getUserPosition_output));
    quotteryGetUserPosition(nodeIp, nodePort, identity, result);

    LOG("Positions for %s (count: %" PRId64 "):\n", identity, result.count);
    LOG("EventId\tOption\tAmount\n");
    for (int64_t i = 0; i < result.count; i++)
    {
        uint64_t eventId = QUOTTERY_EO_GET_EVENTID(result.p[i].eo);
        uint64_t option = QUOTTERY_EO_GET_OPTION(result.p[i].eo);
        LOG("%" PRIu64 "\t%" PRIu64 "\t%" PRId64 "\n", eventId, option, result.p[i].amount);
    }
}

struct qtryPublishResult_input
{
    uint64_t eventId;
    uint64_t option;
};

struct qtryTryFinalizeEvent_input
{
    uint64_t eventId;
};

static bool isCurrentUtcAfterPackedDateTime(uint64_t packedDateTime)
{
    uint32_t endYear;
    uint8_t endMonth, endDay, endHour, endMinute, endSecond;
    uint16_t endMillisec, endMicrosec;
    unpackDateTime(endYear, endMonth, endDay, endHour, endMinute, endSecond, endMillisec, endMicrosec, packedDateTime);

    std::time_t nowTs = std::time(nullptr);
    std::tm nowUtc{};
#if defined(_WIN32)
    gmtime_s(&nowUtc, &nowTs);
#else
    gmtime_r(&nowTs, &nowUtc);
#endif

    const uint32_t nowYear = static_cast<uint32_t>(nowUtc.tm_year + 1900);
    const uint8_t nowMonth = static_cast<uint8_t>(nowUtc.tm_mon + 1);
    const uint8_t nowDay = static_cast<uint8_t>(nowUtc.tm_mday);
    const uint8_t nowHour = static_cast<uint8_t>(nowUtc.tm_hour);
    const uint8_t nowMinute = static_cast<uint8_t>(nowUtc.tm_min);
    const uint8_t nowSecond = static_cast<uint8_t>(nowUtc.tm_sec);

    uint64_t nowPacked = 0;
    packDateTime(nowYear, nowMonth, nowDay, nowHour, nowMinute, nowSecond, 0, 0, nowPacked);

    return nowPacked >= packedDateTime;
}

void qtryPublishResult(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t eventId, uint64_t result)
{
    if (result != 0 && result != 1)
    {
        LOG("Error: result can only be 0 or 1\n");
        return;
    }

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };
    char goIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    qtryBasicInfo_output basic{};
    quotteryGetBasicInfo(qc, basic);

    if (isArrayZero((uint8_t*)&basic, sizeof(basic)))
    {
        LOG("Error: failed to get Quottery basic info\n");
        return;
    }

    if (memcmp(sourcePublicKey, basic.gameOperator, 32) != 0)
    {
        getIdentityFromPublicKey(basic.gameOperator, goIdentity, false);
        LOG("Error: seed is not the game operator\n");
        LOG("Current identity: %s\n", sourceIdentity);
        LOG("Game operator:    %s\n", goIdentity);
        return;
    }

    getEventInfo_output eventInfo{};
    _quotteryGetEventInfo(qc, eventId, eventInfo);

    if (isArrayZero((uint8_t*)&eventInfo, sizeof(eventInfo)))
    {
        LOG("Error: failed to get event info for eventId %" PRIu64 "\n", eventId);
        return;
    }

    if (eventInfo.qei.eid == (uint64_t)-1)
    {
        LOG("Error: eventId %" PRIu64 " does not exist\n", eventId);
        return;
    }

    if (!isCurrentUtcAfterPackedDateTime(eventInfo.qei.endDate))
    {
        uint32_t year;
        uint8_t month, day, hour, minute, second;
        uint16_t millisec, microsec;
        unpackDateTime(year, month, day, hour, minute, second, millisec, microsec, eventInfo.qei.endDate);
        LOG("Error: event %" PRIu64 " has not ended yet\n", eventId);
        LOG("End date (UTC): %04u-%02u-%02u %02u:%02u:%02u\n", year, month, day, hour, minute, second);
        return;
    }

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryPublishResult_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    packet.transaction.amount = static_cast<int64_t>(basic.depositAmountForDispute);

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_PUBLISH_RESULT;
    packet.transaction.inputSize = sizeof(qtryPublishResult_input);

    packet.input.eventId = eventId;
    packet.input.option = result;

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY publish result\n");
    LOG("eventId: %" PRIu64 "\n", eventId);
    LOG("result: %" PRIu64 "\n", result);
    LOG("depositAmountForDispute: %" PRIu64 "\n", basic.depositAmountForDispute);
    LOG("scheduledTick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryPublishResult_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryPublishResult_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryPublishResult_input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("Publish result transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qtryTryFinalizeEvent(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t eventId)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };
    char goIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    qtryBasicInfo_output basic{};
    quotteryGetBasicInfo(qc, basic);

    if (isArrayZero((uint8_t*)&basic, sizeof(basic)))
    {
        LOG("Error: failed to get Quottery basic info\n");
        return;
    }

    if (memcmp(sourcePublicKey, basic.gameOperator, 32) != 0)
    {
        getIdentityFromPublicKey(basic.gameOperator, goIdentity, false);
        LOG("Error: seed is not the game operator\n");
        LOG("Current identity: %s\n", sourceIdentity);
        LOG("Game operator:    %s\n", goIdentity);
        return;
    }

    getEventInfo_output eventInfo{};
    _quotteryGetEventInfo(qc, eventId, eventInfo);

    if (isArrayZero((uint8_t*)&eventInfo, sizeof(eventInfo)))
    {
        LOG("Error: failed to get event info for eventId %" PRIu64 "\n", eventId);
        return;
    }

    if (eventInfo.qei.eid != eventId)
    {
        LOG("Error: eventId %" PRIu64 " does not exist\n", eventId);
        return;
    }

    if (eventInfo.resultByGO == -1)
    {
        LOG("Error: event %" PRIu64 " does not have a published result yet\n", eventId);
        return;
    }

    if (!isZeroPubkey(eventInfo.disputerInfo.pubkey))
    {
        LOG("Error: event %" PRIu64 " is under dispute and cannot be finalized\n", eventId);
        return;
    }

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    if (eventInfo.publishTickTime + 1000 > scheduledTick)
    {
        LOG("Error: event %" PRIu64 " cannot be finalized yet\n", eventId);
        LOG("Publish tick: %" PRIu32 "\n", eventInfo.publishTickTime);
        LOG("Earliest finalize tick: %" PRIu32 "\n", eventInfo.publishTickTime + 1000);
        LOG("Scheduled tick: %" PRIu32 "\n", scheduledTick);
        return;
    }

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryTryFinalizeEvent_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    packet.transaction.amount = 0;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_TRY_FINALIZE_EVENT;
    packet.transaction.inputSize = sizeof(qtryTryFinalizeEvent_input);

    packet.input.eventId = eventId;

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY try finalize event\n");
    LOG("eventId: %" PRIu64 "\n", eventId);
    LOG("scheduledTick: %u\n", scheduledTick);
    LOG("publishTickTime: %" PRIu32 "\n", eventInfo.publishTickTime);
    LOG("earliestFinalizeTick: %" PRIu32 "\n", eventInfo.publishTickTime + 1000);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryTryFinalizeEvent_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryTryFinalizeEvent_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryTryFinalizeEvent_input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("Try finalize event transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

struct qtryDispute_input
{
    uint64_t eventId;
};

void qtryDispute(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t eventId)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    // Fetch basic info to get depositAmountForDispute
    qtryBasicInfo_output basic{};
    quotteryGetBasicInfo(qc, basic);

    if (isArrayZero((uint8_t*)&basic, sizeof(basic)))
    {
        LOG("Error: failed to get Quottery basic info\n");
        return;
    }

    const uint64_t depositAmount = basic.depositAmountForDispute;

    // Fetch event info to validate dispute preconditions
    getEventInfo_output eventInfo{};
    _quotteryGetEventInfo(qc, eventId, eventInfo);

    if (isArrayZero((uint8_t*)&eventInfo, sizeof(eventInfo)))
    {
        LOG("Error: failed to get event info for eventId %" PRIu64 "\n", eventId);
        return;
    }

    if (eventInfo.qei.eid != eventId)
    {
        LOG("Error: eventId %" PRIu64 " does not exist\n", eventId);
        return;
    }

    // Check: result must have been published (event is in dispute window)
    if (eventInfo.resultByGO == -1)
    {
        LOG("Error: event %" PRIu64 " does not have a published result yet. Nothing to dispute.\n", eventId);
        return;
    }

    // Check: event must not already be finalized
    if (eventInfo.publishTickTime == 0xffffffffu)
    {
        LOG("Error: event %" PRIu64 " is already finalized. Cannot dispute.\n", eventId);
        return;
    }

    // Check: event must not already be under dispute
    if (!isZeroPubkey(eventInfo.disputerInfo.pubkey))
    {
        char existingDisputer[128] = { 0 };
        getIdentityFromPublicKey(eventInfo.disputerInfo.pubkey, existingDisputer, false);
        LOG("Error: event %" PRIu64 " is already being disputed by %s\n", eventId, existingDisputer);
        return;
    }

    // Check: event is still in the dispute window (not yet eligible for finalization)
    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    if (eventInfo.publishTickTime + 1000 <= scheduledTick)
    {
        LOG("Warning: dispute window may have passed for event %" PRIu64 "\n", eventId);
        LOG("Publish tick: %" PRIu32 ", finalize eligible at tick: %" PRIu32 ", scheduled tick: %" PRIu32 "\n",
            eventInfo.publishTickTime, eventInfo.publishTickTime + 1000, scheduledTick);
        LOG("The event may already be finalized by the time this transaction executes.\n");
    }

    // Check: user balance >= depositAmount
    {
        long long balance = getBalanceNumber(qc, sourcePublicKey);
        if (balance < 0)
        {
            LOG("Error: failed to query balance\n");
            return;
        }
        if (static_cast<uint64_t>(balance) < depositAmount)
        {
            LOG("Error: insufficient balance for dispute deposit\n");
            LOG("Required: %" PRIu64 ", available: %lld\n", depositAmount, balance);
            return;
        }
    }

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryDispute_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    // The SC checks: qpi.invocationReward() == mDepositAmountForDispute
    packet.transaction.amount = static_cast<int64_t>(depositAmount);
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_DISPUTE;
    packet.transaction.inputSize = sizeof(qtryDispute_input);

    packet.input.eventId = eventId;

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY Dispute\n");
    LOG("eventId: %" PRIu64 "\n", eventId);
    LOG("depositAmountForDispute: %" PRIu64 "\n", depositAmount);
    LOG("scheduledTick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryDispute_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryDispute_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryDispute_input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("Dispute transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

struct qtryResolveDispute_input
{
    uint64_t eventId;
    int64_t vote;
};

void qtryResolveDispute(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t eventId, int64_t vote)
{
    if (vote != 0 && vote != 1)
    {
        LOG("Error: vote must be 0 or 1\n");
        return;
    }

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    // Fetch event info to validate preconditions
    getEventInfo_output eventInfo{};
    _quotteryGetEventInfo(qc, eventId, eventInfo);

    if (isArrayZero((uint8_t*)&eventInfo, sizeof(eventInfo)))
    {
        LOG("Error: failed to get event info for eventId %" PRIu64 "\n", eventId);
        return;
    }

    if (eventInfo.qei.eid != eventId)
    {
        LOG("Error: eventId %" PRIu64 " does not exist\n", eventId);
        return;
    }

    // Check: event must be under dispute
    if (isZeroPubkey(eventInfo.disputerInfo.pubkey))
    {
        LOG("Error: event %" PRIu64 " is not under dispute\n", eventId);
        return;
    }

    // SC requires invocationReward >= 10000000, but refunds it to computors
    constexpr int64_t MIN_INVOCATION_REWARD = 10000000;

    // Check balance
    {
        long long balance = getBalanceNumber(qc, sourcePublicKey);
        if (balance < 0)
        {
            LOG("Error: failed to query balance\n");
            return;
        }
        if (static_cast<uint64_t>(balance) < static_cast<uint64_t>(MIN_INVOCATION_REWARD))
        {
            LOG("Error: insufficient balance\n");
            LOG("Required (refunded if computor): %" PRId64 ", available: %lld\n", MIN_INVOCATION_REWARD, balance);
            return;
        }
    }

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryResolveDispute_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    // SC requires invocationReward >= 10000000; refunded to computors
    packet.transaction.amount = MIN_INVOCATION_REWARD;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_RESOLVE_DISPUTE;
    packet.transaction.inputSize = sizeof(qtryResolveDispute_input);

    packet.input.eventId = eventId;
    packet.input.vote = vote;

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY ResolveDispute\n");
    LOG("Caller: %s\n", sourceIdentity);
    LOG("eventId: %" PRIu64 "\n", eventId);
    LOG("vote: %" PRId64 " (%s)\n", vote, vote == 0 ? "No" : "Yes");
    LOG("invocationReward: %" PRId64 " (refunded if caller is a computor)\n", MIN_INVOCATION_REWARD);
    LOG("scheduledTick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryResolveDispute_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryResolveDispute_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryResolveDispute_input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("ResolveDispute transaction has been sent!\n");
    LOG("Note: only computors can resolve disputes. If the caller is not a computor, the invocation reward will NOT be refunded.\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

struct qtryUserClaimReward_input
{
    uint64_t eventId;
};

void qtryUserClaimReward(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t eventId)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    // Fetch event info to validate preconditions
    getEventInfo_output eventInfo{};
    _quotteryGetEventInfo(qc, eventId, eventInfo);

    if (isArrayZero((uint8_t*)&eventInfo, sizeof(eventInfo)))
    {
        LOG("Error: failed to get event info for eventId %" PRIu64 "\n", eventId);
        return;
    }

    if (eventInfo.qei.eid != eventId)
    {
        LOG("Error: eventId %" PRIu64 " does not exist\n", eventId);
        return;
    }

    // Check: result must be set (finalized)
    if (eventInfo.resultByGO == -1)
    {
        LOG("Error: event %" PRIu64 " does not have a result yet. Cannot claim reward.\n", eventId);
        return;
    }

    // Check: event should be finalized (publishTickTime == 0xFFFFFFFF indicates finalized & waiting for cleanup)
    // If still in dispute window, warn the user
    if (eventInfo.publishTickTime != 0xffffffffu)
    {
        if (!isZeroPubkey(eventInfo.disputerInfo.pubkey))
        {
            LOG("Warning: event %" PRIu64 " is still under dispute. The result may change.\n", eventId);
        }
        else
        {
            LOG("Warning: event %" PRIu64 " may not be finalized yet (publishTickTime: %" PRIu32 ").\n",
                eventId, eventInfo.publishTickTime);
            LOG("The SC will reject the claim if the result is not set.\n");
        }
    }

    // Check user has a position in this event (winning side)
    {
        getUserPosition_output posResult{};
        quotteryGetUserPosition(nodeIp, nodePort, sourceIdentity, posResult);

        bool hasPosition = false;
        for (int64_t idx = 0; idx < posResult.count; idx++)
        {
            uint64_t posEventId = QUOTTERY_EO_GET_EVENTID(posResult.p[idx].eo);
            if (posEventId == eventId)
            {
                uint64_t posOption = QUOTTERY_EO_GET_OPTION(posResult.p[idx].eo);
                LOG("Found position in event %" PRIu64 ": option %" PRIu64 ", amount %" PRId64 "\n",
                    eventId, posOption, posResult.p[idx].amount);
                hasPosition = true;
                break;
            }
        }

        if (!hasPosition)
        {
            LOG("Error: no position found for %s in event %" PRIu64 "\n", sourceIdentity, eventId);
            LOG("You must have a position in this event to claim a reward.\n");
            return;
        }
    }

    // SC requires exactly 1,000,000 qu — NOT refunded if user has no winning position (anti-spam)
    constexpr int64_t CLAIM_INVOCATION_REWARD = 1000000;

    // Check balance
    {
        long long balance = getBalanceNumber(qc, sourcePublicKey);
        if (balance < 0)
        {
            LOG("Error: failed to query balance\n");
            return;
        }
        if (static_cast<uint64_t>(balance) < static_cast<uint64_t>(CLAIM_INVOCATION_REWARD))
        {
            LOG("Error: insufficient balance\n");
            LOG("Required: %" PRId64 " (refunded if you have a winning position), available: %lld\n",
                CLAIM_INVOCATION_REWARD, balance);
            return;
        }
    }

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryUserClaimReward_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    packet.transaction.amount = CLAIM_INVOCATION_REWARD;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_USER_CLAIM_REWARD;
    packet.transaction.inputSize = sizeof(qtryUserClaimReward_input);

    packet.input.eventId = eventId;

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY UserClaimReward\n");
    LOG("Caller: %s\n", sourceIdentity);
    LOG("eventId: %" PRIu64 "\n", eventId);
    LOG("invocationReward: %" PRId64 " (refunded if winning position exists)\n", CLAIM_INVOCATION_REWARD);
    LOG("scheduledTick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryUserClaimReward_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryUserClaimReward_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryUserClaimReward_input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("UserClaimReward transaction has been sent!\n");
    LOG("Note: the 1,000,000 qu fee is only refunded if you have a winning position. Otherwise it is lost (anti-spam).\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

struct qtryGOForceClaimReward_input
{
    uint64_t eventId;
    uint8_t pubkeys[16][32]; // Array<id, 16>
};

void qtryGOForceClaimReward(const char* nodeIp, int nodePort, const char* seed,
    uint32_t scheduledTickOffset, uint64_t eventId,
    const char* identities[], int identityCount)
{
    if (identityCount <= 0 || identityCount > 16)
    {
        LOG("Error: must provide between 1 and 16 public key identities\n");
        return;
    }

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };
    char goIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    // Verify caller is the game operator
    qtryBasicInfo_output basic{};
    quotteryGetBasicInfo(qc, basic);

    if (isArrayZero((uint8_t*)&basic, sizeof(basic)))
    {
        LOG("Error: failed to get Quottery basic info\n");
        return;
    }

    if (memcmp(sourcePublicKey, basic.gameOperator, 32) != 0)
    {
        getIdentityFromPublicKey(basic.gameOperator, goIdentity, false);
        LOG("Error: seed is not the game operator\n");
        LOG("Current identity: %s\n", sourceIdentity);
        LOG("Game operator:    %s\n", goIdentity);
        return;
    }

    // Verify event exists and has a result
    getEventInfo_output eventInfo{};
    _quotteryGetEventInfo(qc, eventId, eventInfo);

    if (isArrayZero((uint8_t*)&eventInfo, sizeof(eventInfo)))
    {
        LOG("Error: failed to get event info for eventId %" PRIu64 "\n", eventId);
        return;
    }

    if (eventInfo.qei.eid != eventId)
    {
        LOG("Error: eventId %" PRIu64 " does not exist\n", eventId);
        return;
    }

    if (eventInfo.resultByGO == -1)
    {
        LOG("Error: event %" PRIu64 " does not have a result yet. Cannot force claim.\n", eventId);
        return;
    }

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryGOForceClaimReward_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    packet.transaction.amount = 0;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_GO_FORCE_CLAIM_REWARD;
    packet.transaction.inputSize = sizeof(qtryGOForceClaimReward_input);

    packet.input.eventId = eventId;
    memset(packet.input.pubkeys, 0, sizeof(packet.input.pubkeys));

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY GOForceClaimReward\n");
    LOG("Caller (GO): %s\n", sourceIdentity);
    LOG("eventId: %" PRIu64 "\n", eventId);
    LOG("Number of pubkeys: %d\n", identityCount);

    for (int idx = 0; idx < identityCount; idx++)
    {
        getPublicKeyFromIdentity(identities[idx], packet.input.pubkeys[idx]);
        char verifyId[128] = { 0 };
        getIdentityFromPublicKey(packet.input.pubkeys[idx], verifyId, false);
        LOG("  [%d] %s\n", idx, verifyId);
    }

    LOG("scheduledTick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryGOForceClaimReward_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryGOForceClaimReward_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryGOForceClaimReward_input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("GOForceClaimReward transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

struct qtryTransferShareManagementRights_input
{
    uint8_t issuer[32];    // Asset.issuer (id = 32 bytes public key)
    uint64_t assetName;    // Asset.assetName (uint64 encoded)
    int64_t numberOfShares;
    uint32_t newManagingContractIndex;
    uint32_t _padding;     // align to 8 bytes
};

static uint64_t encodeAssetName(const char* name)
{
    uint64_t result = 0;
    for (int i = 0; i < 7 && name[i] != '\0'; i++)
    {
        char c = name[i];
        // Qubic asset names are uppercase A-Z mapped to values 1-26
        if (c >= 'a' && c <= 'z')
            c = c - 'a' + 'A';
        if (c < 'A' || c > 'Z')
        {
            LOG("Error: invalid character '%c' in asset name. Only A-Z allowed.\n", name[i]);
            return 0;
        }
        result |= (static_cast<uint64_t>(c - 'A' + 1)) << (i * 8);
    }
    return result;
}

void qtryTransferShareManagementRights(const char* nodeIp, int nodePort, const char* seed,
    uint32_t scheduledTickOffset,
    const char* issuerIdentity, const char* assetName,
    int64_t numberOfShares, uint32_t newManagingContractIndex)
{
    if (numberOfShares <= 0)
    {
        LOG("Error: numberOfShares must be positive\n");
        return;
    }

    uint64_t encodedAssetName = encodeAssetName(assetName);
    if (encodedAssetName == 0)
    {
        LOG("Error: failed to encode asset name '%s'\n", assetName);
        return;
    }

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryTransferShareManagementRights_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    // The SC uses invocationReward() for releaseShares fee; any excess is refunded
    packet.transaction.amount = 0;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_TRANSFER_SHARE_MANAGEMENT_RIGHTS;
    packet.transaction.inputSize = sizeof(qtryTransferShareManagementRights_input);

    memset(&packet.input, 0, sizeof(qtryTransferShareManagementRights_input));
    getPublicKeyFromIdentity(issuerIdentity, packet.input.issuer);
    packet.input.assetName = encodedAssetName;
    packet.input.numberOfShares = numberOfShares;
    packet.input.newManagingContractIndex = newManagingContractIndex;

    char issuerBuf[128] = { 0 };
    getIdentityFromPublicKey(packet.input.issuer, issuerBuf, false);

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY TransferShareManagementRights\n");
    LOG("Caller: %s\n", sourceIdentity);
    LOG("Asset issuer: %s\n", issuerBuf);
    LOG("Asset name: %s (encoded: %" PRIu64 ")\n", assetName, encodedAssetName);
    LOG("Number of shares: %" PRId64 "\n", numberOfShares);
    LOG("New managing contract index: %" PRIu32 "\n", newManagingContractIndex);
    LOG("scheduledTick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryTransferShareManagementRights_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryTransferShareManagementRights_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryTransferShareManagementRights_input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("TransferShareManagementRights transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void qtryCleanMemory(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };
    char goIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    // Verify caller is the game operator
    qtryBasicInfo_output basic{};
    quotteryGetBasicInfo(qc, basic);

    if (isArrayZero((uint8_t*)&basic, sizeof(basic)))
    {
        LOG("Error: failed to get Quottery basic info\n");
        return;
    }

    if (memcmp(sourcePublicKey, basic.gameOperator, 32) != 0)
    {
        getIdentityFromPublicKey(basic.gameOperator, goIdentity, false);
        LOG("Error: seed is not the game operator\n");
        LOG("Current identity: %s\n", sourceIdentity);
        LOG("Game operator:    %s\n", goIdentity);
        return;
    }

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    packet.transaction.amount = 0;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_CLEAN_MEMORY;
    packet.transaction.inputSize = 0; // NoData input

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY CleanMemory\n");
    LOG("Caller (GO): %s\n", sourceIdentity);
    LOG("scheduledTick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("CleanMemory transaction has been sent!\n");
    LOG("This will finalize events with published results (past dispute window) and clean up state.\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

struct qtryTransferQUSD_input
{
    uint8_t receiver[32];
    int64_t amount;
};

void qtryTransferQUSD(const char* nodeIp, int nodePort, const char* seed,
    const char* receiverIdentity, int64_t amount,
    uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryTransferQUSD_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    packet.transaction.amount = 0; // no invocation reward needed

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_TRANSFER_QUSD;
    packet.transaction.inputSize = sizeof(qtryTransferQUSD_input);

    memset(&packet.input, 0, sizeof(qtryTransferQUSD_input));
    getPublicKeyFromIdentity(receiverIdentity, packet.input.receiver);
    packet.input.amount = amount;

    char receiverBuf[128] = { 0 };
    getIdentityFromPublicKey(packet.input.receiver, receiverBuf, false);

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY TransferQUSD (GARTH transfer via Quottery contract)\n");
    LOG("Receiver: %s\n", receiverBuf);
    LOG("Amount: %" PRId64 "\n", amount);
    LOG("Scheduled tick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryTransferQUSD_input),
        digest,
        32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryTransferQUSD_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryTransferQUSD_input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);

    LOG("TransferQUSD transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

// TransferQTRYGOV (proc 15)
struct qtryTransferQTRYGOV_input
{
    uint8_t receiver[32];
    int64_t amount;
};

void qtryTransferQTRYGOV(const char* nodeIp, int nodePort, const char* seed,
    const char* receiverIdentity, int64_t amount,
    uint32_t scheduledTickOffset)
{
    if (amount <= 0)
    {
        LOG("Error: amount must be positive\n");
        return;
    }

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryTransferQTRYGOV_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    packet.transaction.amount = 0;

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_TRANSFER_QTRYGOV;
    packet.transaction.inputSize = sizeof(qtryTransferQTRYGOV_input);

    memset(&packet.input, 0, sizeof(qtryTransferQTRYGOV_input));
    getPublicKeyFromIdentity(receiverIdentity, packet.input.receiver);
    packet.input.amount = amount;

    char receiverBuf[128] = { 0 };
    getIdentityFromPublicKey(packet.input.receiver, receiverBuf, false);

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY TransferQTRYGOV\n");
    LOG("Receiver: %s\n", receiverBuf);
    LOG("Amount: %" PRId64 "\n", amount);
    LOG("Scheduled tick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryTransferQTRYGOV_input),
        digest, 32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryTransferQTRYGOV_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryTransferQTRYGOV_input) + SIGNATURE_SIZE,
        digest, 32);
    getTxHashFromDigest(digest, txHash);

    LOG("TransferQTRYGOV transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

// UpdateFeeDiscountList (proc 20)
struct qtryUpdateFeeDiscountList_input
{
    uint8_t userId[32];
    uint64_t newFeeRate;
    uint64_t ops; // 0 = remove, 1 = set
};

void qtryUpdateFeeDiscountList(const char* nodeIp, int nodePort, const char* seed,
    uint32_t scheduledTickOffset,
    const char* userIdentity, uint64_t newFeeRate, uint64_t ops)
{
    if (ops != 0 && ops != 1)
    {
        LOG("Error: ops must be 0 (remove) or 1 (set)\n");
        return;
    }

    if (ops == 1 && newFeeRate > 1000)
    {
        LOG("Error: newFeeRate must be <= 1000 (100%%)\n");
        return;
    }

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };
    char goIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    qtryBasicInfo_output basic{};
    quotteryGetBasicInfo(qc, basic);

    if (memcmp(sourcePublicKey, basic.gameOperator, 32) != 0)
    {
        getIdentityFromPublicKey(basic.gameOperator, goIdentity, false);
        LOG("Error: seed is not the game operator\n");
        LOG("Current identity: %s\n", sourceIdentity);
        LOG("Game operator:    %s\n", goIdentity);
        return;
    }

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryUpdateFeeDiscountList_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    packet.transaction.amount = 0;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_UPDATE_FEE_DISCOUNT_LIST;
    packet.transaction.inputSize = sizeof(qtryUpdateFeeDiscountList_input);

    memset(&packet.input, 0, sizeof(qtryUpdateFeeDiscountList_input));
    getPublicKeyFromIdentity(userIdentity, packet.input.userId);
    packet.input.newFeeRate = newFeeRate;
    packet.input.ops = ops;

    char userBuf[128] = { 0 };
    getIdentityFromPublicKey(packet.input.userId, userBuf, false);

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY UpdateFeeDiscountList\n");
    LOG("User: %s\n", userBuf);
    LOG("Operation: %s\n", ops == 0 ? "remove" : "set");
    if (ops == 1) LOG("New fee rate: %" PRIu64 " (%.1f%%)\n", newFeeRate, newFeeRate / 10.0);
    LOG("Scheduled tick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryUpdateFeeDiscountList_input),
        digest, 32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryUpdateFeeDiscountList_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryUpdateFeeDiscountList_input) + SIGNATURE_SIZE,
        digest, 32);
    getTxHashFromDigest(digest, txHash);

    LOG("UpdateFeeDiscountList transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

// ProposalVote (proc 100)
struct qtryProposalVote_input
{
    QtryGOV_cli proposed;
};

void qtryProposalVote(const char* nodeIp, int nodePort, const char* seed,
    uint32_t scheduledTickOffset,
    uint64_t operationFee, uint64_t shareholderFee, uint64_t burnFee,
    int64_t feePerDay, int64_t depositAmountForDispute,
    const char* operationIdIdentity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t subSeed[32] = { 0 };
    uint8_t digest[32] = { 0 };
    uint8_t signature[64] = { 0 };
    char txHash[128] = { 0 };
    char sourceIdentity[128] = { 0 };

    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, sourceIdentity, false);

    ((uint64_t*)destPublicKey)[0] = QUOTTERY_CONTRACT_ID;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    const uint32_t currentTick = getTickNumberFromNode(qc);
    const uint32_t scheduledTick = currentTick + scheduledTickOffset;

    struct
    {
        RequestResponseHeader header;
        Transaction transaction;
        qtryProposalVote_input input;
        uint8_t sig[SIGNATURE_SIZE];
    } packet{};

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);

    {
        qtryBasicInfo_output basic{};
        quotteryGetBasicInfo(qc, basic);
        packet.transaction.amount = static_cast<int64_t>(basic.antiSpamAmount);
    }
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = QTRY_PROPOSAL_VOTE;
    packet.transaction.inputSize = sizeof(qtryProposalVote_input);

    memset(&packet.input, 0, sizeof(qtryProposalVote_input));
    packet.input.proposed.mOperationFee = operationFee;
    packet.input.proposed.mShareHolderFee = shareholderFee;
    packet.input.proposed.mBurnFee = burnFee;
    packet.input.proposed.feePerDay = feePerDay;
    packet.input.proposed.mDepositAmountForDispute = depositAmountForDispute;
    getPublicKeyFromIdentity(operationIdIdentity, packet.input.proposed.mOperationId);

    char opIdBuf[128] = { 0 };
    getIdentityFromPublicKey(packet.input.proposed.mOperationId, opIdBuf, false);

    LOG("\n-------------------------------------\n\n");
    LOG("Sending QTRY ProposalVote\n");
    LOG("Caller: %s\n", sourceIdentity);
    LOG("Proposed params:\n");
    LOG("  operationFee: %" PRIu64 " (%.1f%%)\n", operationFee, operationFee / 10.0);
    LOG("  shareholderFee: %" PRIu64 " (%.1f%%)\n", shareholderFee, shareholderFee / 10.0);
    LOG("  burnFee: %" PRIu64 " (%.1f%%)\n", burnFee, burnFee / 10.0);
    LOG("  feePerDay: %" PRId64 "\n", feePerDay);
    LOG("  depositAmountForDispute: %" PRId64 "\n", depositAmountForDispute);
    LOG("  operationId: %s\n", opIdBuf);
    LOG("Scheduled tick: %u\n", scheduledTick);
    LOG("\n-------------------------------------\n\n");

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryProposalVote_input),
        digest, 32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(qtryProposalVote_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t*)&packet, packet.header.size());

    KangarooTwelve((unsigned char*)&packet.transaction,
        sizeof(Transaction) + sizeof(qtryProposalVote_input) + SIGNATURE_SIZE,
        digest, 32);
    getTxHashFromDigest(digest, txHash);

    LOG("ProposalVote transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t*>(&packet.input));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

// GetApprovedAmount (fn 7)
void quotteryGetApprovedAmount(const char* nodeIp, int nodePort, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        getApprovedAmount_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(getApprovedAmount_input);
    packet.rcf.inputType = QTRY_GET_APPROVED_AMOUNT;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
    memset(&packet.input, 0, sizeof(getApprovedAmount_input));
    getPublicKeyFromIdentity(identity, packet.input.pk);
    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        auto result = qc->receivePacketWithHeaderAs<getApprovedAmount_output>();
        LOG("Approved QUSD amount for %s: %" PRIu64 "\n", identity, result.amount);
    }
    catch (std::logic_error)
    {
        LOG("Failed to get approved amount\n");
    }
}

// GetTopProposals (fn 8)
void quotteryGetTopProposals(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = QTRY_GET_TOP_PROPOSALS;
    packet.rcf.contractIndex = QUOTTERY_CONTRACT_ID;
    qc->sendData((uint8_t*)&packet, packet.header.size());

    try
    {
        auto result = qc->receivePacketWithHeaderAs<getTopProposals_output>();
        LOG("Top governance proposals (%" PRId32 " unique this epoch):\n", result.uniqueCount);
        for (int i = 0; i < 3 && i < result.uniqueCount; i++)
        {
            const auto& p = result.top[i];
            if (p.totalVotes == 0) break;
            char opId[128] = { 0 };
            getIdentityFromPublicKey(p.proposed.mOperationId, opId, false);
            LOG("\n  #%d (%" PRId64 " votes):\n", i + 1, p.totalVotes);
            LOG("    operationFee: %" PRIu64 " (%.1f%%)\n", p.proposed.mOperationFee, p.proposed.mOperationFee / 10.0);
            LOG("    shareholderFee: %" PRIu64 " (%.1f%%)\n", p.proposed.mShareHolderFee, p.proposed.mShareHolderFee / 10.0);
            LOG("    burnFee: %" PRIu64 " (%.1f%%)\n", p.proposed.mBurnFee, p.proposed.mBurnFee / 10.0);
            LOG("    feePerDay: %" PRId64 "\n", p.proposed.feePerDay);
            LOG("    depositAmountForDispute: %" PRId64 "\n", p.proposed.mDepositAmountForDispute);
            LOG("    operationId: %s\n", opId);
        }
        if (result.uniqueCount == 0) LOG("  (none)\n");
    }
    catch (std::logic_error)
    {
        LOG("Failed to get top proposals\n");
    }
}

static bool hasRequiredParameters(int currentIndex, int argc, int requiredCount, const char* command)
{
    if (currentIndex + requiredCount >= argc)
    {
        LOG("Error: %s expects %d parameter(s)\n", command, requiredCount);
        return false;
    }
    return true;
}

static bool hasNoExtraParameters(int nextIndex, int argc, const char* command)
{
    if (nextIndex != argc)
    {
        LOG("Error: unexpected extra parameter(s) after %s\n", command);
        return false;
    }
    return true;
}

static bool tryParseUint64Arg(const char* text, uint64_t& value, const char* fieldName)
{
    if (text == nullptr || *text == '\0')
    {
        LOG("Error: %s is empty\n", fieldName);
        return false;
    }

    char* endPtr = nullptr;
    const unsigned long long parsed = std::strtoull(text, &endPtr, 10);
    if (*endPtr != '\0')
    {
        LOG("Error: invalid numeric value for %s: %s\n", fieldName, text);
        return false;
    }

    value = static_cast<uint64_t>(parsed);
    return true;
}

void quotteryEntryPoint(int argc, char** argv, const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset)
{
    constexpr uint64_t defaultAntiSpamAmount = 0;

    int i = 0;
    while (i < argc)
    {
        if (strcmp(argv[i], "getbasicinfo") == 0)
        {
            if (!hasNoExtraParameters(i + 1, argc, "getbasicinfo"))
                return;
            quotteryPrintBasicInfo(nodeIp, nodePort);
            return;
        }

        if (strcmp(argv[i], "getactiveeventsid") == 0)
        {
            if (!hasNoExtraParameters(i + 1, argc, "getactiveeventsid"))
                return;

            quotteryPrintActiveEvents(nodeIp, nodePort);
            return;
        }

        if (strcmp(argv[i], "createevent") == 0)
        {
            if (!hasRequiredParameters(i, argc, 5, "createevent"))
                return;

            const std::string eventDesc = argv[i + 1];
            const std::string opt0Desc = argv[i + 2];
            const std::string opt1Desc = argv[i + 3];
            const std::string endDate = argv[i + 4];

            uint64_t tagIdRaw = 0;
            if (!tryParseUint64Arg(argv[i + 5], tagIdRaw, "tagId"))
                return;
            uint16_t tagId = static_cast<uint16_t>(tagIdRaw);

            i += 6;
            if (!hasNoExtraParameters(i, argc, "createevent"))
                return;

            quotteryCreateEvent(nodeIp, nodePort, seed,
                eventDesc,
                opt0Desc,
                opt1Desc,
                endDate,
                tagId,
                scheduledTickOffset);
            return;
        }

        if (strcmp(argv[i], "order") == 0)
        {
            if (!hasRequiredParameters(i, argc, 6, "order"))
                return;

            const char* action = argv[i + 1];
            const char* side = argv[i + 2];

            uint64_t eventId = 0;
            uint64_t option = 0;
            uint64_t amount = 0;
            uint64_t price = 0;

            if (!tryParseUint64Arg(argv[i + 3], eventId, "eventId"))
                return;
            if (!tryParseUint64Arg(argv[i + 4], option, "option"))
                return;
            if (!tryParseUint64Arg(argv[i + 5], amount, "amount"))
                return;
            if (!tryParseUint64Arg(argv[i + 6], price, "price"))
                return;

            i += 7;
            if (!hasNoExtraParameters(i, argc, "order"))
                return;

            if (strcmp(action, "add") == 0 && strcmp(side, "ask") == 0)
            {
                qtryAddToAskOrder(nodeIp, nodePort, seed, eventId, option, amount,
                    static_cast<int64_t>(price), defaultAntiSpamAmount, scheduledTickOffset);
            }
            else if (strcmp(action, "add") == 0 && strcmp(side, "bid") == 0)
            {
                qtryAddToBidOrder(nodeIp, nodePort, seed, eventId, option, amount,
                    static_cast<int64_t>(price), defaultAntiSpamAmount, scheduledTickOffset);
            }
            else if (strcmp(action, "remove") == 0 && strcmp(side, "ask") == 0)
            {
                qtryRemoveAskOrder(nodeIp, nodePort, seed, eventId, option, amount,
                    static_cast<int64_t>(price), defaultAntiSpamAmount, scheduledTickOffset);
            }
            else if (strcmp(action, "remove") == 0 && strcmp(side, "bid") == 0)
            {
                qtryRemoveBidOrder(nodeIp, nodePort, seed, eventId, option, amount,
                    static_cast<int64_t>(price), defaultAntiSpamAmount, scheduledTickOffset);
            }
            else
            {
                LOG("Invalid qtryorder command: %s %s. Expected: add/remove bid/ask\n", action, side);
            }
            return;
        }

        if (strcmp(argv[i], "getorder") == 0)
        {
            if (!hasRequiredParameters(i, argc, 4, "getorder"))
                return;

            const char* side = argv[i + 1];
            uint64_t eventId = 0;
            uint64_t option = 0;
            uint64_t offset = 0;

            if (!tryParseUint64Arg(argv[i + 2], eventId, "eventId"))
                return;
            if (!tryParseUint64Arg(argv[i + 3], option, "option"))
                return;
            if (!tryParseUint64Arg(argv[i + 4], offset, "offset"))
                return;

            i += 5;
            if (!hasNoExtraParameters(i, argc, "getorder"))
                return;

            const uint64_t isBid = (strcmp(side, "bid") == 0) ? 1 : 0;
            if (strcmp(side, "bid") != 0 && strcmp(side, "ask") != 0)
            {
                LOG("Invalid qtrygetorder side: %s. Expected: bid/ask\n", side);
                return;
            }

            quotteryPrintOrders(nodeIp, nodePort, eventId, option, isBid, offset);
            return;
        }

        if (strcmp(argv[i], "getposition") == 0)
        {
            if (!hasRequiredParameters(i, argc, 1, "getposition"))
                return;

            const char* identity = argv[i + 1];
            i += 2;
            if (!hasNoExtraParameters(i, argc, "getposition"))
                return;

            quotteryPrintUserPosition(nodeIp, nodePort, identity);
            return;
        }

        if (strcmp(argv[i], "geteventinfo") == 0)
        {
            if (!hasRequiredParameters(i, argc, 1, "geteventinfo"))
                return;

            uint64_t eventId = 0;
            if (!tryParseUint64Arg(argv[i + 1], eventId, "eventId"))
                return;

            i += 2;
            if (!hasNoExtraParameters(i, argc, "geteventinfo"))
                return;

            quotteryPrintEventInfo(nodeIp, nodePort, eventId);
            return;
        }

        if (strcmp(argv[i], "geteventinfobatch") == 0)
        {
            if (!hasRequiredParameters(i, argc, 1, "geteventinfobatch"))
                return;

            uint64_t eventIds[64] = {};
            size_t count = 0;
            int j = i + 1;

            while (j < argc && count < 64)
            {
                if (!tryParseUint64Arg(argv[j], eventIds[count], "eventId"))
                    return;
                ++count;
                ++j;
            }

            if (j < argc)
            {
                LOG("Error: geteventinfobatch supports at most 64 event ids\n");
                return;
            }

            quotteryPrintEventInfoBatch(nodeIp, nodePort, eventIds, count);
            return;
        }

        if (strcmp(argv[i], "publishresult") == 0) {
            if (!hasRequiredParameters(i, argc, 2, "publishresult"))
                return;

            uint64_t eventId = 0;
            uint64_t optionId = 0;
            if (!tryParseUint64Arg(argv[i + 1], eventId, "eventId"))
                return;
            if (!tryParseUint64Arg(argv[i + 2], optionId, "optionId"))
                return;

            i += 3;
            if (!hasNoExtraParameters(i, argc, "publishresult"))
                return;

            qtryPublishResult(nodeIp, nodePort, seed, scheduledTickOffset, eventId, optionId);
            return;
        }

        if (strcmp(argv[i], "tryfinalizeevent") == 0)
        {
            if (!hasRequiredParameters(i, argc, 1, "tryfinalizeevent"))
                return;

            uint64_t eventId = 0;
            if (!tryParseUint64Arg(argv[i + 1], eventId, "eventId"))
                return;

            i += 2;
            if (!hasNoExtraParameters(i, argc, "tryfinalizeevent"))
                return;

            qtryTryFinalizeEvent(nodeIp, nodePort, seed, scheduledTickOffset, eventId);
            return;
        }

        if (strcmp(argv[i], "dispute") == 0)
        {
            if (!hasRequiredParameters(i, argc, 1, "dispute"))
                return;

            uint64_t eventId = 0;
            if (!tryParseUint64Arg(argv[i + 1], eventId, "eventId"))
                return;

            i += 2;
            if (!hasNoExtraParameters(i, argc, "dispute"))
                return;

            qtryDispute(nodeIp, nodePort, seed, scheduledTickOffset, eventId);
            return;
        }

        if (strcmp(argv[i], "resolvedispute") == 0)
        {
            if (!hasRequiredParameters(i, argc, 2, "resolvedispute"))
                return;

            uint64_t eventId = 0;
            uint64_t voteRaw = 0;
            if (!tryParseUint64Arg(argv[i + 1], eventId, "eventId"))
                return;
            if (!tryParseUint64Arg(argv[i + 2], voteRaw, "vote"))
                return;

            if (voteRaw != 0 && voteRaw != 1)
            {
                LOG("Error: vote must be 0 (No) or 1 (Yes)\n");
                return;
            }

            i += 3;
            if (!hasNoExtraParameters(i, argc, "resolvedispute"))
                return;

            qtryResolveDispute(nodeIp, nodePort, seed, scheduledTickOffset, eventId, static_cast<int64_t>(voteRaw));
            return;
        }

        if (strcmp(argv[i], "claimreward") == 0)
        {
            if (!hasRequiredParameters(i, argc, 1, "claimreward"))
                return;

            uint64_t eventId = 0;
            if (!tryParseUint64Arg(argv[i + 1], eventId, "eventId"))
                return;

            i += 2;
            if (!hasNoExtraParameters(i, argc, "claimreward"))
                return;

            qtryUserClaimReward(nodeIp, nodePort, seed, scheduledTickOffset, eventId);
            return;
        }

        if (strcmp(argv[i], "forceclaimreward") == 0)
        {
            // Usage: forceclaimreward <eventId> <identity1> [identity2] ... [identity16]
            if (!hasRequiredParameters(i, argc, 2, "forceclaimreward"))
                return;

            uint64_t eventId = 0;
            if (!tryParseUint64Arg(argv[i + 1], eventId, "eventId"))
                return;

            const char* identities[16] = {};
            int identityCount = 0;
            int j = i + 2;

            while (j < argc && identityCount < 16)
            {
                identities[identityCount] = argv[j];
                ++identityCount;
                ++j;
            }

            if (j < argc)
            {
                LOG("Error: forceclaimreward supports at most 16 identities\n");
                return;
            }

            if (identityCount == 0)
            {
                LOG("Error: forceclaimreward requires at least one identity\n");
                return;
            }

            qtryGOForceClaimReward(nodeIp, nodePort, seed, scheduledTickOffset,
                eventId, identities, identityCount);
            return;
        }

        if (strcmp(argv[i], "transfersharemgmt") == 0)
        {
            // Usage: transfersharemgmt <issuerIdentity> <assetName> <numberOfShares> <newManagingContractIndex>
            if (!hasRequiredParameters(i, argc, 4, "transfersharemgmt"))
                return;

            const char* issuerIdentity = argv[i + 1];
            const char* assetName = argv[i + 2];

            uint64_t sharesRaw = 0;
            if (!tryParseUint64Arg(argv[i + 3], sharesRaw, "numberOfShares"))
                return;

            uint64_t contractIndexRaw = 0;
            if (!tryParseUint64Arg(argv[i + 4], contractIndexRaw, "newManagingContractIndex"))
                return;

            i += 5;
            if (!hasNoExtraParameters(i, argc, "transfersharemgmt"))
                return;

            qtryTransferShareManagementRights(nodeIp, nodePort, seed, scheduledTickOffset,
                issuerIdentity, assetName,
                static_cast<int64_t>(sharesRaw),
                static_cast<uint32_t>(contractIndexRaw));
            return;
        }

        if (strcmp(argv[i], "cleanmemory") == 0)
        {
            i += 1;
            if (!hasNoExtraParameters(i, argc, "cleanmemory"))
                return;

            qtryCleanMemory(nodeIp, nodePort, seed, scheduledTickOffset);
            return;
        }

        if (strcmp(argv[i], "transferqtrygov") == 0)
        {
            if (!hasRequiredParameters(i, argc, 2, "transferqtrygov"))
                return;

            const char* receiverIdentity = argv[i + 1];
            uint64_t amount = 0;
            if (!tryParseUint64Arg(argv[i + 2], amount, "amount"))
                return;

            i += 3;
            if (!hasNoExtraParameters(i, argc, "transferqtrygov"))
                return;

            qtryTransferQTRYGOV(nodeIp, nodePort, seed, receiverIdentity,
                static_cast<int64_t>(amount), scheduledTickOffset);
            return;
        }

        if (strcmp(argv[i], "updatediscount") == 0)
        {
            // Usage: updatediscount <userIdentity> <set|remove> [newFeeRate]
            if (!hasRequiredParameters(i, argc, 2, "updatediscount"))
                return;

            const char* userIdentity = argv[i + 1];
            const char* action = argv[i + 2];

            if (strcmp(action, "remove") == 0)
            {
                i += 3;
                if (!hasNoExtraParameters(i, argc, "updatediscount"))
                    return;
                qtryUpdateFeeDiscountList(nodeIp, nodePort, seed, scheduledTickOffset,
                    userIdentity, 0, 0);
            }
            else if (strcmp(action, "set") == 0)
            {
                if (!hasRequiredParameters(i, argc, 3, "updatediscount set"))
                    return;
                uint64_t newFeeRate = 0;
                if (!tryParseUint64Arg(argv[i + 3], newFeeRate, "newFeeRate"))
                    return;
                i += 4;
                if (!hasNoExtraParameters(i, argc, "updatediscount"))
                    return;
                qtryUpdateFeeDiscountList(nodeIp, nodePort, seed, scheduledTickOffset,
                    userIdentity, newFeeRate, 1);
            }
            else
            {
                LOG("Error: updatediscount action must be 'set' or 'remove'\n");
                return;
            }
            return;
        }

        if (strcmp(argv[i], "proposalvote") == 0)
        {
            // Usage: proposalvote <operationFee> <shareholderFee> <burnFee> <feePerDay> <depositAmountForDispute> <operationIdIdentity>
            if (!hasRequiredParameters(i, argc, 6, "proposalvote"))
                return;

            uint64_t opFee = 0, shFee = 0, burnFee = 0;
            uint64_t feePerDayRaw = 0, depositRaw = 0;
            if (!tryParseUint64Arg(argv[i + 1], opFee, "operationFee")) return;
            if (!tryParseUint64Arg(argv[i + 2], shFee, "shareholderFee")) return;
            if (!tryParseUint64Arg(argv[i + 3], burnFee, "burnFee")) return;
            if (!tryParseUint64Arg(argv[i + 4], feePerDayRaw, "feePerDay")) return;
            if (!tryParseUint64Arg(argv[i + 5], depositRaw, "depositAmountForDispute")) return;
            const char* opIdIdentity = argv[i + 6];

            i += 7;
            if (!hasNoExtraParameters(i, argc, "proposalvote"))
                return;

            qtryProposalVote(nodeIp, nodePort, seed, scheduledTickOffset,
                opFee, shFee, burnFee,
                static_cast<int64_t>(feePerDayRaw),
                static_cast<int64_t>(depositRaw),
                opIdIdentity);
            return;
        }

        if (strcmp(argv[i], "getapprovedamount") == 0)
        {
            if (!hasRequiredParameters(i, argc, 1, "getapprovedamount"))
                return;

            const char* identity = argv[i + 1];
            i += 2;
            if (!hasNoExtraParameters(i, argc, "getapprovedamount"))
                return;

            quotteryGetApprovedAmount(nodeIp, nodePort, identity);
            return;
        }

        if (strcmp(argv[i], "gettopproposals") == 0)
        {
            if (!hasNoExtraParameters(i + 1, argc, "gettopproposals"))
                return;

            quotteryGetTopProposals(nodeIp, nodePort);
            return;
        }

        if (strcmp(argv[i], "transferqusd") == 0)
        {
            if (!hasRequiredParameters(i, argc, 2, "transferqusd"))
                return;

            const char* receiverIdentity = argv[i + 1];
            uint64_t amount = 0;
            if (!tryParseUint64Arg(argv[i + 2], amount, "amount"))
                return;

            i += 3;
            if (!hasNoExtraParameters(i, argc, "transferqusd"))
                return;

            qtryTransferQUSD(nodeIp, nodePort, seed, receiverIdentity,
                static_cast<int64_t>(amount), scheduledTickOffset);
            return;
        }

        ++i;
    }

    LOG("Error: no valid Quottery command provided\n");
}