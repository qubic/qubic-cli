#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <cinttypes>

#include "structs.h"
#include "walletUtils.h"
#include "keyUtils.h"
#include "assetUtil.h"
#include "connection.h"
#include "logger.h"
#include "nodeUtils.h"
#include "K12AndKeyUtil.h"
#include "nostromo.h"

#define NOSTROMO_CONTRACT_INDEX 14

// NOSTROMO FUNCTIONS

#define NOSTROMO_TYPE_GET_STATS 1
#define NOSTROMO_TYPE_GET_TIER_LEVEL_BY_USER 2
#define NOSTROMO_TYPE_GET_USER_VOTE_STATUS 3
#define NOSTROMO_TYPE_CHECK_TOKEN_CREATABILITY 4
#define NOSTROMO_TYPE_GET_NUMBER_OF_INVESTED_AND_CLAIMED_PROJECTS 5
#define NOSTROMO_TYPE_GET_PROJECT_BY_INDEX 6
#define NOSTROMO_TYPE_GET_FUNDRAISING_BY_INDEX 7
#define NOSTROMO_TYPE_GET_PROJECT_INDEX_LIST_BY_CREATOR 8
#define NOSTROMO_TYPE_GET_INFO_USER_INVESTED 9
#define NOSTROMO_TYPE_GET_MAX_CLAIM_AMOUNT 10

// NOSTROMO PROCEDURES

#define NOSTROMO_TYPE_REGISTER_IN_TIER 1
#define NOSTROMO_TYPE_LOGOUT_FROM_TIER 2
#define NOSTROMO_TYPE_CREATE_PROJECT 3
#define NOSTROMO_TYPE_VOTE_IN_PROJECT 4
#define NOSTROMO_TYPE_CREATE_FUNDRAISING 5
#define NOSTROMO_TYPE_INVEST_IN_FUNDRAISING 6
#define NOSTROMO_TYPE_CLAIM_TOKEN 7
#define NOSTROMO_TYPE_UPGRADE_TIER 8
#define NOSTROMO_TRANSFER_SHARE_MANAGEMENT_RIGHTS 9

constexpr uint32_t NOSTROMO_CREATE_PROJECT_FEE = 100000000;
constexpr uint64_t NOSTROMO_TIER_FACEHUGGER_STAKE_AMOUNT = 20000000ULL;
constexpr uint64_t NOSTROMO_TIER_CHESTBURST_STAKE_AMOUNT = 100000000ULL;
constexpr uint64_t NOSTROMO_TIER_DOG_STAKE_AMOUNT = 200000000ULL;
constexpr uint64_t NOSTROMO_TIER_XENOMORPH_STAKE_AMOUNT = 800000000ULL;
constexpr uint64_t NOSTROMO_TIER_WARRIOR_STAKE_AMOUNT = 3200000000ULL;
constexpr uint64_t NOSTROMO_QX_TOKEN_ISSUANCE_FEE = 1000000000ULL;
constexpr uint32_t NOSTROMO_SHARE_MANAGEMENT_TRANSFER_FEE = 100;
constexpr uint32_t NOSTROMO_MAX_NUMBER_PROJECT = 262144;

inline static uint32_t GetYear(uint32_t data)
{
    return ((data >> 26) + 24);
}
inline static uint32_t GetMonth(uint32_t data)
{
    return ((data >> 22) & 0b1111);
}
inline static uint32_t GetDay(uint32_t data)
{
    return ((data >> 17) & 0b11111);
}
inline static uint32_t GetHour(uint32_t data)
{
    return ((data >> 12) & 0b11111);
}
inline static uint32_t GetMinute(uint32_t data)
{
    return ((data >> 6) & 0b111111);
}
inline static uint32_t GetSecond(uint32_t data)
{
    return (data & 0b111111);
}

struct registerInTier_input
{
    uint32_t tierLevel;
};

struct registerInTier_output
{
    uint32_t tierLevel;
};

struct logoutFromTier_input
{

};

struct logoutFromTier_output
{
    bool result;
};

struct createProject_input
{
    uint64_t tokenName;
    uint64_t supply;
    uint32_t startYear;
    uint32_t startMonth;
    uint32_t startDay;
    uint32_t startHour;
    uint32_t endYear;
    uint32_t endMonth;
    uint32_t endDay;
    uint32_t endHour;
};

struct createProject_output
{
    uint32_t indexOfProject;
};

struct voteInProject_input
{
    uint32_t indexOfProject;
    bool decision;
};

struct voteInProject_output
{

};

struct createFundraising_input
{
    uint64_t tokenPrice;
    uint64_t soldAmount;
    uint64_t requiredFunds;

    uint32_t indexOfProject;
    uint32_t firstPhaseStartYear;
    uint32_t firstPhaseStartMonth;
    uint32_t firstPhaseStartDay;
    uint32_t firstPhaseStartHour;
    uint32_t firstPhaseEndYear;
    uint32_t firstPhaseEndMonth;
    uint32_t firstPhaseEndDay;
    uint32_t firstPhaseEndHour;

    uint32_t secondPhaseStartYear;
    uint32_t secondPhaseStartMonth;
    uint32_t secondPhaseStartDay;
    uint32_t secondPhaseStartHour;
    uint32_t secondPhaseEndYear;
    uint32_t secondPhaseEndMonth;
    uint32_t secondPhaseEndDay;
    uint32_t secondPhaseEndHour;

    uint32_t thirdPhaseStartYear;
    uint32_t thirdPhaseStartMonth;
    uint32_t thirdPhaseStartDay;
    uint32_t thirdPhaseStartHour;
    uint32_t thirdPhaseEndYear;
    uint32_t thirdPhaseEndMonth;
    uint32_t thirdPhaseEndDay;
    uint32_t thirdPhaseEndHour;

    uint32_t listingStartYear;
    uint32_t listingStartMonth;
    uint32_t listingStartDay;
    uint32_t listingStartHour;

    uint32_t cliffEndYear;
    uint32_t cliffEndMonth;
    uint32_t cliffEndDay;
    uint32_t cliffEndHour;

    uint32_t vestingEndYear;
    uint32_t vestingEndMonth;
    uint32_t vestingEndDay;
    uint32_t vestingEndHour;

    uint8_t threshold;
    uint8_t TGE;
    uint8_t stepOfVesting;
};

struct createFundraising_output
{

};

struct investInProject_input
{
    uint32_t indexOfFundraising;
};

struct investInProject_output
{

};

struct claimToken_input
{
    uint64_t amount;
    uint32_t indexOfFundraising;
};

struct claimToken_output
{

};

struct upgradeTier_input
{
    uint32_t newTierLevel;
};

struct upgradeTier_output
{

};

struct Asset
{
    uint8_t issuer[32];
    uint64_t assetName;
};

struct nostromoTransferShareManagementRights_input
{
    Asset asset;
	int64_t numberOfShares;
	uint32_t newManagingContractIndex;
};
struct nostromoTransferShareManagementRights_output
{
    int64_t transferredNumberOfShares;
};


void registerInTier(const char* nodeIp, int nodePort,
                    const char* seed, 
                    uint32_t scheduledTickOffset, 
                    uint32_t tierLevel)
{
    if (tierLevel < 1 || tierLevel > 5)
    {
        printf("TierLevel should be the number between 1 ~ 5!\n");
        return ;
    }
    
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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        registerInTier_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.tierLevel = tierLevel;

    switch (tierLevel)
    {
    case 1:
        packet.transaction.amount = NOSTROMO_TIER_FACEHUGGER_STAKE_AMOUNT;
        break;
    case 2:
        packet.transaction.amount = NOSTROMO_TIER_CHESTBURST_STAKE_AMOUNT;
        break;
    case 3:
        packet.transaction.amount = NOSTROMO_TIER_DOG_STAKE_AMOUNT;
        break;
    case 4:
        packet.transaction.amount = NOSTROMO_TIER_XENOMORPH_STAKE_AMOUNT;
        break;
    case 5:
        packet.transaction.amount = NOSTROMO_TIER_WARRIOR_STAKE_AMOUNT;
        break;
    default:
        break;
    }
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TYPE_REGISTER_IN_TIER;
    packet.transaction.inputSize = sizeof(registerInTier_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(registerInTier_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(registerInTier_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("registerInTier tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void logoutFromTier(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset)
{
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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        logoutFromTier_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TYPE_LOGOUT_FROM_TIER;
    packet.transaction.inputSize = sizeof(logoutFromTier_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(logoutFromTier_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(logoutFromTier_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("logoutFromTier tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void createProject(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    const char* tokenName,
                    uint64_t supply,
                    uint32_t startYear,
                    uint32_t startMonth,
                    uint32_t startDay,
                    uint32_t startHour,
                    uint32_t endYear,
                    uint32_t endMonth,
                    uint32_t endDay,
                    uint32_t endHour)
{
    auto qc = make_qc(nodeIp, nodePort);

    char assetNameS1[8] = {0};
    memcpy(assetNameS1, tokenName, strlen(tokenName));

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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        createProject_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    memcpy(&packet.input.tokenName, assetNameS1, 8);
    packet.input.supply = supply;
    packet.input.startYear = startYear - 2000;
    packet.input.startMonth = startMonth;
    packet.input.startDay = startDay;
    packet.input.startHour = startHour;
    packet.input.endYear = endYear - 2000;
    packet.input.endMonth = endMonth;
    packet.input.endDay = endDay;
    packet.input.endHour = endHour;

    packet.transaction.amount = NOSTROMO_CREATE_PROJECT_FEE;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TYPE_CREATE_PROJECT;
    packet.transaction.inputSize = sizeof(createProject_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(createProject_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(createProject_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("createProject tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void voteInProject(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint32_t indexOfProject,
		            bool decision)
{
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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        voteInProject_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.decision = decision;
    packet.input.indexOfProject = indexOfProject;

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TYPE_VOTE_IN_PROJECT;
    packet.transaction.inputSize = sizeof(voteInProject_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(voteInProject_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(voteInProject_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("voteInProject tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void createFundraising(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint64_t tokenPrice,
                    uint64_t soldAmount,
                    uint64_t requiredFunds,

                    uint32_t indexOfProject,
                    uint32_t firstPhaseStartYear,
                    uint32_t firstPhaseStartMonth,
                    uint32_t firstPhaseStartDay,
                    uint32_t firstPhaseStartHour,
                    uint32_t firstPhaseEndYear,
                    uint32_t firstPhaseEndMonth,
                    uint32_t firstPhaseEndDay,
                    uint32_t firstPhaseEndHour,

                    uint32_t secondPhaseStartYear,
                    uint32_t secondPhaseStartMonth,
                    uint32_t secondPhaseStartDay,
                    uint32_t secondPhaseStartHour,
                    uint32_t secondPhaseEndYear,
                    uint32_t secondPhaseEndMonth,
                    uint32_t secondPhaseEndDay,
                    uint32_t secondPhaseEndHour,

                    uint32_t thirdPhaseStartYear,
                    uint32_t thirdPhaseStartMonth,
                    uint32_t thirdPhaseStartDay,
                    uint32_t thirdPhaseStartHour,
                    uint32_t thirdPhaseEndYear,
                    uint32_t thirdPhaseEndMonth,
                    uint32_t thirdPhaseEndDay,
                    uint32_t thirdPhaseEndHour,
                    uint32_t listingStartYear,
                    uint32_t listingStartMonth,
                    uint32_t listingStartDay,
                    uint32_t listingStartHour,

                    uint32_t cliffEndYear,
                    uint32_t cliffEndMonth,
                    uint32_t cliffEndDay,
                    uint32_t cliffEndHour,

                    uint32_t vestingEndYear,
                    uint32_t vestingEndMonth,
                    uint32_t vestingEndDay,
                    uint32_t vestingEndHour,

                    uint8_t threshold,
                    uint8_t TGE,
                    uint8_t stepOfVesting)
{
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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        createFundraising_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.tokenPrice = tokenPrice;
    packet.input.soldAmount = soldAmount;
    packet.input.requiredFunds = requiredFunds;

    packet.input.indexOfProject = indexOfProject;
    packet.input.firstPhaseStartYear = firstPhaseStartYear - 2000;
    packet.input.firstPhaseStartMonth = firstPhaseStartMonth;
    packet.input.firstPhaseStartDay = firstPhaseStartDay;
    packet.input.firstPhaseStartHour = firstPhaseStartHour;
    packet.input.firstPhaseEndYear = firstPhaseEndYear - 2000;
    packet.input.firstPhaseEndMonth = firstPhaseEndMonth;
    packet.input.firstPhaseEndDay = firstPhaseEndDay;
    packet.input.firstPhaseEndHour = firstPhaseEndHour;

    packet.input.secondPhaseStartYear = secondPhaseStartYear - 2000;
    packet.input.secondPhaseStartMonth = secondPhaseStartMonth;
    packet.input.secondPhaseStartDay = secondPhaseStartDay;
    packet.input.secondPhaseStartHour = secondPhaseStartHour;
    packet.input.secondPhaseEndYear = secondPhaseEndYear - 2000;
    packet.input.secondPhaseEndMonth = secondPhaseEndMonth;
    packet.input.secondPhaseEndDay = secondPhaseEndDay;
    packet.input.secondPhaseEndHour = secondPhaseEndHour;

    packet.input.thirdPhaseStartYear = thirdPhaseStartYear - 2000;
    packet.input.thirdPhaseStartMonth = thirdPhaseStartMonth;
    packet.input.thirdPhaseStartDay = thirdPhaseStartDay;
    packet.input.thirdPhaseStartHour = thirdPhaseStartHour;
    packet.input.thirdPhaseEndYear = thirdPhaseEndYear - 2000;
    packet.input.thirdPhaseEndMonth = thirdPhaseEndMonth;
    packet.input.thirdPhaseEndDay = thirdPhaseEndDay;
    packet.input.thirdPhaseEndHour = thirdPhaseEndHour;
    packet.input.listingStartYear = listingStartYear - 2000;
    packet.input.listingStartMonth = listingStartMonth;
    packet.input.listingStartDay = listingStartDay;
    packet.input.listingStartHour = listingStartHour;

    packet.input.cliffEndYear = cliffEndYear - 2000;
    packet.input.cliffEndMonth = cliffEndMonth;
    packet.input.cliffEndDay = cliffEndDay;
    packet.input.cliffEndHour = cliffEndHour;

    packet.input.vestingEndYear = vestingEndYear - 2000;
    packet.input.vestingEndMonth = vestingEndMonth;
    packet.input.vestingEndDay = vestingEndDay;
    packet.input.vestingEndHour = vestingEndHour;

    packet.input.threshold = threshold;
    packet.input.TGE = TGE;
    packet.input.stepOfVesting = stepOfVesting;

    packet.transaction.amount = NOSTROMO_QX_TOKEN_ISSUANCE_FEE;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TYPE_CREATE_FUNDRAISING;
    packet.transaction.inputSize = sizeof(createFundraising_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(createFundraising_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(createFundraising_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("createFundraising tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void investInProject(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint32_t indexOfFundraising, uint64_t amount)
{
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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        investInProject_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.indexOfFundraising = indexOfFundraising;

    packet.transaction.amount = amount;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TYPE_INVEST_IN_FUNDRAISING;
    packet.transaction.inputSize = sizeof(investInProject_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(investInProject_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(investInProject_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("investInProject tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void claimToken(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint64_t amount,
		            uint32_t indexOfFundraising)
{
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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        claimToken_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.amount = amount;
    packet.input.indexOfFundraising = indexOfFundraising;

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TYPE_CLAIM_TOKEN;
    packet.transaction.inputSize = sizeof(claimToken_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(claimToken_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(claimToken_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("claimToken tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}


void upgradeTierLevel(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint32_t tierLevel)
{

    if (tierLevel < 2 || tierLevel > 5)
    {
        printf("Wrong Tierlevel for upgrading!\n");
        return ;
    }
    
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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        upgradeTier_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.newTierLevel = tierLevel;
    
    switch (tierLevel)
    {
    case 2:
        packet.transaction.amount = NOSTROMO_TIER_CHESTBURST_STAKE_AMOUNT - NOSTROMO_TIER_FACEHUGGER_STAKE_AMOUNT;
        break;
    case 3:
        packet.transaction.amount = NOSTROMO_TIER_DOG_STAKE_AMOUNT - NOSTROMO_TIER_CHESTBURST_STAKE_AMOUNT;
        break;
    case 4:
        packet.transaction.amount = NOSTROMO_TIER_XENOMORPH_STAKE_AMOUNT - NOSTROMO_TIER_DOG_STAKE_AMOUNT;
        break;
    case 5:
        packet.transaction.amount = NOSTROMO_TIER_WARRIOR_STAKE_AMOUNT - NOSTROMO_TIER_XENOMORPH_STAKE_AMOUNT;
        break;
    default:
        break;
    }

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TYPE_UPGRADE_TIER;
    packet.transaction.inputSize = sizeof(upgradeTier_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(upgradeTier_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(upgradeTier_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("upgradeTierLevel tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void nostromoTransferShareManagementRights(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset, 
                    const char* issuer, 
                    const char* assetName, 
                    int64_t numberOfShares, 
                    uint32_t newManagingContractIndex)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t pubKey[32] = {0};
    char assetNameS1[8] = {0};
    memcpy(assetNameS1, assetName, strlen(assetName));
    getPublicKeyFromIdentity(issuer, pubKey);

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
    ((uint64_t*)destPublicKey)[0] = NOSTROMO_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        nostromoTransferShareManagementRights_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    memcpy(&packet.input.asset.assetName, assetNameS1, 8);
    memcpy(packet.input.asset.issuer, pubKey, 32);
    packet.input.newManagingContractIndex = newManagingContractIndex;
    packet.input.numberOfShares = numberOfShares;
    
    packet.transaction.amount = NOSTROMO_SHARE_MANAGEMENT_TRANSFER_FEE;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = NOSTROMO_TRANSFER_SHARE_MANAGEMENT_RIGHTS;
    packet.transaction.inputSize = sizeof(nostromoTransferShareManagementRights_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(nostromoTransferShareManagementRights_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(nostromoTransferShareManagementRights_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("nostromoTransferShareManagementRights tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void getStats(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    #pragma pack(pop)

    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = NOSTROMO_TYPE_GET_STATS;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetStats_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetStats_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("The Epoch Revenue: %" PRIu64 "\nThe Total Pool Weight: %" PRIu64 "\nThe number of Register: %u\nThe number of created project: %u\nThe number of fundraising: %u\n", result.epochRevenue, result.totalPoolWeight, result.numberOfRegister, result.numberOfCreatedProject, result.numberOfFundraising);
}

void getTierLevelByUser(const char* nodeIp, int nodePort,
                    const char* userId)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(userId, publicKey);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOGetTierLevelByUser_input input;
    } packet;
    #pragma pack(pop)

    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOGetTierLevelByUser_input);
    packet.rcf.inputType = NOSTROMO_TYPE_GET_TIER_LEVEL_BY_USER;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;
    
    memcpy(packet.input.userId, publicKey, 32);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetTierLevelByUser_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetTierLevelByUser_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("The tierLevel: %u\n", result.tierLevel);
}

void getUserVoteStatus(const char* nodeIp, int nodePort,
                    const char* userId)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(userId, publicKey);
    
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOGetUserVoteStatus_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOGetUserVoteStatus_input);
    packet.rcf.inputType = NOSTROMO_TYPE_GET_USER_VOTE_STATUS;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;
    
    memcpy(packet.input.userId, publicKey, 32);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetUserVoteStatus_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetUserVoteStatus_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    if (result.numberOfVotedProjects == 0)
    {
        printf("There is no voted project.\n");
    }
    else
    {
        printf("The index of voted project\n");
        for (uint32_t i = 0; i < result.numberOfVotedProjects - 1; i++)
        {
            printf("%u, ", result.projectIndexList[i]);
        }
        printf("%u", result.projectIndexList[result.numberOfVotedProjects - 1]);
    }
}

void checkTokenCreatability(const char* nodeIp, int nodePort,
                    const char* tokenName)
{
    auto qc = make_qc(nodeIp, nodePort);

    char assetNameS1[8] = {0};
    memcpy(assetNameS1, tokenName, strlen(tokenName));
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOCheckTokenCreatability_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOCheckTokenCreatability_input);
    packet.rcf.inputType = NOSTROMO_TYPE_CHECK_TOKEN_CREATABILITY;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;
    memcpy(&packet.input.tokenName, assetNameS1, 8);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOCheckTokenCreatability_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOCheckTokenCreatability_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }
    if (result.result)
    {
        printf("This token was already created by NOSTROMO. please choose another token name.\n");
    }
    else
    {
        printf("You can issue this token\n");
    }
}

void getNumberOfInvestedProjects(const char* nodeIp, int nodePort,
                    const char* userId)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(userId, publicKey);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOGetNumberOfInvestedProjects_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOGetNumberOfInvestedProjects_input);
    packet.rcf.inputType = NOSTROMO_TYPE_GET_NUMBER_OF_INVESTED_AND_CLAIMED_PROJECTS;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;
    
    memcpy(packet.input.userId, publicKey, 32);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetNumberOfInvestedProjects_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetNumberOfInvestedProjects_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("The number Of invested project: %u\n", result.numberOfInvestedProjects);
}

void getProjectByIndex(const char* nodeIp, int nodePort,
                    uint32_t indexOfProject)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOGetProjectByIndex_input input;
    } packet;
    #pragma pack(pop)

    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOGetProjectByIndex_input);
    packet.rcf.inputType = NOSTROMO_TYPE_GET_PROJECT_BY_INDEX;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;
    packet.input.indexOfProject = indexOfProject;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetProjectByIndex_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetProjectByIndex_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char creator[128] = {0};
    getIdentityFromPublicKey(result.project.creator, creator, false);

    char tokenName[9] = {0};
    std::memcpy(tokenName, &result.project.tokenName, 8);

    printf("The creator of project is %s\n", creator);
    printf("The token name: %s\n", tokenName);
    printf("The supply of token: %" PRIu64 "\n", result.project.supplyOfToken);
    printf("The start date: %u.%u.%u %u\n", GetYear(result.project.startDate) + 2000, GetMonth(result.project.startDate), GetDay(result.project.startDate), GetHour(result.project.startDate));
    printf("The end date: %u.%u.%u %u\n", GetYear(result.project.endDate) + 2000, GetMonth(result.project.endDate), GetDay(result.project.endDate), GetHour(result.project.endDate));
    printf("The number of yes: %u\n", result.project.numberOfYes);
    printf("The number of no: %u\n", result.project.numberOfNo);
    if (result.project.isCreatedFundarasing)
    {
        printf("This project created the fundraising.\n");
    }
    else
    {
        printf("This project does not create the fundraising\n");
    }
}

void getFundarasingByIndex(const char* nodeIp, int nodePort,
                    uint32_t indexOfFundarasing)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOGetFundarasingByIndex_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOGetFundarasingByIndex_input);
    packet.rcf.inputType = NOSTROMO_TYPE_GET_FUNDRAISING_BY_INDEX;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;
    packet.input.indexOfFundarasing = indexOfFundarasing;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetFundarasingByIndex_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetFundarasingByIndex_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }
    printf("The token price: %" PRIu64 "\n", result.fundarasing.tokenPrice);
    printf("The sold amount: %" PRIu64 "\n", result.fundarasing.soldAmount);
    printf("The required funds: %" PRIu64 "\n", result.fundarasing.requiredFunds);
    printf("The raised funds: %" PRIu64 "\n", result.fundarasing.raisedFunds);
    printf("The index of project: %u\n", result.fundarasing.indexOfProject);
    printf("The first phase start date: %u.%u.%u %u\n", GetYear(result.fundarasing.firstPhaseStartDate) + 2000, GetMonth(result.fundarasing.firstPhaseStartDate), GetDay(result.fundarasing.firstPhaseStartDate), GetHour(result.fundarasing.firstPhaseStartDate));
    printf("The first phase end date: %u.%u.%u %u\n", GetYear(result.fundarasing.firstPhaseEndDate) + 2000, GetMonth(result.fundarasing.firstPhaseEndDate), GetDay(result.fundarasing.firstPhaseEndDate), GetHour(result.fundarasing.firstPhaseEndDate));
    printf("The second phase start date: %u.%u.%u %u\n", GetYear(result.fundarasing.secondPhaseStartDate) + 2000, GetMonth(result.fundarasing.secondPhaseStartDate), GetDay(result.fundarasing.secondPhaseStartDate), GetHour(result.fundarasing.secondPhaseStartDate));
    printf("The second phase end date: %u.%u.%u %u\n", GetYear(result.fundarasing.secondPhaseEndDate) + 2000, GetMonth(result.fundarasing.secondPhaseEndDate), GetDay(result.fundarasing.secondPhaseEndDate), GetHour(result.fundarasing.secondPhaseEndDate));
    printf("The third phase start date: %u.%u.%u %u\n", GetYear(result.fundarasing.thirdPhaseStartDate) + 2000, GetMonth(result.fundarasing.thirdPhaseStartDate), GetDay(result.fundarasing.thirdPhaseStartDate), GetHour(result.fundarasing.thirdPhaseStartDate));
    printf("The third phase end date: %u.%u.%u %u\n", GetYear(result.fundarasing.thirdPhaseEndDate) + 2000, GetMonth(result.fundarasing.thirdPhaseEndDate), GetDay(result.fundarasing.thirdPhaseEndDate), GetHour(result.fundarasing.thirdPhaseEndDate));
    printf("The listing start date: %u.%u.%u %u\n", GetYear(result.fundarasing.listingStartDate) + 2000, GetMonth(result.fundarasing.listingStartDate), GetDay(result.fundarasing.listingStartDate), GetHour(result.fundarasing.listingStartDate));
    printf("The cliff end date: %u.%u.%u %u\n", GetYear(result.fundarasing.cliffEndDate) + 2000, GetMonth(result.fundarasing.cliffEndDate), GetDay(result.fundarasing.cliffEndDate), GetHour(result.fundarasing.cliffEndDate));
    printf("The vesting end date: %u.%u.%u %u\n", GetYear(result.fundarasing.vestingEndDate) + 2000, GetMonth(result.fundarasing.vestingEndDate), GetDay(result.fundarasing.vestingEndDate), GetHour(result.fundarasing.vestingEndDate));
    printf("The thresholds: %u\n", result.fundarasing.threshold);
    printf("The TGE: %u\n", result.fundarasing.TGE);
    printf("The step of vesting: %u\n", result.fundarasing.stepOfVesting);
    if (result.fundarasing.isCreatedToken)
    {
        printf("This fundarasing was already rasied the enough funds to pass.\n");
    }
    else
    {
        printf("Thif fundarasing is not yet raised the enough funds to pass.\n");
    }
}

void getProjectIndexListByCreator(const char* nodeIp, int nodePort, 
                    const char* creator)
{
    auto qc = make_qc(nodeIp, nodePort);
   
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(creator, publicKey);

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOGetProjectIndexListByCreator_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOGetProjectIndexListByCreator_input);
    packet.rcf.inputType = NOSTROMO_TYPE_GET_PROJECT_INDEX_LIST_BY_CREATOR;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;

    memcpy(packet.input.creator, publicKey, 32);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetProjectIndexListByCreator_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetProjectIndexListByCreator_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    if (result.indexListForProjects[0] == NOSTROMO_MAX_NUMBER_PROJECT)
    {
        printf("There are no projects created by %s\n", creator);
    }
    
    for (uint32_t i = 0; i < 128; i++)
    {
        if (i < 127 && result.indexListForProjects[i + 1] == NOSTROMO_MAX_NUMBER_PROJECT)
        {
            printf("%u\n", result.indexListForProjects[i]);
            break;
        }
        printf("%u, ", result.indexListForProjects[i]);
    }
}

void getInfoUserInvested(const char* nodeIp, int nodePort,
                    const char* invsetorId)
{
    auto qc = make_qc(nodeIp, nodePort);
   
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(invsetorId, publicKey);

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOGetInfoUserInvested_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOGetInfoUserInvested_input);
    packet.rcf.inputType = NOSTROMO_TYPE_GET_INFO_USER_INVESTED;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;

    memcpy(packet.input.investorId, publicKey, 32);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetInfoUserInvested_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetInfoUserInvested_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }
    for (uint32_t i = 0; i < 128; i++)
    {
        if (result.listUserInvested[i].investedAmount == 0 && result.listUserInvested[i].claimedAmount == 0)
        {
            if (i == 0)
            {
                printf("There are no projects invested by %s\n", invsetorId);
            }
            break;
        }
        printf("IndexOfFundraising: %u\nInvestedAmount: %" PRIu64 "\nClaimedAmount: %" PRIu64 "\n\n", result.listUserInvested[i].indexOfFundraising, result.listUserInvested[i].investedAmount, result.listUserInvested[i].claimedAmount);
    }
}

void getMaxClaimAmount(const char* nodeIp, int nodePort,
                    const char* invsetorId, 
                    uint32_t indexOfFundarasing)
{
    auto qc = make_qc(nodeIp, nodePort);
   
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(invsetorId, publicKey);

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        NOSTROMOGetMaxClaimAmount_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(NOSTROMOGetMaxClaimAmount_input);
    packet.rcf.inputType = NOSTROMO_TYPE_GET_MAX_CLAIM_AMOUNT;
    packet.rcf.contractIndex = NOSTROMO_CONTRACT_INDEX;

    memcpy(packet.input.investorId, publicKey, 32);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    NOSTROMOGetMaxClaimAmount_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<NOSTROMOGetMaxClaimAmount_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }
    printf("Max Claim Amount: %" PRIu64 "\n", result.amount);
}