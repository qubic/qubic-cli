#pragma once
#include "structs.h"

#define ESCROW_CONTRACT_INDEX 27

struct EscrowCreateDeal_input {
    uint8_t acceptorId[32];
    struct AssetWithAmount
    {
        uint8_t issuer[32];
        uint64_t name;
        int64_t amount;
    };
    uint64_t offeredQU;
    uint64_t offeredAssetsAmount;
    AssetWithAmount offeredAssets[4];
    uint64_t requestedQU;
    uint64_t requestedAssetsAmount;
    AssetWithAmount requestedAssets[4];
};

struct EscrowGetDeals_input {
    uint8_t owner[32];
    int64_t proposedOffset;
    int64_t publicOffset;
};
struct EscrowGetDeals_output {
    uint64_t ownedDealsAmount;
    uint64_t proposedDealsAmount;
    uint64_t publicDealsAmount;
    struct AssetWithAmount
    {
        uint8_t issuer[32];
        uint64_t name;
        int64_t amount;
    };
    struct Deal
    {
        int64_t index;
        uint8_t acceptorId[32];
        uint64_t offeredQU;
        uint64_t offeredAssetsAmount;
        AssetWithAmount offeredAssets[4];
        uint64_t requestedQU;
        uint64_t requestedAssetsAmount;
        AssetWithAmount requestedAssets[4];
        int16_t creationEpoch;
    };
    Deal ownedDeals[8];
    Deal proposedDeals[32];
    Deal publicDeals[64];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct EscrowOperateDeal_input {
    int64_t index;
};

struct EscrowGetFreeAsset_input {
    uint8_t owner[32];
    struct
    {
        uint8_t issuer[32];
        uint64_t assetName;
    } asset;
};
struct EscrowGetFreeAsset_output {
    uint64_t freeAmount;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct TransferShareManagementRights_input
{
    struct
    {
        uint8_t issuer[32];
        uint64_t assetName;
    } asset;
    int64_t amount;
};
struct TransferShareManagementRights_output
{
    int64_t transferredShares;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

void escrowCreateDeal(const char* nodeIp, int nodePort, const char* seed,
    const char* acceptorId,
    const char* offeredAssetsCommaSeparated,
    const char* requestedAssetsCommaSeparated);
void escrowGetDeals(const char* nodeIp, int nodePort, const char* seed, const int64_t proposedOffset, const int64_t publicOffset);
void escrowAcceptDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t index);
void escrowMakeDealPublic(const char* nodeIp, int nodePort, const char* seed, const int64_t index);
void escrowCancelDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t index);
void escrowTransferRights(const char* nodeIp, int nodePort, const char* seed, const char* assetName, const char* issuer, const int64_t amount);
void escrowOperateDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t index, const int64_t fee, const unsigned short inputType);
void escrowGetFreeAsset(const char* nodeIp, int nodePort, const char* seed, const char* assetName, const char* issuer);

EscrowGetDeals_output escrowGetDealsOutput(const char* nodeIp, int nodePort, const char* seed, const int64_t proposedOffset, const int64_t publicOffset);
int64_t escrowGetRequestedQUForDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t& index);
int64_t escrowGetSharesFeesForDeal(const char* nodeIp, int nodePort, const char* seed, const int64_t& index);
int parseAssets(const std::string& inputStr, EscrowCreateDeal_input::AssetWithAmount* outputArray, const int& maxCount, uint64_t& QUAmount, uint64_t& sharesFees);
void printDeals(int64_t dealsAmount, const EscrowGetDeals_output::Deal* deals, const char* dealTypeName, const char* p1, const char* p2, const char* p3);
