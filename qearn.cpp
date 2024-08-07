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

#define QEARN_CONTRACT_INDEX 6

// QEARN FUNCTIONS
#define QEARN_GET_LOCK_INFO_PER_EPOCH 1
#define QEARN_GET_USER_LOCKED_INFO 2

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
    uint64_t LockedAmount;                      /* total locked amount in epoch */
    uint64_t BonusAmount;                       /* bonus amount in epoch excluding the early unlocking */
};

struct GetUserLockedInfo_input {
};

struct GetUserLockedInfo_output {

    uint64_t LockedAmount1;                   /* the amount user locked  */
    uint64_t LockedAmount2;
    uint64_t LockedAmount3;
    uint64_t LockedAmount4;
    uint64_t LockedAmount5;
    uint64_t LockedAmount6;
    uint64_t LockedAmount7;
    uint64_t LockedAmount8;
    uint64_t LockedAmount9;
    uint64_t LockedAmount10;
    uint64_t LockedAmount11;
    uint64_t LockedAmount12;
    uint64_t LockedAmount13;
    uint64_t LockedAmount14;
    uint64_t LockedAmount15;
    uint64_t LockedAmount16;
    uint64_t LockedAmount17;
    uint64_t LockedAmount18;
    uint64_t LockedAmount19;
    uint64_t LockedAmount20;
    uint64_t LockedAmount21;
    uint64_t LockedAmount22;
    uint64_t LockedAmount23;
    uint64_t LockedAmount24;
    uint64_t LockedAmount25;
    uint64_t LockedAmount26;
    uint64_t LockedAmount27;
    uint64_t LockedAmount28;
    uint64_t LockedAmount29;
    uint64_t LockedAmount30;
    uint64_t LockedAmount31;
    uint64_t LockedAmount32;
    uint64_t LockedAmount33;
    uint64_t LockedAmount34;
    uint64_t LockedAmount35;
    uint64_t LockedAmount36;
    uint64_t LockedAmount37;
    uint64_t LockedAmount38;
    uint64_t LockedAmount39;
    uint64_t LockedAmount40;
    uint64_t LockedAmount41;
    uint64_t LockedAmount42;
    uint64_t LockedAmount43;
    uint64_t LockedAmount44;
    uint64_t LockedAmount45;
    uint64_t LockedAmount46;
    uint64_t LockedAmount47;
    uint64_t LockedAmount48;
    uint64_t LockedAmount49;
    uint64_t LockedAmount50;
    uint64_t LockedAmount51;
    uint64_t LockedAmount52;
    uint64_t LockedAmount53;
    uint64_t LockedAmount54;
    uint64_t LockedAmount55;
    uint64_t LockedAmount56;
    uint64_t LockedAmount57;
    uint64_t LockedAmount58;
    uint64_t LockedAmount59;
    uint64_t LockedAmount60;
    uint64_t LockedAmount61;
    uint64_t LockedAmount62;
    uint64_t LockedAmount63;
    uint64_t LockedAmount64;
    

    uint32_t LockedEpoch1;                    /* the epoch user locked */
    uint32_t LockedEpoch2;
    uint32_t LockedEpoch3;
    uint32_t LockedEpoch4;
    uint32_t LockedEpoch5;
    uint32_t LockedEpoch6;
    uint32_t LockedEpoch7;
    uint32_t LockedEpoch8;
    uint32_t LockedEpoch9;
    uint32_t LockedEpoch10;
    uint32_t LockedEpoch11;
    uint32_t LockedEpoch12;
    uint32_t LockedEpoch13;
    uint32_t LockedEpoch14;
    uint32_t LockedEpoch15;
    uint32_t LockedEpoch16;
    uint32_t LockedEpoch17;
    uint32_t LockedEpoch18;
    uint32_t LockedEpoch19;
    uint32_t LockedEpoch20;
    uint32_t LockedEpoch21;
    uint32_t LockedEpoch22;
    uint32_t LockedEpoch23;
    uint32_t LockedEpoch24;
    uint32_t LockedEpoch25;
    uint32_t LockedEpoch26;
    uint32_t LockedEpoch27;
    uint32_t LockedEpoch28;
    uint32_t LockedEpoch29;
    uint32_t LockedEpoch30;
    uint32_t LockedEpoch31;
    uint32_t LockedEpoch32;
    uint32_t LockedEpoch33;
    uint32_t LockedEpoch34;
    uint32_t LockedEpoch35;
    uint32_t LockedEpoch36;
    uint32_t LockedEpoch37;
    uint32_t LockedEpoch38;
    uint32_t LockedEpoch39;
    uint32_t LockedEpoch40;
    uint32_t LockedEpoch41;
    uint32_t LockedEpoch42;
    uint32_t LockedEpoch43;
    uint32_t LockedEpoch44;
    uint32_t LockedEpoch45;
    uint32_t LockedEpoch46;
    uint32_t LockedEpoch47;
    uint32_t LockedEpoch48;
    uint32_t LockedEpoch49;
    uint32_t LockedEpoch50;
    uint32_t LockedEpoch51;
    uint32_t LockedEpoch52;
    uint32_t LockedEpoch53;
    uint32_t LockedEpoch54;
    uint32_t LockedEpoch55;
    uint32_t LockedEpoch56;
    uint32_t LockedEpoch57;
    uint32_t LockedEpoch58;
    uint32_t LockedEpoch59;
    uint32_t LockedEpoch60;
    uint32_t LockedEpoch61;
    uint32_t LockedEpoch62;
    uint32_t LockedEpoch63;
    uint32_t LockedEpoch64;
    
    uint32_t NumberOfLockedEpoch;
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
    LOG("BurnQubic tx has been sent!\n");
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
    LOG("BurnQubic tx has been sent!\n");
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
            auto info = (GetLockInfoPerEpoch_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *info;
        }
        ptr+= header->size();
    }
    printf("bonus amount:%llu\nlocked amount:%llu\nminimum yield:%d", result.BonusAmount, result.LockedAmount, result.BonusAmount * 100 / result.LockedAmount);
}

void qearnGetUserLockedInfo(const char* nodeIp, const int nodePort){
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
            auto fees = (GetUserLockedInfo_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *fees;
        }
        ptr+= header->size();
    }

    printf("number of locked epoch: %d\n", result.NumberOfLockedEpoch);
    for(uint16_t t = 0 ; t < result.NumberOfLockedEpoch; t++) {
        if(t == 0) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount1, result.LockedEpoch1);
        if(t == 1) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount2, result.LockedEpoch2);
        if(t == 2) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount3, result.LockedEpoch3);
        if(t == 3) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount4, result.LockedEpoch4);
        if(t == 4) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount5, result.LockedEpoch5);
        if(t == 5) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount6, result.LockedEpoch6);
        if(t == 6) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount7, result.LockedEpoch7);
        if(t == 7) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount8, result.LockedEpoch8);
        if(t == 8) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount9, result.LockedEpoch9);
        if(t == 9) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount10, result.LockedEpoch10);
        if(t == 10) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount11, result.LockedEpoch11);
        if(t == 11) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount12, result.LockedEpoch12);
        if(t == 12) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount13, result.LockedEpoch13);
        if(t == 13) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount14, result.LockedEpoch14);
        if(t == 14) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount15, result.LockedEpoch15);
        if(t == 15) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount16, result.LockedEpoch16);
        if(t == 16) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount17, result.LockedEpoch17);
        if(t == 17) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount18, result.LockedEpoch18);
        if(t == 18) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount19, result.LockedEpoch19);
        if(t == 19) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount20, result.LockedEpoch20);
        if(t == 20) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount21, result.LockedEpoch21);
        if(t == 21) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount22, result.LockedEpoch22);
        if(t == 22) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount23, result.LockedEpoch23);
        if(t == 23) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount24, result.LockedEpoch24);
        if(t == 24) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount25, result.LockedEpoch25);
        if(t == 25) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount26, result.LockedEpoch26);
        if(t == 26) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount27, result.LockedEpoch27);
        if(t == 27) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount28, result.LockedEpoch28);
        if(t == 28) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount29, result.LockedEpoch29);
        if(t == 29) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount30, result.LockedEpoch30);
        if(t == 30) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount31, result.LockedEpoch31);
        if(t == 31) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount32, result.LockedEpoch32);
        if(t == 32) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount33, result.LockedEpoch33);
        if(t == 33) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount34, result.LockedEpoch34);
        if(t == 34) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount35, result.LockedEpoch35);
        if(t == 35) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount36, result.LockedEpoch36);
        if(t == 36) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount37, result.LockedEpoch37);
        if(t == 37) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount38, result.LockedEpoch38);
        if(t == 38) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount39, result.LockedEpoch39);
        if(t == 39) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount40, result.LockedEpoch40);
        if(t == 40) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount41, result.LockedEpoch41);
        if(t == 41) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount42, result.LockedEpoch42);
        if(t == 42) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount43, result.LockedEpoch43);
        if(t == 43) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount44, result.LockedEpoch44);
        if(t == 44) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount45, result.LockedEpoch45);
        if(t == 45) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount46, result.LockedEpoch46);
        if(t == 46) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount47, result.LockedEpoch47);
        if(t == 47) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount48, result.LockedEpoch48);
        if(t == 48) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount49, result.LockedEpoch49);
        if(t == 49) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount50, result.LockedEpoch50);
        if(t == 50) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount51, result.LockedEpoch51);
        if(t == 51) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount52, result.LockedEpoch52);
        if(t == 52) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount53, result.LockedEpoch53);
        if(t == 53) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount54, result.LockedEpoch54);
        if(t == 54) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount55, result.LockedEpoch55);
        if(t == 55) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount56, result.LockedEpoch56);
        if(t == 56) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount57, result.LockedEpoch57);
        if(t == 57) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount58, result.LockedEpoch58);
        if(t == 58) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount59, result.LockedEpoch59);
        if(t == 59) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount60, result.LockedEpoch60);
        if(t == 60) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount61, result.LockedEpoch61);
        if(t == 61) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount62, result.LockedEpoch62);
        if(t == 62) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount63, result.LockedEpoch63);
        if(t == 63) printf("locked amount:%lld  locked epoch:%d\n", result.LockedAmount64, result.LockedEpoch64);
    } 
}