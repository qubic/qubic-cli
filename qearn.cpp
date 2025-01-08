#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <algorithm>

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
#define QEARN_GET_STATS 6

// QEARN PROCEDURES
#define QEARN_LOCK 1
#define QEARN_UNLOCK 2

struct Unlock_input
{
    uint64_t Amount;                            /* unlocking amount */	
    uint32_t Locked_Epoch;                      /* locked epoch */
};
struct Unlock_output
{
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

void qearnGetInfoPerEpoch(const char* nodeIp, const int nodePort, uint32_t epoch)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QEarnGetLockInfoPerEpoch_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QEarnGetLockInfoPerEpoch_input);
    packet.rcf.inputType = QEARN_GET_LOCK_INFO_PER_EPOCH;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX; 
    packet.input.Epoch = epoch;
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QEarnGetLockInfoPerEpoch_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QEarnGetLockInfoPerEpoch_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to receive data\n");
        return;
    }
    uint64_t bonus = result.BonusAmount;
    uint64_t lockedAmount = result.LockedAmount;
    uint64_t currentBonus = result.CurrentBonusAmount;
    uint64_t currentLocked = result.CurrentLockedAmount;
    uint32_t t = 0, i = 0;
    char str1[100] = {0,};
    char str2[100] = {0,};
    char str3[100] = {0,};
    char str4[100] = {0,};

    while(bonus)
    {
        str1[i++] = (bonus % 10) + '0';
        bonus /= 10;
        t++;
        if(t % 3 == 0 && bonus != 0) 
        {
            str1[i++] = ',';
        }
    }
    for (size_t r = 0, j = i - 1; r < j; ++r, --j) {
        std::swap(str1[r], str1[j]);
    }
    t = 0; i = 0;
    while(lockedAmount)
    {
        str2[i++] = (lockedAmount % 10) + '0';
        lockedAmount /= 10;
        t++;
        if(t % 3 == 0 && lockedAmount != 0) 
        {
            str2[i++] = ',';
        }
    }
    for (size_t r = 0, j = i - 1; r < j; ++r, --j) {
        std::swap(str2[r], str2[j]);
    }
    t = 0; i = 0;
    while(currentBonus)
    {
        str3[i++] = (currentBonus % 10) + '0';
        currentBonus /= 10;
        t++;
        if(t % 3 == 0 && currentBonus != 0) 
        {
            str3[i++] = ',';
        }
    }
    for (size_t r = 0, j = i - 1; r < j; ++r, --j) {
        std::swap(str3[r], str3[j]);
    }
    t = 0; i = 0;
    while(currentLocked)
    {
        str4[i++] = (currentLocked % 10) + '0';
        currentLocked /= 10;
        t++;
        if(t % 3 == 0 && currentLocked != 0) 
        {
            str4[i++] = ',';
        }
    }
    
    for (size_t r = 0, j = i - 1; r < j; ++r, --j) {
        std::swap(str4[r], str4[j]);
    }
    printf("Initial Bonus Amount:  %s\nInitial Locked Amount: %s\nCurrent Bonus Amount:  %s\nCurrent Locked Amount: %s\nYield: %.6f%%\n", str1, str2, str3, str4, double(result.Yield) / 100000.0);
}

void qearnGetUserLockedInfo(const char* nodeIp, const int nodePort, char* Identity, uint32_t epoch)
{
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(Identity, publicKey);

    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QEarnGetUserLockedInfo_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QEarnGetUserLockedInfo_input);
    packet.rcf.inputType = QEARN_GET_USER_LOCKED_INFO;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    packet.input.epoch = epoch;
    memcpy(packet.input.publicKey, publicKey, 32);

    qc->sendData((uint8_t *) &packet, packet.header.size());

    QEarnGetUserLockedInfo_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QEarnGetUserLockedInfo_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to receive data\n");
        return;
    }
    printf("The amount of epoch %d: %llu\n", epoch,  result.LockedAmount);
}

void qearnGetStateOfRound(const char* nodeIp, const int nodePort, uint32_t epoch)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QEarnGetStateOfRound_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QEarnGetStateOfRound_input);
    packet.rcf.inputType = QEARN_GET_STATE_OF_ROUND;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    packet.input.Epoch = epoch;

    qc->sendData((uint8_t *) &packet, packet.header.size());

    QEarnGetStateOfRound_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QEarnGetStateOfRound_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to receive data\n");
        return;
    }

    if(result.state == 0) printf("The state of epoch %d is not started.\n", epoch);
    if(result.state == 1) printf("The state of epoch %d is running.\n", epoch);
    if(result.state == 2) printf("The state of epoch %d is ended.\n", epoch);
}

void qearnGetStatsPerEpoch(const char* nodeIp, const int nodePort, uint32_t epoch)
{
    auto qc = make_qc(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QEarnGetStatsPerEpoch_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QEarnGetStatsPerEpoch_input);
    packet.rcf.inputType = QEARN_GET_STATS;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    packet.input.epoch = epoch;

    qc->sendData((uint8_t *) &packet, packet.header.size());

    QEarnGetStatsPerEpoch_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QEarnGetStatsPerEpoch_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to receive data\n");
        return;
    }

    uint64_t earlyUnlockedAmount = result.earlyUnlockedAmount;
    uint64_t totalLockedAmount = result.totalLockedAmount;
    uint32_t t = 0, i = 0;
    char str1[100] = {0,};
    char str2[100] = {0,};

    if(result.earlyUnlockedAmount == 0)
    {
        str1[0] = '0';
    }

    while(earlyUnlockedAmount)
    {
        str1[i++] = (earlyUnlockedAmount % 10) + '0';
        earlyUnlockedAmount /= 10;
        t++;
        if(t % 3 == 0 && earlyUnlockedAmount != 0) 
        {
            str1[i++] = ',';
        }
    }
    if(result.earlyUnlockedAmount) for (size_t r = 0, j = i - 1; r < j; ++r, --j) {
        std::swap(str1[r], str1[j]);
    }
    t = 0; i = 0;
    while(totalLockedAmount)
    {
        str2[i++] = (totalLockedAmount % 10) + '0';
        totalLockedAmount /= 10;
        t++;
        if(t % 3 == 0 && totalLockedAmount != 0) 
        {
            str2[i++] = ',';
        }
    }
    
    for (size_t r = 0, j = i - 1; r < j; ++r, --j) {
        std::swap(str2[r], str2[j]);
    }

    printf("Early Unlocked Amount: %s\nEarly Unlocked Percent: %.3f%%\nTotal Locked Amount In QEarn SC: %s\nAverage APY Of QEarn SC: %.6f%%\n", str1, double(result.earlyUnlockedPercent) / 100.0, str2, double(result.averageAPY) / 100000.0);
}

void qearnGetUserLockedStatus(const char* nodeIp, const int nodePort, char* Identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(Identity, publicKey);
    
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QEarnGetUserLockStatus_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QEarnGetUserLockStatus_input);
    packet.rcf.inputType = QEARN_GET_USER_LOCKED_STATE;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    memcpy(packet.input.publicKey, publicKey, 32);

    qc->sendData((uint8_t *) &packet, packet.header.size());

    QEarnGetUserLockStatus_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QEarnGetUserLockStatus_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to receive data\n");
        return;
    }

    printf("binary result: %llu\nThe number of locked epoch is as follows.\n", result.status);
    auto curSystemInfo = getSystemInfoFromNode(qc);
    uint32_t curEpoch = curSystemInfo.epoch;

    uint64_t bn = 1;

    for (uint64_t i = 0 ; i <= 52; i++)
    {
        if (result.status & bn)
        {
            printf("%u ", curEpoch);
        }
        curEpoch--;
        bn *= 2;
    }
    printf("\n");
}

void qearnGetEndedStatus(const char* nodeIp, const int nodePort, char* Identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(Identity, publicKey);
    
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QEarnGetEndedStatus_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QEarnGetEndedStatus_input);
    packet.rcf.inputType = QEARN_GET_UNLOCKED_STATE;
    packet.rcf.contractIndex = QEARN_CONTRACT_INDEX;

    memcpy(packet.input.publicKey, publicKey, 32);

    qc->sendData((uint8_t *) &packet, packet.header.size());

    QEarnGetEndedStatus_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QEarnGetEndedStatus_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to receive data\n");
        return;
    }
    printf("fully unlocking amount: %llu\nfully rewarded amount: %llu\nearly unlocking amount: %llu\nearly rewarded amount: %llu\n", result.Fully_Unlocked_Amount, result.Fully_Rewarded_Amount, result.Early_Unlocked_Amount, result.Early_Rewarded_Amount);
}