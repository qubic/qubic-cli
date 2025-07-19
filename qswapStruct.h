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
    int64_t assetName;
    uint8_t newOwnerAndPossessor[32];
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

struct AddLiquidity_input{
    uint8_t issuer[32];
    uint64_t assetName;
    int64_t assetAmountDesired;
    int64_t quAmountMin;
    int64_t assetAmountMin;
};

struct RemoveLiquidity_input{
    uint8_t issuer[32];
    uint64_t assetName;
    int64_t burnLiquidity;
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

struct qswapGetLiquidityOf_input{
    uint8_t issuer[32];
    uint64_t assetName;
    uint8_t account[32];

};

struct qswapGetLiquidityOf_output{
    uint64_t liquidity;

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
    int64_t poolExists;
    int64_t reservedQuAmount;
    int64_t reservedAssetAmount;
    int64_t totalLiquidity;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};