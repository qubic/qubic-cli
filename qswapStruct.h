#pragma once

#include "structs.h"

struct QswapIssueAsset_input
{
    uint64_t name;
    int64_t numberOfUnits;
    uint64_t unitOfMeasurement;
    char numberOfDecimalPlaces;
};

struct QswapTransferAssetOwnershipAndPossession_input
{
    uint8_t issuer[32];
    uint8_t newOwnerAndPossessor[32];
    unsigned long long assetName;
    long long numberOfUnits;
};

struct SwapQuForAssetAction_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t assetAmountOut;
};

struct SwapAssetForQuAction_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t assetAmountIn;
    uint64_t quAmountOut;
};

struct CreatePool_input {
    uint8_t issuer[32];
    uint64_t assetName;
};

struct AddLiqudity_input{
    uint8_t issuer[32];
    uint64_t assetName;
    int64_t assetAmountDesired;
    int64_t quAmountMin;
    int64_t assetAmountMin;
};

struct RemoveLiqudity_input{
    uint8_t issuer[32];
    uint64_t assetName;
    int64_t burnLiqudity;
    int64_t quAmountMin;
    int64_t assetAmountMin;
};

struct qswapQuote_input {
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t amount;
};

struct qswapQuote_output {
    uint64_t amount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct qswapGetLiqudityOf_input{
    uint8_t issuer[32];
    uint64_t assetName;
    uint8_t account[32];

};

struct qswapGetLiqudityOf_output{
    uint64_t liqudity;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct qswapGetPoolBasicState_input{
    uint8_t issuer[32];
    uint64_t assetName;
};

struct qswapGetPoolBasicState_output{
    bool poolExists;
    int64_t reservedQuAmount;
    int64_t reservedAssetAmount;
    int64_t totalLiqudity;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};