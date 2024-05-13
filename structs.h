#pragma once
#include "defines.h"
#include "utils.h"
#include <cstddef>
#include <cstring>
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
    GET_QUORUM_TICK = 10,
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
    QUOTTERY_ISSUE_BET=25,
    QUOTTERY_JOIN_BET=26,
    QUOTTERY_GET_BET_INFO=27,
    QUOTTERY_GET_BET_DETAIL=28,
    QUOTTERY_GET_ACTIVE_BET=29,
    QUOTTERY_GET_ACTIVE_BET_BY_CREATOR=30,
    QUOTTERY_GET_BET_FEE=31,
    QUOTTERY_PUBLISH_RESULT=32,
    QUOTTERY_CANCEL_BET=33,
    TOOGLE_MAIN_AUX = 34,
    SET_SOLUTION_THRESHOLD=35,
    REFRESH_PEER_LIST=36,
    FORCE_NEXT_TICK=37,
    REISSUE_VOTE=38,
    QUTIL_SEND_TO_MANY_V1=39,
    TOTAL_COMMAND = 40,
    SYNC_TIME = 41,
    GET_SYSTEM_INFO = 42,
    QX_ORDER=43,
    QX_GET_ORDER=44,
    GET_MINING_SCORE_RANKING=45,
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

struct Tick
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

    unsigned long long prevResourceTestingDigest;
    unsigned long long saltedResourceTestingDigest;

    uint8_t prevSpectrumDigest[32];
    uint8_t prevUniverseDigest[32];
    uint8_t prevComputerDigest[32];
    uint8_t saltedSpectrumDigest[32];
    uint8_t saltedUniverseDigest[32];
    uint8_t saltedComputerDigest[32];

    uint8_t transactionDigest[32];
    uint8_t expectedNextTickTransactionDigest[32];

    unsigned char signature[SIGNATURE_SIZE];
    static constexpr unsigned char type()
    {
        return 3;
    }
};

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
    unsigned int initialTick;
} CurrentTickInfo;

typedef struct
{
    short version;
    unsigned short epoch;
    unsigned int tick;
    unsigned int initialTick;
    unsigned int latestCreatedTick;

    unsigned short initialMillisecond;
    unsigned char initialSecond;
    unsigned char initialMinute;
    unsigned char initialHour;
    unsigned char initialDay;
    unsigned char initialMonth;
    unsigned char initialYear;

    unsigned int numberOfEntities;
    unsigned int numberOfTransactions;

    uint8_t randomMiningSeed[32];
    int solutionThreshold;
} CurrentSystemInfo;

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
    enum {
        type = 16,
    };
} RequestTickData;

typedef struct
{
    unsigned int tick;
    unsigned char voteFlags[(676 + 7) / 8];
    enum {
        type = 14,
    };
} RequestedQuorumTick;

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
    static constexpr unsigned char type()
    {
        return 255;
    }
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
    unsigned int universeIndex;
    unsigned char siblings[ASSETS_DEPTH][32];
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
    unsigned int universeIndex;
    unsigned char siblings[ASSETS_DEPTH][32];
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
    unsigned int universeIndex;
    unsigned char siblings[ASSETS_DEPTH][32];
} RespondPossessedAssets;

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

struct QxFees_output
{
    uint32_t assetIssuanceFee; // Amount of qus
    uint32_t transferFee; // Amount of qus
    uint32_t tradeFee; // Number of billionths
};
struct GetSendToManyV1Fee_output
{
    long long fee; // Number of billionths
    static constexpr unsigned char type()
    {
        return 43;
    }
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

struct SpecialCommandToggleMainModeResquestAndResponse
{
    unsigned long long everIncreasingNonceAndCommandType;
    unsigned char mainModeFlag;
    unsigned char padding[7];
    static constexpr unsigned char type()
    {
        return 255;
    }
};
struct SpecialCommandSetSolutionThresholdResquestAndResponse
{
    unsigned long long everIncreasingNonceAndCommandType;
    unsigned int epoch;
    int threshold;
    static constexpr unsigned char type()
    {
        return 255;
    }
};

struct UtcTime
{
    unsigned short    year;              // 1900 - 9999
    unsigned char     month;             // 1 - 12
    unsigned char     day;               // 1 - 31
    unsigned char     hour;              // 0 - 23
    unsigned char     minute;            // 0 - 59
    unsigned char     second;            // 0 - 59
    unsigned char     pad1;
    unsigned int      nanosecond;        // 0 - 999,999,999
};

struct SpecialCommandSendTime
{
    unsigned long long everIncreasingNonceAndCommandType;
    UtcTime utcTime;
    static constexpr unsigned char type()
    {
        return 255;
    }
};

#pragma pack(push, 1)
struct SpecialCommandGetMiningScoreRanking
{
    struct ScoreEntry
    {
        unsigned char minerPublicKey[32];
        unsigned int minerScore;
    };
    unsigned long long everIncreasingNonceAndCommandType;
    unsigned int numRankings;
    std::vector<ScoreEntry> rankings;
    static constexpr unsigned char type()
    {
        return 255;
    }
};
#pragma pack(pop)

#define REQUEST_TX_STATUS 201

struct RequestTxStatus
{
    unsigned int tick;
};

static_assert(sizeof(RequestTxStatus) == 4, "unexpected size");

#define RESPOND_TX_STATUS 202

#pragma pack(push, 1)
struct RespondTxStatus
{
    unsigned int currentTickOfNode;
    unsigned int tick;
    unsigned int txCount;
    unsigned char moneyFlew[(NUMBER_OF_TRANSACTIONS_PER_TICK + 7) / 8];

    // only txCount digests are sent with this message, so only read the first txCount digests when using this as a view to the received data
    uint8_t txDigests[NUMBER_OF_TRANSACTIONS_PER_TICK][32];

    // return size of this struct to be sent (last txDigests are 0 and do not need to be sent)
    unsigned int size() const
    {
        return offsetof(RespondTxStatus, txDigests) + txCount * 32;
    }
};
#pragma pack(pop)