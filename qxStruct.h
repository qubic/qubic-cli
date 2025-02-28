#pragma once

#include "structs.h"

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

struct qxGetAssetOrder_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t offset;
};
static_assert(sizeof(qxGetAssetOrder_input) == sizeof(AssetAskOrders_input), "wrong implementation");
static_assert(sizeof(qxGetAssetOrder_input) == sizeof(AssetBidOrders_input), "wrong implementation");

struct qxGetAssetOrder_output
{
    struct Order
    {
        uint8_t entity[32];
        long long price;
        long long numberOfShares;
    };
    Order orders[256];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

static_assert(sizeof(qxGetAssetOrder_output) == sizeof(AssetAskOrders_output), "wrong implementation");
static_assert(sizeof(qxGetAssetOrder_output) == sizeof(AssetBidOrders_output), "wrong implementation");

struct qxGetEntityOrder_input
{
    uint8_t entity[32];
    uint64_t offset;
};
static_assert(sizeof(qxGetEntityOrder_input) == sizeof(EntityAskOrders_input), "wrong implementation");
static_assert(sizeof(qxGetEntityOrder_input) == sizeof(EntityBidOrders_input), "wrong implementation");

struct qxGetEntityOrder_output
{
    struct Order
    {
        uint8_t issuer[32];
        uint64_t assetName;
        long long price;
        long long numberOfShares;
    };
    Order orders[256];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
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

struct qxOrderAction_input
{
    uint8_t issuer[32];
    uint64_t assetName;
    long long price;
    long long numberOfShares;
};
static_assert(sizeof(qxOrderAction_input) == sizeof(RemoveFromBidOrder_input), "wrong implementation");
static_assert(sizeof(qxOrderAction_input) == sizeof(RemoveFromAskOrder_input), "wrong implementation");
static_assert(sizeof(qxOrderAction_input) == sizeof(AddToBidOrder_input), "wrong implementation");
static_assert(sizeof(qxOrderAction_input) == sizeof(AddToAskOrder_input), "wrong implementation");

#pragma pack (push, 1)
struct qxTransferShareManagementRights_input
{
    struct
    {
        uint8_t issuer[32];
        uint64_t assetName;
    } asset;
    int64_t numberOfShares;
    uint32_t newManagingContractIndex;
};
#pragma pack (pop)
static_assert(sizeof(qxTransferShareManagementRights_input) == 52, "wrong implementation");

#pragma pack (push, 8)
struct QX : ContractBase
{
    uint64_t _earnedAmount;
    uint64_t _distributedAmount;
    uint64_t _burnedAmount;

    uint32_t _assetIssuanceFee; // Amount of qus
    uint32_t _transferFee; // Amount of qus
    uint32_t _tradeFee; // Number of billionths

    struct _AssetOrder
    {
        uint8_t entity[32];
        int64_t numberOfShares;
    };
    collection<_AssetOrder, 2097152 * X_MULTIPLIER> _assetOrders;
    static_assert(sizeof(_assetOrders) == 302514192, "The size of the _assetOrders must be the same with core.");

    struct _EntityOrder
    {
        uint8_t issuer[32];
        uint64_t assetName;
        int64_t numberOfShares;
    };
    collection<_EntityOrder, 2097152 * X_MULTIPLIER> _entityOrders;
    static_assert(sizeof(_entityOrders) == 319291408, "The size of the _entityOrders must be the same with core.");

    // Belows are used for replicate exactly QX struct in core.
    // TODO: change to "locals" variables and remove from state? -> every func/proc can define struct of "locals" that is passed as an argument (stored on stack structure per processor)
    int64_t _elementIndex, _elementIndex2;
    uint8_t _issuerAndAssetName[32];
    _AssetOrder _assetOrder;
    _EntityOrder _entityOrder;
    int64_t _price;
    int64_t _fee;
    AssetAskOrders_output::Order _assetAskOrder;
    AssetBidOrders_output::Order _assetBidOrder;
    EntityAskOrders_output::Order _entityAskOrder;
    EntityBidOrders_output::Order _entityBidOrder;

    struct _TradeMessage
    {
        uint32_t _contractIndex;
        uint32_t _type;

        uint8_t issuer[32];
        uint64_t assetName;
        int64_t price;
        int64_t numberOfShares;

        char _terminator;
    } _tradeMessage;

    struct _NumberOfReservedShares_input
    {
        uint8_t issuer[32];
        uint64_t assetName;
    } _numberOfReservedShares_input;
    struct _NumberOfReservedShares_output
    {
        int64_t numberOfShares;
    } _numberOfReservedShares_output;
};
#pragma pack (pop)

// Match size between cli and core struct. may be changed in the future
static constexpr uint64_t QX_STATE_SIZE = 621806120ULL;
static_assert(sizeof(QX) == QX_STATE_SIZE, "Size of QX must match with core's QX");

//ContractDescription qxContractDescriptions = {"QX", 66, 10000, sizeof(QX)};


