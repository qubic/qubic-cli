#pragma once

struct IssueAsset_input
{
    uint64_t name;
    int64_t numberOfUnits;
    uint64_t unitOfMeasurement;
    char numberOfDecimalPlaces;
};

struct AssetAskOrders_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t offset;
};
struct AssetAskOrders_output
{
    struct Order
    {
        uint8_t entity[32];
        long long price;
        long long numberOfShares;
    };

    Order orders[256];
};

struct AssetBidOrders_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t offset;
};
struct AssetBidOrders_output
{
    struct Order
    {
        uint8_t entity[32];
        long long price;
        long long numberOfShares;
    };

    Order orders[256];
};

struct EntityAskOrders_input
{
    uint8_t entity[32];
    uint64_t offset;
};
struct EntityAskOrders_output
{
    struct Order
    {
        uint8_t issuer[32];
        uint64_t assetName;
        long long price;
        long long numberOfShares;
    };

    Order orders[256];
};

struct EntityBidOrders_input
{
    uint8_t entity[32];
    uint64_t offset;
};
struct EntityBidOrders_output
{
    struct Order
    {
        uint8_t issuer[32];
        uint64_t assetName;
        long long price;
        long long numberOfShares;
    };

    Order orders[256];
};

struct qxGetAssetOrder_input{
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t offset;
};
static_assert(sizeof(qxGetAssetOrder_input) == sizeof(AssetAskOrders_input), "wrong implementation");
static_assert(sizeof(qxGetAssetOrder_input) == sizeof(AssetBidOrders_input), "wrong implementation");

struct qxGetAssetOrder_output{
    struct Order
    {
        uint8_t entity[32];
        long long price;
        long long numberOfShares;
    };

    Order orders[256];
};

static_assert(sizeof(qxGetAssetOrder_output) == sizeof(AssetAskOrders_output), "wrong implementation");
static_assert(sizeof(qxGetAssetOrder_output) == sizeof(AssetBidOrders_output), "wrong implementation");

struct qxGetEntityOrder_input{
    uint8_t entity[32];
    uint64_t offset;
};
static_assert(sizeof(qxGetEntityOrder_input) == sizeof(EntityAskOrders_input), "wrong implementation");
static_assert(sizeof(qxGetEntityOrder_input) == sizeof(EntityBidOrders_input), "wrong implementation");

struct qxGetEntityOrder_output{
    struct Order
    {
        uint8_t issuer[32];
        uint64_t assetName;
        long long price;
        long long numberOfShares;
    };
    Order orders[256];
};
static_assert(sizeof(qxGetEntityOrder_output) == sizeof(EntityAskOrders_output), "wrong implementation");
static_assert(sizeof(qxGetEntityOrder_output) == sizeof(EntityBidOrders_output), "wrong implementation");



struct TransferAssetOwnershipAndPossession_input
{
    uint8_t issuer[32];
    uint8_t newOwnerAndPossessor[32];
    unsigned long long assetName;
    long long numberOfUnits;
};

struct AddToAskOrder_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    long long price;
    long long numberOfShares;
};
struct AddToAskOrder_output
{
    long long addedNumberOfShares;
};

struct AddToBidOrder_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    long long price;
    long long numberOfShares;
};
struct AddToBidOrder_output
{
    long long addedNumberOfShares;
};

struct RemoveFromAskOrder_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    long long price;
    long long numberOfShares;
};
struct RemoveFromAskOrder_output
{
    long long removedNumberOfShares;
};

struct RemoveFromBidOrder_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    long long price;
    long long numberOfShares;
};
struct RemoveFromBidOrder_output
{
    long long removedNumberOfShares;
};

struct qxOrderAction_input{
    uint8_t issuer[32];
    uint64_t assetName;
    long long price;
    long long numberOfShares;
};
static_assert(sizeof(qxOrderAction_input) == sizeof(RemoveFromBidOrder_input), "wrong implementation");
static_assert(sizeof(qxOrderAction_input) == sizeof(RemoveFromAskOrder_input), "wrong implementation");
static_assert(sizeof(qxOrderAction_input) == sizeof(AddToBidOrder_input), "wrong implementation");
static_assert(sizeof(qxOrderAction_input) == sizeof(AddToAskOrder_input), "wrong implementation");
