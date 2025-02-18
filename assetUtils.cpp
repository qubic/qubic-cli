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
#include "utils.h"
#include "sanityCheck.h"

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

    return qc->getLatestVectorPacketAs<RespondOwnedAssets>();
}

std::vector<RespondPossessedAssets> getPossessionAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    std::vector<RespondPossessedAssets> result;
    uint8_t publicKey[32] = {0};
    getPublicKeyFromIdentity(requestedIdentity, publicKey);
    struct {
        RequestResponseHeader header;
        RequestPossessedAssets req;
    } packet;
    memcpy(packet.req.publicKey, publicKey, 32);
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(REQUEST_POSSESSED_ASSETS);
    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t *) &packet, packet.header.size());

    return qc->getLatestVectorPacketAs<RespondPossessedAssets>();
}

template<typename T>
void getAssetDigest(T& respondedAsset, uint8_t* assetDigest)
{
    // Check if the size of entity is good
    const size_t asset_size = sizeof(respondedAsset.asset);
    if (asset_size != 48)
    {
        LOG("Size of asset is unexpected: %lu\n", sizeof(asset_size));
        return;
    }

    // Compute the root hash from the entity and siblings
    if (respondedAsset.universeIndex < 0 )
    {
        LOG("Universe index is invalid: %d\n", respondedAsset.universeIndex);
        return;
    }

    // Compute the spectrum digest
    getDigestFromSiblings<32>(
        ASSETS_DEPTH,
        (uint8_t*)(&respondedAsset.asset),
        asset_size,
        respondedAsset.universeIndex,
        respondedAsset.siblings,
        assetDigest);
}

template<typename T>
void printAssetDigest(T& respondedAsset)
{
    uint8_t assetDigest[32];
    getAssetDigest(respondedAsset, assetDigest);
    char hex_digest[65];
    byteToHex(assetDigest, hex_digest, 32);
    LOG("Asset Digest: %s\n", hex_digest);
}

static void printOwnedAsset(Asset owned, Asset iss)
{
    char issuer[128] = {0};
    char name[8] = {0};
    char unitOfMeasurement[7] = {0};
    memcpy(name, iss.varStruct.issuance.name, 7);
    memcpy(unitOfMeasurement, iss.varStruct.issuance.unitOfMeasurement, 7);
    getIdentityFromPublicKey(iss.varStruct.issuance.publicKey, issuer, false);
    LOG("Asset issuer: %s\n", issuer);
    LOG("Asset name: %s\n", name);
    LOG("Managing contract index: %d\n", owned.varStruct.ownership.managingContractIndex);
    LOG("Issuance Index: %d\n", owned.varStruct.ownership.issuanceIndex);
    LOG("Number Of Units: %lld\n", owned.varStruct.ownership.numberOfUnits);
}

static void printPossessionAsset(Asset owner, Asset possession, Asset iss)
{
    char issuer[128] = {0};
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
    LOG("Number Of Units: %lld\n", possession.varStruct.possession.numberOfUnits);
}

void printOwnedAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    LOG("======== OWNERSHIP ========\n");
    auto vroa = getOwnedAsset(nodeIp, nodePort, requestedIdentity);
    for (auto& roa : vroa)
    {
        printOwnedAsset(roa.asset, roa.issuanceAsset);
        printAssetDigest(roa);
        LOG("Tick: %u\n\n", roa.tick);
    }
}

void printPossessionAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity)
{
    LOG("======== POSSESSION ========\n");
    auto vrpa = getPossessionAsset(nodeIp, nodePort, requestedIdentity);
    for (auto& rpa : vrpa)
    {
        printPossessionAsset(rpa.ownershipAsset, rpa.asset, rpa.issuanceAsset);
        printAssetDigest(rpa);
        LOG("Tick: %u\n\n", rpa.tick);
    }
}

void printAssetResponse(const RespondAssets& response, bool verbose)
{
    char identity[128] = { 0 };
    char name[8] = { 0 };
    switch (response.asset.varStruct.issuance.type)
    {
    case EMPTY:
        LOG("Empty slot in universe\n");
        break;
    case ISSUANCE:
        getIdentityFromPublicKey(response.asset.varStruct.issuance.publicKey, identity, false);
        memcpy(name, response.asset.varStruct.issuance.name, 7);
        LOG("Asset (issuance)\n");
        LOG("\tname = %s\n", name);
        LOG("\tissuer = %s\n", identity);
        LOG("\tnumber of decimal places = %d\n", (int)response.asset.varStruct.issuance.numberOfDecimalPlaces);
        // TODO: print response.asset.varStruct.issuance.unitOfMeasurement;
        break;
    case OWNERSHIP:
        getIdentityFromPublicKey(response.asset.varStruct.ownership.publicKey, identity, false);
        LOG("Share ownership\n");
        LOG("\towner = %s\n", identity);
        LOG("\tnumber of shares = %lld\n", response.asset.varStruct.ownership.numberOfUnits);
        LOG("\tmanaging contract = %u\n", (unsigned int)response.asset.varStruct.ownership.managingContractIndex);
        if (verbose)
        {
            LOG("\tissuance index = %u\n", response.asset.varStruct.ownership.issuanceIndex);
        }
        break;
    case POSSESSION:
        getIdentityFromPublicKey(response.asset.varStruct.possession.publicKey, identity, false);
        LOG("Share possession\n");
        LOG("\tpossessor = %s\n", identity);
        LOG("\tnumber of shares = %lld\n", response.asset.varStruct.possession.numberOfUnits);
        LOG("\tmanaging contract = %u\n", (unsigned int)response.asset.varStruct.possession.managingContractIndex);
        if (verbose)
        {
            LOG("\townership index = %u\n", response.asset.varStruct.possession.ownershipIndex);
        }
        break;
    }
    if (verbose)
    {
        LOG("\tindex = %u\n", response.universeIndex);
        LOG("\ttick = %u\n", response.tick);
    }
}

void printAssetResponseWithSiblings(const RespondAssetsWithSiblings& response, bool verbose)
{
    printAssetResponse(response, verbose);

    uint8_t assetDigest[32];
    getAssetDigest(response, assetDigest);
    char hex_digest[65];
    byteToHex(assetDigest, hex_digest, 32);
    LOG("\tuniverse digest = %s\n", hex_digest);
}

void printAssetRecordsHelpAndExit()
{
    LOG("qubic-cli -queryassets [type] [query]\n");
    LOG("  [type] is one of the following (without \"):\n");
    LOG("    - \"issuances\" or \"i\": Get asset issuance records. For filtering, [query] can be comma-separated list with:\n");
    LOG("        - \"issuer=[identity]\": Only consider assets of the given issuer.\n");
    LOG("        - \"name=[name]\": Only consider assets with the given name.\n");
    LOG("    - \"ownerships\" or \"o\": Get asset ownership records, [query] is comma-separated list with at least \"name\".\n");
    LOG("        - \"issuer=[identity]\": Issuer of shares to consider (skip for NULL_ID / contract shares).\n");
    LOG("        - \"name=[name]\": Asset name of shares to consider.\n");
    LOG("        - \"owner=[identity]\": Owner of shares to consider (skip for all owners).\n");
    LOG("        - \"oc=[index]\": Index of contract managing ownership (skip for all contracts).\n");
    LOG("    - \"possessions\" or \"p\": Get asset possession records, [query] is like for ownerships, but may additionally have:\n");
    LOG("        - \"possessor=[identity]\": Possessor of shares to consider (skip for all possessors).\n");
    LOG("        - \"pc=[index]\": Index of contract managing possession (skip for all contracts).\n");
    LOG("    - \"idx\": Get asset record with given universe index, [query] is the index number.\n");
    exit(1);
}

void printAssetRecords(const char* nodeIp, const int nodePort, const char* requestType, const char* requestQueryString)
{
#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif
    bool withSiblings = false;
    bool verbose = true;

    struct {
        RequestResponseHeader header;
        RequestAssets req;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestAssets::type());
    memset(&packet.req, 0, sizeof(packet.req));
    if (withSiblings)
        packet.req.byFilter.flags |= RequestAssets::getSiblings;

    bool queryOwnerships = strcasecmp(requestType, "ownerships") == 0 || strcasecmp(requestType, "o") == 0;
    bool queryPossessions = strcasecmp(requestType, "possessions") == 0 || strcasecmp(requestType, "p") == 0;

    if (strcasecmp(requestType, "idx") == 0)
    {
        packet.req.assetReqType = RequestAssets::requestByUniverseIdx;
        if (requestQueryString == nullptr || sscanf(requestQueryString, "%u", &packet.req.byUniverseIdx.universeIdx) != 1)
        {
            LOG("Error: Expected positive number (index in universe), found \"%s\"!\n", requestQueryString ? requestQueryString : "");
            exit(1);
        }
    }
    else if (strcasecmp(requestType, "issuances") == 0 || strcasecmp(requestType, "i") == 0)
    {
        packet.req.assetReqType = RequestAssets::requestIssuanceRecords;
        packet.req.byFilter.flags = RequestAssets::anyIssuer | RequestAssets::anyAssetName;
        if (requestQueryString != nullptr && strlen(requestQueryString) > 0)
        {
            auto assignments = splitString(requestQueryString, ",");
            for (std::string& assignment : assignments)
            {
                auto parts = splitString(assignment, "=");
                std::string id = parts[0];
                std::string value = parts.size() > 1 ? parts[1] : "";
                if (strcasecmp(id.c_str(), "issuer") == 0)
                {
                    packet.req.byFilter.flags &= ~RequestAssets::anyIssuer;
                    sanityCheckIdentity(value.c_str());
                    getPublicKeyFromIdentity(value.c_str(), packet.req.byFilter.issuer);
                }
                else if (strcasecmp(id.c_str(), "name") == 0)
                {
                    packet.req.byFilter.flags &= ~RequestAssets::anyAssetName;
                    sanityCheckValidAssetName(value.c_str());
                    memcpy(&packet.req.byFilter.assetName, value.c_str(), 7);
                }
                else
                {
                    LOG("Error: Invalid identifier in query string. Expected \"issuer\" or \"name\", found \"%s\"!\n", id.c_str());
                    exit(1);
                }
            }
        }
    }
    else if (queryOwnerships || queryPossessions)
    {
        if (queryOwnerships)
            packet.req.assetReqType = RequestAssets::requestOwnershipRecords;
        else
            packet.req.assetReqType = RequestAssets::requestPossessionRecords;
        packet.req.byFilter.flags = RequestAssets::anyOwner | RequestAssets::anyPossessor | RequestAssets::anyOwnershipManagingContract | RequestAssets::anyPossessionManagingContract;
        bool issuerGiven = false, nameGiven = false;
        if (requestQueryString != nullptr && strlen(requestQueryString) > 0)
        {
            auto assignments = splitString(requestQueryString, ",");
            for (std::string& assignment : assignments)
            {
                auto parts = splitString(assignment, "=");
                std::string id = parts[0];
                std::string value = parts.size() > 1 ? parts[1] : "";
                if (strcasecmp(id.c_str(), "issuer") == 0)
                {
                    issuerGiven = true;
                    sanityCheckIdentity(value.c_str());
                    getPublicKeyFromIdentity(value.c_str(), packet.req.byFilter.issuer);
                }
                else if (strcasecmp(id.c_str(), "name") == 0)
                {
                    nameGiven = true;
                    sanityCheckValidAssetName(value.c_str());
                    memcpy(&packet.req.byFilter.assetName, value.c_str(), 7);
                }
                else if (strcasecmp(id.c_str(), "owner") == 0)
                {
                    packet.req.byFilter.flags &= ~RequestAssets::anyOwner;
                    sanityCheckIdentity(value.c_str());
                    getPublicKeyFromIdentity(value.c_str(), packet.req.byFilter.owner);
                }
                else if (strcasecmp(id.c_str(), "possessor") == 0)
                {
                    if (queryOwnerships)
                    {
                        LOG("Warning: Ignoring %s, because ownership records are queried!\n", assignment.c_str());
                        continue;
                    }
                    packet.req.byFilter.flags &= ~RequestAssets::anyPossessor;
                    sanityCheckIdentity(value.c_str());
                    getPublicKeyFromIdentity(value.c_str(), packet.req.byFilter.possessor);
                }
                else if (strcasecmp(id.c_str(), "oc") == 0)
                {
                    packet.req.byFilter.flags &= ~RequestAssets::anyOwnershipManagingContract;
                    if (sscanf(value.c_str(), "%hu", &packet.req.byFilter.ownershipManagingContract) != 1)
                    {
                        LOG("Error: Expected unsigned short number for ownership managing contract index, found \"%s\"!\n", value.c_str());
                        exit(1);
                    }
                }
                else if (strcasecmp(id.c_str(), "pc") == 0)
                {
                    if (queryOwnerships)
                    {
                        LOG("Warning: Ignoring %s, because ownership records are queried!\n", assignment.c_str());
                        continue;
                    }
                    packet.req.byFilter.flags &= ~RequestAssets::anyPossessionManagingContract;
                    if (sscanf(value.c_str(), "%hu", &packet.req.byFilter.possessionManagingContract) != 1)
                    {
                        LOG("Error: Expected unsigned short number for possession managing contract index, found \"%s\"!\n", value.c_str());
                        exit(1);
                    }
                }
                else
                {
                    LOG("Error: Invalid identifier in query string. Found \"%s\"!\n", id.c_str());
                    printAssetRecordsHelpAndExit();
                }
            }
        }
        if (!nameGiven)
        {
            LOG("Error: You need to specify an asset name with name=[NAME]!\n");
            exit(1);
        }
        if (!issuerGiven)
        {
            LOG("Warning: No issuer given, assuming NULL_ID (issued by quorum like contract shares).\n");
        }
    }
    else
    {
        printAssetRecordsHelpAndExit();
    }

    auto qc = make_qc(nodeIp, nodePort);
    qc->sendData((uint8_t*)&packet, packet.header.size());

    bool receivedResponses = false;
    if (withSiblings)
    {
        auto responses = qc->getLatestVectorPacketAs<RespondAssetsWithSiblings>();
        for (const auto& response : responses)
        {
            printAssetResponseWithSiblings(response, verbose);
            receivedResponses = true;
        }
    }
    else
    {
        auto responses = qc->getLatestVectorPacketAs<RespondAssets>();
        for (const auto& response : responses)
        {
            printAssetResponse(response, verbose);
            receivedResponses = true;
        }
    }

    if (!receivedResponses)
    {
        LOG("No assets match your query.\n");
    }

#ifdef _MSC_VER
#undef strcasecmp
#endif
}
