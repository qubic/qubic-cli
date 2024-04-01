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

std::vector<RespondOwnedAssets> getOwnedAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    std::vector<RespondOwnedAssets> result;
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(requestedIdentity, publicKey);
    struct {
        RequestResponseHeader header;
        RequestOwnedAssets req;
    } packet;
    memcpy(packet.req.publicKey, publicKey, 32);
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_OWNED_ASSETS);
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RESPOND_OWNED_ASSETS){
            auto roa = (RespondOwnedAssets *)(data + ptr + sizeof(RequestResponseHeader));
            result.push_back(* roa);
        }
        ptr+= header->size();
    }
    return result;
}
std::vector<RespondPossessedAssets> getPossessionAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    std::vector<RespondPossessedAssets> result;
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(requestedIdentity, publicKey);
    struct {
        RequestResponseHeader header;
        RequestOwnedAssets req;
    } packet;
    memcpy(packet.req.publicKey, publicKey, 32);
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_POSSESSED_ASSETS);
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RESPOND_POSSESSED_ASSETS){
            auto rpa = (RespondPossessedAssets *)(data + ptr + sizeof(RequestResponseHeader));
            result.push_back(* rpa);
        }
        ptr+= header->size();
    }
    return result;
}
static void printOwnedAsset(Asset owned, Asset iss)
{
    char issuer[128];
    char name[8] = {0};
    char unitOfMeasurement[7] = {0};
    memcpy(name, iss.varStruct.issuance.name, 7);
    memcpy(unitOfMeasurement, iss.varStruct.issuance.unitOfMeasurement, 7);
    getIdentityFromPublicKey(iss.varStruct.issuance.publicKey, issuer, false);
    LOG("Asset issuer: %s\n", issuer);
    LOG("Asset name: %s\n", name);
    LOG("Managing contract index: %d\n", owned.varStruct.ownership.managingContractIndex);
    LOG("Issuance Index: %d\n", owned.varStruct.ownership.issuanceIndex);
    LOG("Number Of Units: %d\n", owned.varStruct.ownership.numberOfUnits);
}
static void printPossessionAsset(Asset owner, Asset possession, Asset iss)
{
    char issuer[128];
    char name[8] = {0};
    char ownerId[128] = {0};
    getIdentityFromPublicKey(owner.varStruct.ownership.publicKey, ownerId, false);
    memcpy(name, iss.varStruct.issuance.name, 7);
    getIdentityFromPublicKey(iss.varStruct.issuance.publicKey, issuer, false);
    LOG("Asset issuer: %s\n", issuer);
    LOG("Asset name: %s\n", name);
    LOG("Managing contract index: %d\n", possession.varStruct.possession.managingContractIndex);
    LOG("Owner index: %u\n", possession.varStruct.possession.ownershipIndex);
    LOG("Owner ID: %s\n", ownerId);
    LOG("Number Of Units: %d\n", possession.varStruct.possession.numberOfUnits);
}
void printOwnedAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    LOG("======== OWNERSHIP ========\n");
    auto vroa = getOwnedAsset(nodeIp, nodePort, requestedIdentity);
    for (auto& roa : vroa){
        printOwnedAsset(roa.asset, roa.issuanceAsset);
        LOG("Tick: %u\n", roa.tick);
    }
}
void printPossessionAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    LOG("======== POSSESSION ========\n");
    auto vrpa = getPossessionAsset(nodeIp, nodePort, requestedIdentity);
    for (auto& rpa : vrpa){
        printPossessionAsset(rpa.ownershipAsset, rpa.asset, rpa.issuanceAsset);
        LOG("Tick: %u\n", rpa.tick);
    }
}