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

// QVAULT FUNCTIONS
#define QVAULT_GETDATA 1

// QVAULT PROCEDURES
#define QVAULT_SUBMITAUTHADDRESS 1
#define QVAULT_CHANGEAUTHADDRESS 2
#define QVAULT_SUBMITFEES 3
#define QVAULT_CHANGEFEES 4
#define QVAULT_SUBMITREINVESTINGADDRESS 5
#define QVAULT_CHANGEREINVESTINGADDRESS 6
#define QVAULT_SUBMITADMINADDRESS 7
#define QVAULT_CHANGEADMINADDRESS 8
#define QVAULT_SUBMITBANNEDADDRESS 9
#define QVAULT_SAVEBANNEDADDRESS 10
#define QVAULT_SUBMITUNBANNEDADDRESS 11
#define QVAULT_SAVEUNBANNEDADDRESS 12

struct submitAuthAddress_input 
{
    uint8_t newAddress[32];
};
struct submitAuthAddress_output
{
};

struct changeAuthAddress_input
{
    uint32_t numberOfChangedAddress;
};
struct changeAuthAddress_output
{
};

struct submitFees_input
{
    uint32_t newQCAPHolder_permille;
    uint32_t newreinvesting_permille;
    uint32_t newdev_permille;
};
struct submitFees_output
{
};

struct changeFees_input
{
    uint32_t newQCAPHolder_permille;
    uint32_t newreinvesting_permille;
    uint32_t newdev_permille;
};
struct changeFees_output
{
};

struct submitReinvestingAddress_input
{
    uint8_t newAddress[32];
};
struct submitReinvestingAddress_output
{
};

struct changeReinvestingAddress_input
{
    uint8_t newAddress[32];
};
struct changeReinvestingAddress_output
{
};

struct submitAdminAddress_input
{
    uint8_t newAddress[32];
};
struct submitAdminAddress_output
{
};

struct changeAdminAddress_input
{
    uint8_t newAddress[32];
};
struct changeAdminAddress_output
{
};

struct submitBannedAddress_input
{
    uint8_t bannedAddress[32];
};
struct submitBannedAddress_output
{
};

struct saveBannedAddress_input
{
    uint8_t bannedAddress[32];
};
struct saveBannedAddress_output
{
};

struct submitUnbannedAddress_input
{
    uint8_t unbannedAddress[32];
};
struct submitUnbannedAddress_output
{
};

struct unblockBannedAddress_input
{
    uint8_t unbannedAddress[32];
};
struct unblockBannedAddress_output
{
};

void submitAuthAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitAuthAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.newAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMITAUTHADDRESS;
    packet.transaction.inputSize = sizeof(submitAuthAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitAuthAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitAuthAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitAuthAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void changeAuthAddress(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t numberOfChangedAddress)
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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        changeAuthAddress_input input;
        unsigned char signature[64];
    } packet;
    
    packet.input.numberOfChangedAddress = numberOfChangedAddress;

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_CHANGEAUTHADDRESS;
    packet.transaction.inputSize = sizeof(changeAuthAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(changeAuthAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(changeAuthAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("changeAuthAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitFees(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t newQCAPHolder_permille, uint32_t newreinvesting_permille, uint32_t newdev_permille)
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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitFees_input input;
        unsigned char signature[64];
    } packet;
    
    packet.input.newdev_permille = newdev_permille;
    packet.input.newQCAPHolder_permille = newQCAPHolder_permille;
    packet.input.newreinvesting_permille = newreinvesting_permille;

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMITFEES;
    packet.transaction.inputSize = sizeof(submitFees_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitFees_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitFees_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitFees tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void changeFees(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t newQCAPHolder_permille, uint32_t newreinvesting_permille, uint32_t newdev_permille)
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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        changeFees_input input;
        unsigned char signature[64];
    } packet;

    packet.input.newdev_permille = newdev_permille;
    packet.input.newQCAPHolder_permille = newQCAPHolder_permille;
    packet.input.newreinvesting_permille = newreinvesting_permille;

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_CHANGEFEES;
    packet.transaction.inputSize = sizeof(changeFees_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(changeFees_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(changeFees_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("changeFees tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitReinvestingAddress(const char* nodeIp, int nodePort, const char* seed,  uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitReinvestingAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.newAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMITREINVESTINGADDRESS;
    packet.transaction.inputSize = sizeof(submitReinvestingAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitReinvestingAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitReinvestingAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitReinvestingAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void changeReinvestingAddress(const char* nodeIp, int nodePort, const char* seed,  uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        changeReinvestingAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.newAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_CHANGEREINVESTINGADDRESS;
    packet.transaction.inputSize = sizeof(changeReinvestingAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(changeReinvestingAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(changeReinvestingAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("changeReinvestingAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitAdminAddress(const char* nodeIp, int nodePort, const char* seed,  uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitAdminAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.newAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMITADMINADDRESS;
    packet.transaction.inputSize = sizeof(submitAdminAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitAdminAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitAdminAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitAdminAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void changeAdminAddress(const char* nodeIp, int nodePort, const char* seed,  uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        changeAdminAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.newAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_CHANGEADMINADDRESS;
    packet.transaction.inputSize = sizeof(changeAdminAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(changeAdminAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(changeAdminAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("changeAdminAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void getData(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        QVaultGetData_input input;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(QVaultGetData_input);
    packet.rcf.inputType = QVAULT_GETDATA;
    packet.rcf.contractIndex = QVAULT_CONTRACT_INDEX;
    packet.input.t = 10;
    
    qc->sendData((uint8_t *) &packet, packet.header.size());

    QVaultGetData_output result;
    try
    {
        result = qc->receivePacketWithHeaderAs<QVaultGetData_output>();
    }
    catch (std::logic_error& e)
    {
        LOG("Failed to receive data\n");
        return;
    }

    char auth1[128] = {0};
    char auth2[128] = {0};
    char auth3[128] = {0};
    char reinvesting_address[128] = {0};
    char admin_address[128] = {0};
    char newAuth1[128] = {0};
    char newAuth2[128] = {0};
    char newAuth3[128] = {0};
    char newReinvestingAddress1[128] = {0};
    char newReinvestingAddress2[128] = {0};
    char newReinvestingAddress3[128] = {0};
    char newAdminAddress1[128] = {0};
    char newAdminAddress2[128] = {0};
    char newAdminAddress3[128] = {0};
    char newBannedAddress1[128] = {0};
    char newBannedAddress2[128] = {0};
    char newBannedAddress3[128] = {0};
    char newUnbannedAddress1[128] = {0};
    char newUnbannedAddress2[128] = {0};
    char newUnbannedAddress3[128] = {0};
    getIdentityFromPublicKey(result.AUTH_ADDRESS1, auth1, false);
    getIdentityFromPublicKey(result.AUTH_ADDRESS2, auth2, false);
    getIdentityFromPublicKey(result.AUTH_ADDRESS3, auth3, false);
    getIdentityFromPublicKey(result.Reinvesting_address, reinvesting_address, false);
    getIdentityFromPublicKey(result.admin_address, admin_address, false);
    getIdentityFromPublicKey(result.newAuthAddress1, newAuth1, false);
    getIdentityFromPublicKey(result.newAuthAddress2, newAuth2, false);
    getIdentityFromPublicKey(result.newAuthAddress3, newAuth3, false);
    getIdentityFromPublicKey(result.newReinvesting_address1, newReinvestingAddress1, false);
    getIdentityFromPublicKey(result.newReinvesting_address2, newReinvestingAddress2, false);
    getIdentityFromPublicKey(result.newReinvesting_address3, newReinvestingAddress3, false);
    getIdentityFromPublicKey(result.newAdmin_address1, newAdminAddress1, false);
    getIdentityFromPublicKey(result.newAdmin_address2, newAdminAddress2, false);
    getIdentityFromPublicKey(result.newAdmin_address3, newAdminAddress3, false);
    getIdentityFromPublicKey(result.bannedAddress1, newBannedAddress1, false);
    getIdentityFromPublicKey(result.bannedAddress2, newBannedAddress2, false);
    getIdentityFromPublicKey(result.bannedAddress3, newBannedAddress3, false);
    getIdentityFromPublicKey(result.unbannedAddress1, newUnbannedAddress1, false);
    getIdentityFromPublicKey(result.unbannedAddress2, newUnbannedAddress2, false);
    getIdentityFromPublicKey(result.unbannedAddress3, newUnbannedAddress3, false);

    printf("AUTH_ADDRESS1: %s", auth1);
    printf("\nAUTH_ADDRESS2: %s", auth2);
    printf("\nAUTH_ADDRESS3: %s", auth3);
    printf("\nReinvesting_address: %s", reinvesting_address);
    printf("\nadmin_address: %s", admin_address);
    printf("\nnewAuthAddress1: %s", newAuth1);
    printf("\nnewAuthAddress2: %s", newAuth2);
    printf("\nnewAuthAddress3: %s", newAuth3);
    printf("\nnewReinvesting_address1: %s", newReinvestingAddress1);
    printf("\nnewReinvesting_address2: %s", newReinvestingAddress2);
    printf("\nnewReinvesting_address3: %s", newReinvestingAddress3);
    printf("\nnewAdmin_address1: %s", newAdminAddress1);
    printf("\nnewAdmin_address2: %s", newAdminAddress2);
    printf("\nnewAdmin_address3: %s", newAdminAddress3);
    printf("\nPermille for development:%u Permille for Reinvesting:%u Permille for Computors:%u Permille for Holders:%u\n", result.dev_permille, result.reinvesting_permille, result.computor_permille, result.QCAPHolder_permille);

    printf("\nbanned_address1: %s", newBannedAddress1);
    printf("\nbanned_address2: %s", newBannedAddress2);
    printf("\nbanned_address3: %s", newBannedAddress3);
    printf("\nunbanned_address1: %s", newUnbannedAddress1);
    printf("\nunbanned_address2: %s", newUnbannedAddress2);
    printf("\nunbanned_address3: %s", newUnbannedAddress3);
}

void submitBannedAddress(const char* nodeIp, int nodePort, const char* seed,  uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitBannedAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.bannedAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMITBANNEDADDRESS;
    packet.transaction.inputSize = sizeof(submitBannedAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitBannedAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitBannedAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitBannedAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void saveBannedAddress(const char* nodeIp, int nodePort, const char* seed,  uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        saveBannedAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.bannedAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SAVEBANNEDADDRESS;
    packet.transaction.inputSize = sizeof(saveBannedAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(saveBannedAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(saveBannedAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("saveBannedAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void submitUnbannedannedAddress(const char* nodeIp, int nodePort, const char* seed,  uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        submitUnbannedAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.unbannedAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SUBMITUNBANNEDADDRESS;
    packet.transaction.inputSize = sizeof(submitUnbannedAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitUnbannedAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(submitUnbannedAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("submitUnbannedannedAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void saveUnbannedAddress(const char* nodeIp, int nodePort, const char* seed,  uint32_t scheduledTickOffset, const char* identity)
{
    auto qc = make_qc(nodeIp, nodePort);

    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(identity, publicKey);

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

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        unblockBannedAddress_input input;
        unsigned char signature[64];
    } packet;

    memcpy(packet.input.unbannedAddress, publicKey, 32);

    packet.transaction.amount = 0;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = QVAULT_SAVEUNBANNEDADDRESS;
    packet.transaction.inputSize = sizeof(unblockBannedAddress_input);
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(unblockBannedAddress_input),
                   digest,
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.signature, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(unblockBannedAddress_input) + SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("saveUnbannedAddress tx has been sent!\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}