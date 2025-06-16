#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "structs.h"
#include "walletUtils.h"
#include "keyUtils.h"
#include "assetUtil.h"
#include "connection.h"
#include "logger.h"
#include "nodeUtils.h"
#include "K12AndKeyUtil.h"
#include "qvault.h"

#define QVAULT_CONTRACT_INDEX 10
#define QVAULT_PROPOSAL_CREATION_FEE 10000000
#define QVAULT_SHARE_MANAGEMENT_TRANSFER_FEE 100

// QVAULT FUNCTIONS
#define QVAULT_GETDATA 1
#define QVAULT_GET_STAKED_AMOUNT_AND_VOTING_POWER 2
#define QVAULT_GET_GP 3
#define QVAULT_GET_QCP 4
#define QVAULT_GET_IPOP 5
#define QVAULT_GET_QEARNP 6
#define QVAULT_GET_FUNDP 7
#define QVAULT_GET_MKTP 8
#define QVAULT_GET_ALLOP 9
#define QVAULT_GET_IDENTITIES_HV_VT_PW 11
#define QVAULT_GET_PP_CREATION_POWER 12
#define QVAULT_GET_QCAP_BURNT_AMOUNT_IN_LAST_EPOCHES 13
#define QVAULT_GET_AMOUNT_TO_BE_SOLD_PER_YEAR 14
#define QVAULT_GET_TOTAL_REVENUE_IN_QCAP 15
#define QVAULT_GET_REVENUE_IN_QCAP_PER_EPOCH 16
#define QVAULT_GET_REVENUE_PER_SHARE 17
#define QVAULT_GET_AMOUNT_OF_SHARE_QVAULT_HOLD 18
#define QVAULT_GET_NUMBER_OF_HOLDER_AND_AVG_AM 19
#define QVAULT_GET_AMOUNT_FOR_QEARN_IN_UPCOMING_EPOCH 20

// QVAULT PROCEDURES
#define QVAULT_STAKE 1
#define QVAULT_UNSTAKE 2
#define QVAULT_SUBMIT_GP 3
#define QVAULT_SUBMIT_QCP 4
#define QVAULT_SUBMIT_IPOP 5
#define QVAULT_SUBMIT_QEARNP 6
#define QVAULT_SUBMIT_FUNDP 7
#define QVAULT_SUBMIT_MKTP 8
#define QVAULT_SUBMIT_ALLOP 9
#define QVAULT_VOTE_IN_PROPOSAL 11
#define QVAULT_BUY_QCAP 12
#define QVAULT_TRANSFER_SHARE_MANAGEMENT_RIGHTS 13

struct stake_input
{
    uint32_t amount;
};

struct stake_output
{
    uint32_t returnCode;
};

struct unStake_input
{
    uint32_t amount;
};

struct unStake_output
{
    uint32_t returnCode;
};

struct submitGP_input
{
    uint8_t url[256];
};

struct submitGP_output
{
    uint32_t returnCode;
};

struct submitQCP_input
{
    uint32_t newQuorumPercent;
    uint8_t url[256];
};

struct submitQCP_output
{
    uint32_t returnCode;
};

struct submitIPOP_input
{
    uint32_t ipoContractIndex;
    uint8_t url[256];
};

struct submitIPOP_output
{
    uint32_t returnCode;
};

struct submitQEarnP_input
{
    uint64_t amountPerEpoch;
    uint32_t numberOfEpoch;
    uint8_t url[256];
};

struct submitQEarnP_output
{
    uint32_t returnCode;
};

struct submitFundP_input
{
    uint64_t priceOfOneQcap;
    uint32_t amountOfQcap;
    uint8_t url[256];
};

struct submitFundP_output
{
    uint32_t returnCode;
};

struct submitMKTP_input
{
    uint64_t amountOfQubic;
    uint64_t shareName;
    uint32_t amountOfQcap;
    uint32_t indexOfShare;
    uint32_t amountOfShare;
    uint8_t url[256];
};

struct submitMKTP_output
{
    uint32_t returnCode;
};

struct submitAlloP_input
{
    uint32_t reinvested;
    uint32_t team;
    uint32_t burn;
    uint32_t distribute;
    uint8_t url[256];
};

struct submitAlloP_output
{
    uint32_t returnCode;
};

struct voteInProposal_input
{
    uint64_t priceOfIPO;
    uint32_t proposalType;
    uint32_t proposalId;
    bool yes;
};

struct voteInProposal_output
{
    uint32_t returnCode;
};

struct buyQcap_input
{
    uint32_t amount;
};

struct buyQcap_output
{
    uint32_t returnCode;
};

struct TransferShareManagementRights_input
{
    qpiAsset asset;
    int64_t numberOfShares;
    uint32_t newManagingContractIndex;
};
struct TransferShareManagementRights_output
{
    int64_t transferredNumberOfShares;
    uint32_t returnCode;
};

void stake(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount)
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        stake_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.amount = amount;
    packet.transaction.amount = 0;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_STAKE;
    packet.transaction.inputSize = sizeof(stake_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(stake_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(stake_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("stake tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void unStake(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount)
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        unStake_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.amount = amount;
    packet.transaction.amount = 0;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_UNSTAKE;
    packet.transaction.inputSize = sizeof(unStake_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(unStake_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(unStake_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("unStake tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitGP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* url)
{
    if (strlen(url) > 255)
    {
        printf("The url should be less than 255.\nThe command is failed"); 
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitGP_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.transaction.amount = QVAULT_PROPOSAL_CREATION_FEE;
    memcpy(&packet.input.url, url, 256);

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMIT_GP;
    packet.transaction.inputSize = sizeof(submitGP_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitGP_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitGP_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitGP tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitQCP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t newQuorumPercent, const char* url)
{
    if (strlen(url) > 255)
    {
        printf("The url should be less than 255.\nThe command is failed"); 
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitQCP_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.newQuorumPercent = newQuorumPercent;
    memcpy(&packet.input.url, url, 256);
    packet.transaction.amount = QVAULT_PROPOSAL_CREATION_FEE;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMIT_QCP;
    packet.transaction.inputSize = sizeof(submitQCP_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitQCP_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitQCP_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitQCP tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitIPOP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t ipoContractIndex, const char* url)
{
    if (strlen(url) > 255)
    {
        printf("The url should be less than 255.\nThe command is failed"); 
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitIPOP_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.ipoContractIndex = ipoContractIndex;
    memcpy(&packet.input.url, url, 256);
    packet.transaction.amount = QVAULT_PROPOSAL_CREATION_FEE;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMIT_IPOP;
    packet.transaction.inputSize = sizeof(submitIPOP_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitIPOP_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitIPOP_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitIPOP tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitQEarnP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t amountPerEpoch, uint32_t numberOfEpoch, const char* url)
{
    if (strlen(url) > 255)
    {
        printf("The url should be less than 255.\nThe command is failed"); 
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitQEarnP_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.amountPerEpoch = amountPerEpoch;
    packet.input.numberOfEpoch = numberOfEpoch;
    memcpy(&packet.input.url, url, 256);
    packet.transaction.amount = 0;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMIT_QEARNP;
    packet.transaction.inputSize = sizeof(submitQEarnP_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitQEarnP_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitQEarnP_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitQEarnP tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitFundP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t priceOfOneQcap, uint32_t amountOfQcap, const char* url)
{
    if (strlen(url) > 255)
    {
        printf("The url should be less than 255.\nThe command is failed"); 
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitFundP_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.amountOfQcap = amountOfQcap;
    packet.input.priceOfOneQcap = priceOfOneQcap;
    memcpy(&packet.input.url, url, 256);

    packet.transaction.amount = QVAULT_PROPOSAL_CREATION_FEE;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMIT_FUNDP;
    packet.transaction.inputSize = sizeof(submitFundP_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitFundP_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitFundP_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitFundP tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitMKTP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t amountOfQubic, const char* shareName, uint32_t amountOfQcap, uint32_t indexOfShare, uint32_t amountOfShare, const char* url)
{
    if (strlen(url) > 255)
    {
        printf("The url should be less than 255.\nThe command is failed"); 
        return ;
    }
    auto qc = make_qc(nodeIp, nodePort);

    char assetNameS1[8] = {0};
    memcpy(assetNameS1, shareName, strlen(shareName));

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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitMKTP_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.amountOfQcap = amountOfQcap;
    packet.input.amountOfQubic = amountOfQubic;
    packet.input.amountOfShare = amountOfShare;
    packet.input.indexOfShare = indexOfShare;
    memcpy(&packet.input.url, url, 256);
    memcpy(&packet.input.shareName, assetNameS1, 8);
    
    packet.transaction.amount = 0;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMIT_MKTP;
    packet.transaction.inputSize = sizeof(submitMKTP_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitMKTP_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitMKTP_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitMKTP tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitAlloP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t reinvested, uint32_t team, uint32_t burn, uint32_t distribute, const char* url)
{
    if (strlen(url) > 255)
    {
        printf("The url should be less than 255.\nThe command is failed"); 
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitAlloP_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.burn = burn;
    packet.input.distribute = distribute;
    packet.input.reinvested = reinvested;
    packet.input.team = team;
    memcpy(&packet.input.url, url, 256);
    
    packet.transaction.amount = QVAULT_PROPOSAL_CREATION_FEE;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMIT_ALLOP;
    packet.transaction.inputSize = sizeof(submitAlloP_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitAlloP_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitAlloP_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitAlloP tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void voteInProposal(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t priceOfIPO, uint32_t proposalType, uint32_t proposalId, bool yes)
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        voteInProposal_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.priceOfIPO = priceOfIPO;
    packet.input.proposalId = proposalId;
    packet.input.proposalType = proposalType;
    packet.input.yes = yes;
    
    packet.transaction.amount = 0;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_VOTE_IN_PROPOSAL;
    packet.transaction.inputSize = sizeof(voteInProposal_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(voteInProposal_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(voteInProposal_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("voteInProposal tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void buyQcap(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount, uint64_t priceOfOneQcap)
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        buyQcap_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    packet.input.amount = amount;
    
    packet.transaction.amount = priceOfOneQcap * amount;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_BUY_QCAP;
    packet.transaction.inputSize = sizeof(buyQcap_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(buyQcap_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(buyQcap_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("buyQcap tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void TransferShareManagementRights(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* issuer, const char* assetName, int64_t numberOfShares, uint32_t newManagingContractIndex)
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
    ((uint64_t*)destPublicKey)[0] = QVAULT_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        Transaction transaction;
        TransferShareManagementRights_input input;
        unsigned char signature[64];
    } packet;
    #pragma pack(pop)

    memcpy(&packet.input.asset.assetName, assetNameS1, 8);
    memcpy(packet.input.asset.issuer, pubKey, 32);
    packet.input.newManagingContractIndex = newManagingContractIndex;
    packet.input.numberOfShares = numberOfShares;
    
    packet.transaction.amount = QVAULT_SHARE_MANAGEMENT_TRANSFER_FEE;

    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_TRANSFER_SHARE_MANAGEMENT_RIGHTS;
    packet.transaction.inputSize = sizeof(TransferShareManagementRights_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(TransferShareManagementRights_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(TransferShareManagementRights_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("TransferShareManagementRights tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void getData(const char* nodeIp, int nodePort)
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
    packet.rcf.inputType = QVAULT_GETDATA;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QVaultGetData_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QVaultGetData_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char adminAddress[128] = {0};
    getIdentityFromPublicKey(result.adminAddress , adminAddress, false);

    printf("adminAddress: %s\n", adminAddress);
    printf("totalVotingPower: %llu\nproposalCreateFund: %llu\nreinvestingFund: %llu\ntotalEpochRevenue: %llu\nfundForBurn: %llu\ntotalStakedQcapAmount: %llu\nqcapMarketCap: %llu\nraisedFundByQcap: %llu\nlastRoundPriceOfQcap: %llu\nrevenueByQearn:%llu\nqcapSoldAmount: %u\nshareholderDividend: %u\nQCAPHolderPermille: %u\nreinvestingPermille: %u\ndevPermille: %u\nburnPermille: %u\nqcapBurnPermille: %u\nnumberOfStaker: %u\nnumberOfVotingPower: %u\nnumberOfGP: %u\nnumberOfQCP: %u\nnumberOfIPOP: %u\nnumberOfQEarnP: %u\nnumberOfFundP: %u\nnumberOfMKTP: %u\nnumberOfAlloP: %u\ntransferRightsFee: %u\nminQuorumRq: %u\nmaxQuorumRq: %u\ntotalQcapBurntAmount: %u\ncirculatingSupply: %u\nquorumPercent: %u\n", result.totalVotingPower, result.proposalCreateFund, result.reinvestingFund, result.totalEpochRevenue, result.fundForBurn, result.totalStakedQcapAmount, result.qcapMarketCap, result.raisedFundByQcap, result.lastRoundPriceOfQcap, result.revenueByQearn, result.qcapSoldAmount, result.shareholderDividend, result.QCAPHolderPermille, result.reinvestingPermille, result.devPermille, result.burnPermille, result.qcapBurnPermille, result.numberOfStaker, result.numberOfVotingPower, result.numberOfGP, result.numberOfQCP, result.numberOfIPOP, result.numberOfQEarnP, result.numberOfFundP, result.numberOfMKTP, result.numberOfAlloP, result.transferRightsFee, result.minQuorumRq, result.maxQuorumRq, result.totalQcapBurntAmount, result.circulatingSupply, result.quorumPercent);
}

void getStakedAmountAndVotingPower(const char* nodeIp, int nodePort, const char* address)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t pubKey[32] = {0};
    getPublicKeyFromIdentity(address, pubKey);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetStakedAmountAndVotingPower_input input;
    } packet;
    #pragma pack(pop)

    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetStakedAmountAndVotingPower_input);
    packet.rcf.inputType = QVAULT_GET_STAKED_AMOUNT_AND_VOTING_POWER;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;

    memcpy(packet.input.address, pubKey, 32);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetStakedAmountAndVotingPower_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetStakedAmountAndVotingPower_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("stakedAmount: %u\nvotingPower: %u\n", result.stakedAmount, result.votingPower);
}

void getGP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetGP_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetGP_input);
    packet.rcf.inputType = QVAULT_GET_GP;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.proposalId = proposalId;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetGP_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetGP_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB") == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\n", result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getQCP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetQCP_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetQCP_input);
    packet.rcf.inputType = QVAULT_GET_QCP;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.proposalId = proposalId;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetQCP_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetQCP_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB") == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\nnewQuorumPercent: %u\n", result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.newQuorumPercent);

    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 3)
    {
        printf("The proposal has been rejected due to another proposal with more yes vote!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getIPOP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetIPOP_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetIPOP_input);
    packet.rcf.inputType = QVAULT_GET_IPOP;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.proposalId = proposalId;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetIPOP_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetIPOP_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB") == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %utotalWeight: %llu\nassignedFund: %llu\nipoContractIndex: %u\n", result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.totalWeight, result.proposal.assignedFund, result.proposal.ipoContractIndex);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getQEarnP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetQEarnP_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetQEarnP_input);
    packet.rcf.inputType = QVAULT_GET_QEARNP;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.proposalId = proposalId;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetQEarnP_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetQEarnP_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB") == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\namountOfInvestPerEpoch: %llu\nassignedFundPerEpoch: %llu\nnumberOfEpoch: %u\n", result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.amountOfInvestPerEpoch, result.proposal.assignedFundPerEpoch, result.proposal.numberOfEpoch);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getFundP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetFundP_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetFundP_input);
    packet.rcf.inputType = QVAULT_GET_FUNDP;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.proposalId = proposalId;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetFundP_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetFundP_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB") == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\npricePerOneQcap: %llu\namountOfQcap: %u\nrestSaleAmount: %u\n", result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.pricePerOneQcap, result.proposal.amountOfQcap, result.proposal.restSaleAmount);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 3)
    {
        printf("The proposal has been rejected due to overflow sale amount per year!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getMKTP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetMKTP_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetMKTP_input);
    packet.rcf.inputType = QVAULT_GET_MKTP;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.proposalId = proposalId;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetMKTP_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetMKTP_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB") == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\namountOfQubic: %llu\nshareName: %llu\namountOfQcap: %u\nshareIndex: %u\namountOfShare: %u\n", result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.amountOfQubic, result.proposal.shareName, result.proposal.amountOfQcap, result.proposal.shareIndex, result.proposal.amountOfShare);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 3)
    {
        printf("The proposal has been rejected due to overflow sale amount per year!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getAlloP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetAlloP_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetAlloP_input);
    packet.rcf.inputType = QVAULT_GET_ALLOP;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.proposalId = proposalId;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetAlloP_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetAlloP_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB") == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\nreinvested: %u\ndistributed: %u\nteam: %u\nburnQcap: %u\n", result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.reinvested, result.proposal.distributed, result.proposal.team, result.proposal.burnQcap);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 3)
    {
        printf("The proposal has been rejected due to another proposal with more yes vote!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getIdentitiesHvVtPw(const char* nodeIp, int nodePort, uint32_t offset, uint32_t count)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetIdentitiesHvVtPw_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetIdentitiesHvVtPw_input);
    packet.rcf.inputType = QVAULT_GET_IDENTITIES_HV_VT_PW;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.count = count;
    packet.input.offset = offset;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetIdentitiesHvVtPw_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetIdentitiesHvVtPw_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    for (uint32_t i = 0; i < count; i++)
    {
        char stakerAddress[128] = {0};
        getIdentityFromPublicKey(result.idList[i], stakerAddress, false);
        if (strcmp(stakerAddress, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB") == 0)
        {
            break;
        }
        printf("stakerAddress: %s amount: %u\n", stakerAddress, result.amountList[i]);
    }
}

void ppCreationPower(const char* nodeIp, int nodePort, const char* address)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t pubKey[32] = {0};
    getPublicKeyFromIdentity(address, pubKey);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultppCreationPower_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultppCreationPower_input);
    packet.rcf.inputType = QVAULT_GET_PP_CREATION_POWER;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    memcpy(packet.input.address, pubKey, 32);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultppCreationPower_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultppCreationPower_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("%s\n", result.status ? "You have a power to create the proposal": "You can not create the proposal");
}

void getQcapBurntAmountInLastEpoches(const char* nodeIp, int nodePort, uint32_t numberOfLastEpoches)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetQcapBurntAmountInLastEpoches_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetQcapBurntAmountInLastEpoches_input);
    packet.rcf.inputType = QVAULT_GET_QCAP_BURNT_AMOUNT_IN_LAST_EPOCHES;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.numberOfLastEpoches = numberOfLastEpoches;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetQcapBurntAmountInLastEpoches_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetQcapBurntAmountInLastEpoches_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("%u\n", result.burntAmount);
}

void getAmountToBeSoldPerYear(const char* nodeIp, int nodePort, uint32_t year)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetAmountToBeSoldPerYear_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetAmountToBeSoldPerYear_input);
    packet.rcf.inputType = QVAULT_GET_AMOUNT_TO_BE_SOLD_PER_YEAR;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.year = year;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetAmountToBeSoldPerYear_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetAmountToBeSoldPerYear_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("%u\n", result.amount);
}

void getTotalRevenueInQcap(const char* nodeIp, int nodePort)
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
    packet.rcf.inputType = QVAULT_GET_TOTAL_REVENUE_IN_QCAP;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetTotalRevenueInQcap_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetTotalRevenueInQcap_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("%llu\n", result.revenue);
}

void getRevenueInQcapPerEpoch(const char* nodeIp, int nodePort, uint32_t epoch)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetRevenueInQcapPerEpoch_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetRevenueInQcapPerEpoch_input);
    packet.rcf.inputType = QVAULT_GET_REVENUE_IN_QCAP_PER_EPOCH;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.epoch = epoch;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetRevenueInQcapPerEpoch_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetRevenueInQcapPerEpoch_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("epochTotalRevenue: %llu\nepochOneQcapRevenue: %llu\nepochOneQvaultRevenue: %llu\nepochReinvestAmount: %llu\n", result.epochTotalRevenue, result.epochOneQcapRevenue, result.epochOneQvaultRevenue, result.epochReinvestAmount);
}

void getRevenuePerShare(const char* nodeIp, int nodePort, uint32_t contractIndex)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetRevenuePerShare_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetRevenuePerShare_input);
    packet.rcf.inputType = QVAULT_GET_REVENUE_PER_SHARE;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.contractIndex = contractIndex;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetRevenuePerShare_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetRevenuePerShare_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("revenue: %llu\n", result.revenue);
}

void getAmountOfShareQvaultHold(const char* nodeIp, int nodePort, const char* assetName, const char* issuer)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint8_t pubKey[32] = {0};
    char assetNameS1[8] = {0};
    memcpy(assetNameS1, assetName, strlen(assetName));
    getPublicKeyFromIdentity(issuer, pubKey);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetAmountOfShareQvaultHold_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetAmountOfShareQvaultHold_input);
    packet.rcf.inputType = QVAULT_GET_AMOUNT_OF_SHARE_QVAULT_HOLD;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    memcpy(packet.input.assetInfo.issuer, pubKey, 32);
    memcpy(&packet.input.assetInfo.assetName, assetNameS1, 8);
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetAmountOfShareQvaultHold_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetAmountOfShareQvaultHold_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("The amount of %s that SC held: %u\n", assetName, result.amount);
}

void getNumberOfHolderAndAvgAm(const char* nodeIp, int nodePort)
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
    packet.rcf.inputType = QVAULT_GET_NUMBER_OF_HOLDER_AND_AVG_AM;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetNumberOfHolderAndAvgAm_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetNumberOfHolderAndAvgAm_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("numberOfQcapHolder: %u\navgAmount: %u\n", result.numberOfQcapHolder, result.avgAmount);
}

void getAmountForQearnInUpcomingEpoch(const char* nodeIp, int nodePort, uint32_t epoch)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    #pragma pack(push, 1)
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QvaultGetAmountForQearnInUpcomingEpoch_input input;
    } packet;
    #pragma pack(pop)
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QvaultGetAmountForQearnInUpcomingEpoch_input);
    packet.rcf.inputType = QVAULT_GET_AMOUNT_FOR_QEARN_IN_UPCOMING_EPOCH;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.epoch = epoch;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QvaultGetAmountForQearnInUpcomingEpoch_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QvaultGetAmountForQearnInUpcomingEpoch_output>();
    }
    catch (std::logic_error)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("%llu\n", result.amount);
}