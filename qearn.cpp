#include <cstdint>
#include <cstring>
#include "structs.h"
#include "walletUtils.h"
#include "keyUtils.h"
#include "assetUtil.h"
#include "connection.h"
#include "logger.h"
#include "nodeUtils.h"
#include "K12AndKeyUtil.h"
#include "qearn.h"

#define QEARN_CONTRACT_INDEX 9

// QEARN FUNCTIONS
#define QEARN_GET_LOCK_INFO_PER_EPOCH 1
#define QEARN_GET_USER_LOCKED_INFO 2
#define QEARN_GET_STATE_OF_ROUND 3
#define QEARN_GET_USER_LOCKED_STATE 4
#define QEARN_GET_UNLOCKED_STATE 5

// QEARN PROCEDURES
#define QEARN_LOCK 1
#define QEARN_UNLOCK 2

struct Unlock_input {
    uint64_t Amount;                            /* unlocking amount */	
    uint32_t Locked_Epoch;                      /* locked epoch */
};

struct Unlock_output {
};

struct GetLockInfoPerEpoch_input {
    uint32_t Epoch;                             /* epoch number to get information */
};

struct GetLockInfoPerEpoch_output {
    uint64_t LockedAmount;                      /* initial total locked amount in epoch */
    uint64_t BonusAmount;                       /* initial bonus amount in epoch*/
    uint64_t CurrentLockedAmount;               /* total locked amount in epoch. exactly the amount excluding the amount unlocked early*/
    uint64_t CurrentBonusAmount;                /* bonus amount in epoch excluding the early unlocking */
    uint64_t Yield;                             /* Yield calculated by 10000000 multiple*/  
};

struct GetUserLockStatus_input {
    uint8_t publicKey[32];
};

struct GetUserLockStatus_output {
    uint64_t status;
};

struct GetStateOfRound_input {
    uint32_t Epoch;
};

struct GetStateOfRound_output {
    uint32_t state;
};

struct GetUserLockedInfo_input {
    uint8_t publicKey[32];
    uint32_t epoch;
};

struct GetUserLockedInfo_output {
    uint64_t LockedAmount;
};

struct GetEndedStatus_input {
    uint8_t publicKey[32];
};

struct GetEndedStatus_output {
    uint64_t Fully_Unlocked_Amount;
    uint64_t Fully_Rewarded_Amount;
    uint64_t Early_Unlocked_Amount;
    uint64_t Early_Rewarded_Amount;
};

void qearnLock(const char* nodeIp, int nodePort, const char* seed, long long lock_amount, uint32_t scheduledTickOffset)
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
    ((uint64_t*)destPublicKey)[0] = QEARN_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        unsigned char signature[64];
    } packet;
    packet.transaction.amount = lock_amount;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QEARN_LOCK;
    packet.transaction.inputSize = 0;
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("LockQubic tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qearnUnlock(const char* nodeIp, int nodePort, const char* seed, long long unlock_amount, uint32_t locked_epoch, uint32_t scheduledTickOffset)
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
    ((uint64_t*)destPublicKey)[0] = QEARN_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        Unlock_input input;
        unsigned char signature[64];
    } packet;
    packet.input.Amount = unlock_amount;
    packet.input.Locked_Epoch = locked_epoch;
    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QEARN_UNLOCK;
    packet.transaction.inputSize = sizeof(Unlock_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(Unlock_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(Unlock_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("UnlockQubic tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void qearnGetInfoPerEpoch(const char* nodeIp, const int nodePort, uint32_t epoch){
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetLockInfoPerEpoch_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(GetLockInfoPerEpoch_input);
    packet.rcf.inputType = QEARN_GET_LOCK_INFO_PER_EPOCH;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX; 
    packet.input.Epoch = epoch;
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;

    GetLockInfoPerEpoch_output result;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            auto output = (GetLockInfoPerEpoch_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *output;
        }
        ptr+= header->size();
    }
    printf("initial bonus amount:  %llu\ninitial locked amount: %llu\ncurrent bonus amount:  %llu\ncurrent locked amount: %llu\nyield: %.6f%%\n", result.BonusAmount, result.LockedAmount, result.CurrentBonusAmount, result.CurrentLockedAmount, double(result.Yield) / 100000.0);
}

void qearnGetUserLockedInfo(const char* nodeIp, const int nodePort, char* Identity, uint32_t epoch){
    
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(Identity, publicKey);

    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetUserLockedInfo_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(GetUserLockedInfo_input);
    packet.rcf.inputType = QEARN_GET_USER_LOCKED_INFO;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    packet.input.epoch = epoch;
    memcpy(packet.input.publicKey, publicKey, 32);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;

    GetUserLockedInfo_output result;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            auto output = (GetUserLockedInfo_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *output;
        }
        ptr+= header->size();
    }

    printf("The amount of epoch %d: %llu\n", epoch,  result.LockedAmount);
}

void qearnGetStateOfRound(const char* nodeIp, const int nodePort, uint32_t epoch){

    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetStateOfRound_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(GetStateOfRound_input);
    packet.rcf.inputType = QEARN_GET_STATE_OF_ROUND;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    packet.input.Epoch = epoch;

    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;

    GetStateOfRound_output result;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            auto output = (GetStateOfRound_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *output;
        }
        ptr+= header->size();
    }

    if(result.state == 0) printf("The state of epoch %d is not started.\n", epoch);
    if(result.state == 1) printf("The state of epoch %d is running.\n", epoch);
    if(result.state == 2) printf("The state of epoch %d is ended.\n", epoch);
}

void qearnGetUserLockedStatus(const char* nodeIp, const int nodePort, char* Identity){

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(Identity, publicKey);
    
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetUserLockStatus_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(GetUserLockStatus_input);
    packet.rcf.inputType = QEARN_GET_USER_LOCKED_STATE;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    memcpy(packet.input.publicKey, publicKey, 32);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;

    GetUserLockStatus_output result;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            auto output = (GetUserLockStatus_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *output;
        }
        ptr+= header->size();
    }

    printf("binary result: %llu\nThe number of locked epoch is as follows.\n", result.status);
    auto curSystemInfo = getSystemInfoFromNode(qc);
    uint32_t curEpoch = curSystemInfo.epoch;

    uint64_t bn = 1;

    for(uint64_t i = 0 ; i <= 52; i++) {
        if(result.status & bn)
        {
            printf("%u ", curEpoch);
        }
        curEpoch--;
        bn *= 2;
    }
    printf("\n");
}

void qearnGetEndedStatus(const char* nodeIp, const int nodePort, char* Identity){

    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(Identity, publicKey);
    
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        GetEndedStatus_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(GetEndedStatus_input);
    packet.rcf.inputType = QEARN_GET_UNLOCKED_STATE;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    memcpy(packet.input.publicKey, publicKey, 32);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;

    GetEndedStatus_output result;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            auto output = (GetEndedStatus_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *output;
        }
        ptr+= header->size();
    }

    printf("fully unlocking amount: %llu\nfully rewarded amount: %llu\nearly unlocking amount: %llu\nearly rewarded amount: %llu\n", result.Fully_Unlocked_Amount, result.Fully_Rewarded_Amount, result.Early_Unlocked_Amount, result.Early_Rewarded_Amount);

}