#include <cstring>
#include <string>
#include <sstream>

#include "msvault.h"
#include "walletUtils.h"
#include "nodeUtils.h"
#include "keyUtils.h"
#include "K12AndKeyUtil.h"
#include "logger.h"
#include "connection.h"
#include "structs.h"
#include "sanityCheck.h"
#include <set>


#define MSVAULT_CONTRACT_INDEX 11

constexpr uint64_t REGISTERING_FEE = 5000000ULL;
constexpr uint64_t RELEASE_FEE = 100000ULL;
constexpr uint64_t RELEASE_RESET_FEE = 1000000ULL;

#define MSVAULT_REGISTER_VAULT 1
#define MSVAULT_DEPOSIT 2
#define MSVAULT_RELEASE_TO 3
#define MSVAULT_RESET_RELEASE 4
#define MSVAULT_GET_VAULTS 5
#define MSVAULT_GET_RELEASE_STATUS 6
#define MSVAULT_GET_BALANCE_OF 7
#define MSVAULT_GET_VAULT_NAME 8
#define MSVAULT_GET_REVENUE_INFO 9
#define MSVAULT_GET_FEES 10
#define MSVAULT_GET_VAULT_OWNERS 11

static bool queryVaults(const char* nodeIp, int nodePort, const uint8_t publicKey[32],
    MsVaultGetVaults_output& output) 
{
    const int maxAttempts = 3;
    int attempts = 0;
    while (attempts < maxAttempts) 
    {
        MsVaultGetVaults_input input;
        memset(&input, 0, sizeof(input));
        memcpy(input.publicKey, publicKey, 32);

        auto qc = make_qc(nodeIp, nodePort);
        if (!qc) 
        {
            LOG("queryVaults: Failed to connect to node (attempt %d/%d).\n", attempts + 1, maxAttempts);
            attempts++;
            continue;
        }

        struct 
        {
            RequestResponseHeader header;
            RequestContractFunction rcf;
            MsVaultGetVaults_input in;
        } req;
        memset(&req, 0, sizeof(req));
        req.rcf.contractIndex = MSVAULT_CONTRACT_INDEX;
        req.rcf.inputType = MSVAULT_GET_VAULTS;
        req.rcf.inputSize = sizeof(input);
        memcpy(&req.in, &input, sizeof(input));
        req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
        req.header.randomizeDejavu();
        req.header.setType(RequestContractFunction::type());

        qc->sendData((uint8_t*)&req, req.header.size());

        memset(&output, 0, sizeof(output));
        try 
        {
            output = qc->receivePacketWithHeaderAs<MsVaultGetVaults_output>();
            return true;
        }
        catch (std::logic_error& e) 
        {
            LOG("queryVaults: Failed to get vaults on attempt %d/%d: %s\n", attempts + 1, maxAttempts, e.what());
            attempts++;
        }
    }
    LOG("queryVaults: All %d attempts failed. Aborting query.\n", maxAttempts);
    return false;
}

static uint64_t getVaultCountForOwner(const char* nodeIp, int nodePort, const std::string& ownerStr)
{
    uint8_t pubKey[32] = { 0 };
    getPublicKeyFromIdentity(ownerStr.c_str(), pubKey);

    MsVaultGetVaults_output output;
    if (!queryVaults(nodeIp, nodePort, pubKey, output))
    {
        LOG("getVaultCountForOwner: Failed to query vaults for owner %s\n", ownerStr.c_str());
        return 0;
    }
    return output.numberOfVaults;
}

void msvaultRegisterVault(const char* nodeIp, int nodePort, const char* seed,
    uint64_t requiredApprovals, const uint8_t vaultName[32],
    const char* ownersCommaSeparated,
    uint32_t scheduledTickOffset)
{
    std::set<std::string> uniqueOwners;
    std::string ownersStr(ownersCommaSeparated);
    std::stringstream ss(ownersStr);
    std::string owner;
    int count = 0;

    MsVaultRegisterVault_input input;
    memset(&input, 0, sizeof(input));
    memcpy(input.vaultName, vaultName, 32);

    while (std::getline(ss, owner, ',') && count < owner.size())
    {
        while (!owner.empty() && (owner.back() == ' ' || owner.back() == '\n')) owner.pop_back();
        while (!owner.empty() && owner.front() == ' ') owner.erase(owner.begin());

        if (uniqueOwners.find(owner) != uniqueOwners.end())
        {
            LOG("Duplicate owner found: %s\n", owner.c_str());
            return;
        }
        uniqueOwners.insert(owner);

        uint8_t buf[32] = { 0 };
        if (!checkSumIdentity(owner.c_str()))
        {
            LOG("Invalid owner: %s\n", owner.c_str());
            return;
        }
        getPublicKeyFromIdentity(owner.c_str(), buf);
        memcpy(input.owners + count * 32, buf, 32);
        count++;
    }

    if (count < 2)
    {
        LOG("At least 2 unique owners are required for a vault.\n");
        return;
    }

    if (count > MSVAULT_MAX_OWNERS)
    {
        LOG("At most 16 unique owners are allowed for a vault.\n");
        return;
    }

    if (requiredApprovals <= 1)
    {
        LOG("Required approvals must be greater than 1.\n");
        return;
    }
    if (requiredApprovals > (uint64_t)count)
    {
        LOG("Required approvals (%llu) cannot exceed the number of unique owners (%d).\n",
            requiredApprovals, count);
        return;
    }
    input.requiredApprovals = requiredApprovals;

    // For each unique owner, check how many vaults they already are in.
    for (const auto& own : uniqueOwners)
    {
        uint64_t vaultCount = getVaultCountForOwner(nodeIp, nodePort, own);
        if (vaultCount >= MSVAULT_MAX_COOWNER)
        {
            LOG("Owner %s already has %llu vaults (max allowed is %d). Cannot register new vault.\n",
                own.c_str(), vaultCount, MSVAULT_MAX_COOWNER);
            return;
        }
    }

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }
    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t digest[32];
    uint8_t signature[64];
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    const bool isLowerCase = false;

    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    memset(destPublicKey, 0, 32);
    ((uint64_t*)destPublicKey)[0] = MSVAULT_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        MsVaultRegisterVault_input inputData;
        uint8_t sig[64];
    } packet;
    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = REGISTERING_FEE;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = MSVAULT_REGISTER_VAULT;
    packet.transaction.inputSize = sizeof(input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(input),
        digest,
        32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction,
        sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE,
        digest,
        32);
    getTxHashFromDigest(digest, txHash);
    LOG("MsVault registerVault transaction sent.\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void msvaultDeposit(const char* nodeIp, int nodePort, const char* seed,
    uint64_t vaultID, uint64_t amount, uint32_t scheduledTickOffset)
{
    MsVaultDeposit_input input;
    input.vaultID = vaultID;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }
    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t digest[32];
    uint8_t signature[64];
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    const bool isLowerCase = false;

    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    memset(destPublicKey, 0, 32);
    ((uint64_t*)destPublicKey)[0] = MSVAULT_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        MsVaultDeposit_input inputData;
        uint8_t sig[64];
    } packet;
    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = amount;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = MSVAULT_DEPOSIT;
    packet.transaction.inputSize = sizeof(input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction, 
                   sizeof(packet.transaction) + sizeof(input),
                   digest, 
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction, 
                   sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE, 
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);
    LOG("MsVault deposit transaction sent.\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void msvaultReleaseTo(const char* nodeIp, int nodePort, const char* seed,
    uint64_t vaultID, uint64_t amount, const char* destinationIdentity,
    uint32_t scheduledTickOffset)
{
    MsVaultReleaseTo_input input;
    memset(&input, 0, sizeof(input));
    input.vaultID = vaultID;
    input.amount = amount;

    if (!checkSumIdentity(destinationIdentity)) {
        LOG("Invalid destination identity: %s\n", destinationIdentity);
        return;
    }
    getPublicKeyFromIdentity(destinationIdentity, input.destination);

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }
    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t digest[32];
    uint8_t signature[64];
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    const bool isLowerCase = false;

    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    memset(destPublicKey, 0, 32);
    ((uint64_t*)destPublicKey)[0] = MSVAULT_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        MsVaultReleaseTo_input inputData;
        uint8_t sig[64];
    } packet;
    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = RELEASE_FEE;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = MSVAULT_RELEASE_TO;
    packet.transaction.inputSize = sizeof(input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction, 
                   sizeof(packet.transaction) + sizeof(input), 
                   digest, 
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction, 
                   sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE,
                   digest, 
                   32);
    getTxHashFromDigest(digest, txHash);
    LOG("MsVault releaseTo transaction sent.\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void msvaultResetRelease(const char* nodeIp, int nodePort, const char* seed,
    uint64_t vaultID, uint32_t scheduledTickOffset)
{
    MsVaultResetRelease_input input;
    input.vaultID = vaultID;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }
    uint8_t subseed[32] = { 0 };
    uint8_t privateKey[32] = { 0 };
    uint8_t sourcePublicKey[32] = { 0 };
    uint8_t destPublicKey[32] = { 0 };
    uint8_t digest[32];
    uint8_t signature[64];
    char publicIdentity[128] = { 0 };
    char txHash[128] = { 0 };
    const bool isLowerCase = false;

    getSubseedFromSeed((uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    getIdentityFromPublicKey(sourcePublicKey, publicIdentity, isLowerCase);
    memset(destPublicKey, 0, 32);
    ((uint64_t*)destPublicKey)[0] = MSVAULT_CONTRACT_INDEX;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        MsVaultResetRelease_input inputData;
        uint8_t sig[64];
    } packet;
    memset(&packet, 0, sizeof(packet));
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = RELEASE_RESET_FEE;
    uint32_t currentTick = getTickNumberFromNode(qc);
    packet.transaction.tick = currentTick + scheduledTickOffset;
    packet.transaction.inputType = MSVAULT_RESET_RELEASE;
    packet.transaction.inputSize = sizeof(input);
    memcpy(&packet.inputData, &input, sizeof(input));
    KangarooTwelve((uint8_t*)&packet.transaction,
                   sizeof(packet.transaction) + sizeof(input),
                   digest, 
                   32);
    sign(subseed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, 64);
    packet.header.setSize(sizeof(packet));
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);
    qc->sendData((uint8_t*)&packet, packet.header.size());
    KangarooTwelve((uint8_t*)&packet.transaction, 
                   sizeof(packet.transaction) + sizeof(input) + SIGNATURE_SIZE, 
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);
    LOG("MsVault resetRelease transaction sent.\n");
    printReceipt(packet.transaction, txHash, nullptr);
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", currentTick + scheduledTickOffset, txHash);
    LOG("to check your tx confirmation status\n");
}

void msvaultGetVaults(const char* nodeIp, int nodePort, const char* identity)
{
    uint8_t pubKey[32] = { 0 };
    if (!checkSumIdentity(identity))
    {
        LOG("Invalid identity: %s\n", identity);
        return;
    }
    getPublicKeyFromIdentity(identity, pubKey);

    MsVaultGetVaults_output output;
    if (!queryVaults(nodeIp, nodePort, pubKey, output))
    {
        LOG("Failed to get vaults.\n");
        return;
    }

    LOG("Number of vaults: %u\n", output.numberOfVaults);
    for (int i = 0; i < output.numberOfVaults; i++) {
        char vaultNameStr[33];
        memcpy(vaultNameStr, output.vaultNames[i], 32);
        vaultNameStr[32] = '\0';
        LOG("Vault #%d: ID %llu, Name: %s\n", i, (unsigned long long)output.vaultIDs[i], vaultNameStr);
    }
}

void msvaultGetReleaseStatus(const char* nodeIp, int nodePort, uint64_t vaultID)
{
    MsVaultGetReleaseStatus_input input;
    input.vaultID = vaultID;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        MsVaultGetReleaseStatus_input in;
    } req;
    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = MSVAULT_CONTRACT_INDEX;
    req.rcf.inputType = MSVAULT_GET_RELEASE_STATUS;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    MsVaultGetReleaseStatus_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<MsVaultGetReleaseStatus_output>();
    }
    catch (std::logic_error& e) {
        LOG("Failed to get release status.\n");
        return;
    }

    if (!output.status)
    {
        LOG("Failed to get release status. Status returned false.\n");
        return;
    }

    for (int i = 0; i < MSVAULT_MAX_OWNERS; i++)
    {
        if (output.amounts[i] != 0 || !isZeroPubkey(output.destinations[i]))
        {
            char destId[128];
            memset(destId, 0, 128);
            getIdentityFromPublicKey(output.destinations[i], destId, false);
            LOG("Owner #%d wants to release %llu to %s\n", i, (unsigned long long)output.amounts[i], destId);
        }
    }
}

void msvaultGetBalanceOf(const char* nodeIp, int nodePort, uint64_t vaultID)
{
    MsVaultGetBalanceOf_input input;
    input.vaultID = vaultID;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        MsVaultGetBalanceOf_input in;
    } req;
    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = MSVAULT_CONTRACT_INDEX;
    req.rcf.inputType = MSVAULT_GET_BALANCE_OF;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    MsVaultGetBalanceOf_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<MsVaultGetBalanceOf_output>();
    }
    catch (std::logic_error& e) {
        LOG("Failed to get balance.\n");
        return;
    }

    if (!output.status)
    {
        LOG("Failed to get balance. Status returned false.\n");
        return;
    }

    LOG("VaultID %llu balance: %lld\n", (unsigned long long)vaultID, (long long)output.balance);
}

void msvaultGetVaultName(const char* nodeIp, int nodePort, uint64_t vaultID)
{
    MsVaultGetVaultName_input input;
    input.vaultID = vaultID;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        MsVaultGetVaultName_input in;
    } req;
    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = MSVAULT_CONTRACT_INDEX;
    req.rcf.inputType = MSVAULT_GET_VAULT_NAME;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    MsVaultGetVaultName_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<MsVaultGetVaultName_output>();
    }
    catch (std::logic_error& e) {
        LOG("Failed to get vault name.\n");
        return;
    }

    if (!output.status)
    {
        LOG("Failed to get vault name. Status returned false.\n");
        return;
    }

    char vaultNameStr[33] = { 0 };
    memcpy(vaultNameStr, output.vaultName, 32);
    vaultNameStr[32] = '\0';
    LOG("Vault name: %s\n", vaultNameStr);
}

void msvaultGetRevenueInfo(const char* nodeIp, int nodePort)
{
    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } req;
    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = MSVAULT_CONTRACT_INDEX;
    req.rcf.inputType = MSVAULT_GET_REVENUE_INFO;
    req.rcf.inputSize = 0;
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    MsVaultGetRevenueInfo_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<MsVaultGetRevenueInfo_output>();
    }
    catch (std::logic_error& e) {
        LOG("Failed to get revenue info.\n");
        return;
    }

    LOG("Number of Active Vaults: %llu\n", output.numberOfActiveVaults);
    LOG("Total Revenue: %llu\n", (unsigned long long)output.totalRevenue);
    LOG("Total Distributed To Shareholders: %llu\n", (unsigned long long)output.totalDistributedToShareholders);
}

void msvaultGetFees(const char* nodeIp, int nodePort)
{
    MsVaultGetFees_input input;
    memset(&input, 0, sizeof(input));

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        MsVaultGetFees_input in;
    } req;
    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = MSVAULT_CONTRACT_INDEX;
    req.rcf.inputType = MSVAULT_GET_FEES;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));
    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    if (qc->sendData((uint8_t*)&req, req.header.size()) != (int)req.header.size()) {
        LOG("Failed to send msVault getFees request.\n");
        return;
    }

    MsVaultGetFees_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<MsVaultGetFees_output>();
    }
    catch (std::logic_error& e) {
        LOG("Failed to receive MsVault getFees output: %s\n", e.what());
        return;
    }

    LOG("MsVault Fees:\n");
    LOG("Registering Fee: %llu\n", (unsigned long long)output.registeringFee);
    LOG("Release Fee: %llu\n", (unsigned long long)output.releaseFee);
    LOG("Release Reset Fee: %llu\n", (unsigned long long)output.releaseResetFee);
    LOG("Holding Fee: %llu\n", (unsigned long long)output.holdingFee);
    LOG("Deposit Fee: %llu\n", (unsigned long long)output.depositFee); // always 0
}

void msvaultGetVaultOwners(const char* nodeIp, int nodePort, uint64_t vaultID)
{
    MsVaultGetVaultOwners_input input;
    memset(&input, 0, sizeof(input));
    input.vaultID = vaultID;

    auto qc = make_qc(nodeIp, nodePort);
    if (!qc) {
        LOG("Failed to connect to node.\n");
        return;
    }

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        MsVaultGetVaultOwners_input in;
    } req;
    memset(&req, 0, sizeof(req));
    req.rcf.contractIndex = MSVAULT_CONTRACT_INDEX;
    req.rcf.inputType = MSVAULT_GET_VAULT_OWNERS;
    req.rcf.inputSize = sizeof(input);
    memcpy(&req.in, &input, sizeof(input));

    req.header.setSize(sizeof(req.header) + sizeof(req.rcf) + sizeof(input));
    req.header.randomizeDejavu();
    req.header.setType(RequestContractFunction::type());

    qc->sendData((uint8_t*)&req, req.header.size());

    MsVaultGetVaultOwners_output output;
    memset(&output, 0, sizeof(output));
    try {
        output = qc->receivePacketWithHeaderAs<MsVaultGetVaultOwners_output>();
    }
    catch (std::logic_error& e) {
        LOG("Failed to get vault owners.\n");
        return;
    }

    // Check status
    if (!output.status) {
        LOG("Vault is invalid or inactive. Status=0\n");
        return;
    }

    LOG("Vault %llu has %llu owners, requiredApprovals=%llu\n",
        (unsigned long long)vaultID,
        (unsigned long long)output.numberOfOwners,
        (unsigned long long)output.requiredApprovals
    );

    for (uint64_t i = 0; i < output.numberOfOwners && i < MSVAULT_MAX_OWNERS; i++) {
        // Convert 32-byte public key to identity or hex
        char ownerIdentity[128] = { 0 };
        getIdentityFromPublicKey(output.owners[i], ownerIdentity, false);

        LOG("Owner #%d => %s\n", (int)i, ownerIdentity);
    }
}
