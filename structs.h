#pragma once
#include "defines.h"
#include "utils.h"

enum COMMAND
{
    SHOW_KEYS = 0,
    GET_CURRENT_TICK = 1,
    GET_BALANCE = 2,
    GET_ASSET = 3,
    SEND_COIN = 4,
    SEND_CUSTOM_TX = 5,
    SEND_SPECIAL_COMMAND = 6,
    SEND_RAW_PACKET = 7,
    PUBLISH_PROPOSAL = 8,
    VOTE_PROPOSAL = 9,
    QX_TRANSFER_QXSHARE = 10,
    GET_TICK_DATA=11,
    READ_TICK_DATA=12,
    CHECK_TX_ON_TICK=13,
    CHECK_TX_ON_FILE=14,
    GET_COMP_LIST=15,
    QX_ISSUE_ASSET = 16,
    QX_TRANSFER_ASSET = 17,
    GET_NODE_IP_LIST=18,
    GET_LOG_FROM_NODE = 19,
    DUMP_SPECTRUM_FILE = 20,
    DUMP_UNIVERSE_FILE = 21,
    PRINT_QX_FEE =22,
    MAKE_IPO_BID=23,
    GET_IPO_STATUS=24,
    TOTAL_COMMAND = 25
};

struct RequestResponseHeader {
private:
    uint8_t _size[3];
    uint8_t _type;
    unsigned int _dejavu;

public:
    inline unsigned int size() {
        if (((*((unsigned int*)_size)) & 0xFFFFFF)==0) return INT32_MAX; // size is never zero, zero means broken packets
        return (*((unsigned int*)_size)) & 0xFFFFFF;
    }

    inline void setSize(unsigned int size) {
        _size[0] = (uint8_t)size;
        _size[1] = (uint8_t)(size >> 8);
        _size[2] = (uint8_t)(size >> 16);
    }

    inline bool isDejavuZero()
    {
        return !_dejavu;
    }

    inline void zeroDejavu()
    {
        _dejavu = 0;
    }

    inline void randomizeDejavu()
    {
        rand32(&_dejavu);
        if (!_dejavu)
        {
            _dejavu = 1;
        }
    }

    inline uint8_t type()
    {
        return _type;
    }

    inline void setType(const uint8_t type)
    {
        _type = type;
    }
};
typedef struct
{
    unsigned char publicKey[32];
} RequestedEntity;

struct Entity
{
    unsigned char publicKey[32];
    long long incomingAmount, outgoingAmount;
    unsigned int numberOfIncomingTransfers, numberOfOutgoingTransfers;
    unsigned int latestIncomingTransferTick, latestOutgoingTransferTick;
};
typedef struct
{
    Entity entity;
    unsigned int tick;
    int spectrumIndex;
    unsigned char siblings[SPECTRUM_DEPTH][32];
} RespondedEntity;

typedef struct
{
    unsigned char sourcePublicKey[32];
    unsigned char destinationPublicKey[32];
    long long amount;
    unsigned int tick;
    unsigned short inputType;
    unsigned short inputSize;
} Transaction;

typedef struct
{
    unsigned short tickDuration;
    unsigned short epoch;
    unsigned int tick;
    unsigned short numberOfAlignedVotes;
    unsigned short numberOfMisalignedVotes;
} CurrentTickInfo;

typedef struct
{
    unsigned short computorIndex;
    unsigned short epoch;
    unsigned int tick;

    unsigned short millisecond;
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned char year;

    union
    {
        struct
        {
            unsigned char uriSize;
            unsigned char uri[255];
        } proposal;
        struct
        {
            unsigned char zero;
            unsigned char votes[(676 * 3 + 7) / 8];
            unsigned char quasiRandomNumber;
        } ballot;
    } varStruct;

    unsigned char timelock[32];
    unsigned char transactionDigests[NUMBER_OF_TRANSACTIONS_PER_TICK][32];
    long long contractFees[1024];

    unsigned char signature[SIGNATURE_SIZE];
} TickData;
typedef struct
{
    unsigned int tick;
} RequestedTickData;

typedef struct
{
    RequestedTickData requestedTickData;
} RequestTickData;

typedef struct
{
    unsigned int tick;
    unsigned char transactionFlags[NUMBER_OF_TRANSACTIONS_PER_TICK / 8];
} RequestedTickTransactions;

typedef struct
{
    uint8_t sig[SIGNATURE_SIZE];
} SignatureStruct;
typedef struct
{
    char hash[60];
} TxhashStruct;
typedef struct
{
    std::vector<uint8_t> vecU8;
} extraDataStruct;

struct SpecialCommand
{
    unsigned long long everIncreasingNonceAndCommandType;
};

#define EMPTY 0
#define ISSUANCE 1
#define OWNERSHIP 2
#define POSSESSION 3

struct Asset
{
    union
    {
        struct
        {
            unsigned char publicKey[32];
            unsigned char type;
            char name[7]; // Capital letters + digits
            char numberOfDecimalPlaces;
            char unitOfMeasurement[7]; // Powers of the corresponding SI base units going in alphabetical order
        } issuance;

        struct
        {
            unsigned char publicKey[32];
            unsigned char type;
            char padding[1];
            unsigned short managingContractIndex;
            unsigned int issuanceIndex;
            long long numberOfUnits;
        } ownership;

        struct
        {
            unsigned char publicKey[32];
            unsigned char type;
            char padding[1];
            unsigned short managingContractIndex;
            unsigned int ownershipIndex;
            long long numberOfUnits;
        } possession;
    } varStruct;
};

typedef struct
{
    unsigned char publicKey[32];
} RequestIssuedAssets;

typedef struct
{
    Asset asset;
    unsigned int tick;
    // TODO: Add siblings
} RespondIssuedAssets;

typedef struct
{
    unsigned char publicKey[32];
} RequestOwnedAssets;

typedef struct
{
    Asset asset;
    Asset issuanceAsset;
    unsigned int tick;
    // TODO: Add siblings
} RespondOwnedAssets;

typedef struct
{
    unsigned char publicKey[32];
} RequestPossessedAssets;

typedef struct
{
    Asset asset;
    Asset ownershipAsset;
    Asset issuanceAsset;
    unsigned int tick;
    // TODO: Add siblings
} RespondPossessedAssets;

typedef struct
{
    uint8_t issuer[32];
    uint8_t possessor[32];
    uint8_t newOwner[32];
    unsigned long long assetName;
    long long numberOfUnits;
} TransferAssetOwnershipAndPossession_input;

struct IssueAsset_input
{
    uint64_t name;
    int64_t numberOfUnits;
    uint64_t unitOfMeasurement;
    char numberOfDecimalPlaces;
};

typedef struct
{
    // TODO: Padding
    unsigned short epoch;
    unsigned char publicKeys[NUMBER_OF_COMPUTORS][32];
    unsigned char signature[SIGNATURE_SIZE];
} Computors;

typedef struct
{
    Computors computors;
} BroadcastComputors;

typedef struct
{
    unsigned char peers[4][4];
} ExchangePublicPeers;

struct RequestLog // Fetches log
{
    unsigned long long passcode[4];

    static constexpr unsigned char type()
    {
        return 44;
    }
};

struct RespondLog // Returns buffered log; clears the buffer; make sure you fetch log quickly enough, if the buffer is overflown log stops being written into it till the node restart
{
    // Variable-size log;

    static constexpr unsigned char type()
    {
        return 45;
    }
};

struct RequestContractFunction // Invokes contract function
{
    unsigned int contractIndex;
    unsigned short inputType;
    unsigned short inputSize;
    // Variable-size input

    static constexpr unsigned char type()
    {
        return 42;
    }
};

struct RespondContractFunction // Returns result of contract function invocation
{
    // Variable-size output; the size must be 0 if the invocation has failed for whatever reason (e.g. no a function registered for [inputType], or the function has timed out)

    static constexpr unsigned char type()
    {
        return 43;
    }
};

struct Fees_output
{
    uint32_t assetIssuanceFee; // Amount of qus
    uint32_t transferFee; // Amount of qus
    uint32_t tradeFee; // Number of billionths
};
struct ContractIPOBid
{
    long long price;
    unsigned short quantity;
};
#define REQUEST_CONTRACT_IPO 33
typedef struct
{
    unsigned int contractIndex;
} RequestContractIPO;

#define RESPOND_CONTRACT_IPO 34

typedef struct
{
    unsigned int contractIndex;
    unsigned int tick;
    uint8_t publicKeys[NUMBER_OF_COMPUTORS][32];
    long long prices[NUMBER_OF_COMPUTORS];
} RespondContractIPO;