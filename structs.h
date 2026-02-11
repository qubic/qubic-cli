#pragma once

#include <cstddef>
#include <cstring>

#include "defines.h"
#include "utils.h"

enum COMMAND
{
    SHOW_KEYS,
    GET_CURRENT_TICK,
    GET_BALANCE,
    GET_ASSET,
    SEND_COIN,
    SEND_CUSTOM_TX,
    INVOKE_CONTRACT_PROCEDURE,
    CALL_CONTRACT_FUNCTION,
    SEND_SPECIAL_COMMAND,
    SEND_RAW_PACKET,
    PUBLISH_PROPOSAL,
    VOTE_PROPOSAL,
    GET_QUORUM_TICK,
    GET_TICK_DATA,
    READ_TICK_DATA,
    CHECK_TX_ON_TICK,
    CHECK_TX_ON_FILE,
    GET_COMP_LIST,
    QX_ISSUE_ASSET,
    QX_TRANSFER_ASSET,
    GET_NODE_IP_LIST,
    //GET_LOG_FROM_NODE, // moved to qlogging tool
    DUMP_SPECTRUM_FILE,
    DUMP_UNIVERSE_FILE,
    PRINT_QX_FEE,
    MAKE_IPO_BID,
    GET_IPO_STATUS,
    GET_ACTIVE_IPOS,
    QUOTTERY_ISSUE_BET,
    QUOTTERY_JOIN_BET,
    QUOTTERY_GET_BET_INFO,
    QUOTTERY_GET_BET_DETAIL,
    QUOTTERY_GET_ACTIVE_BET,
    QUOTTERY_GET_ACTIVE_BET_BY_CREATOR,
    QUOTTERY_GET_BASIC_INFO,
    QUOTTERY_PUBLISH_RESULT,
    QUOTTERY_CANCEL_BET,
    TOOGLE_MAIN_AUX,
    SET_SOLUTION_THRESHOLD,
    REFRESH_PEER_LIST,
    FORCE_NEXT_TICK,
    REISSUE_VOTE,
    SYNC_TIME,
    GET_SYSTEM_INFO,
    QX_ORDER,
    QX_GET_ORDER,
    GET_MINING_SCORE_RANKING,
    SEND_COIN_IN_TICK,
    GET_VOTE_COUNTER_TX,
    GQMPROP_SET_PROPOSAL,
    GQMPROP_CLEAR_PROPOSAL,
    GQMPROP_GET_PROPOSALS,
    GQMPROP_VOTE,
    GQMPROP_GET_VOTE,
    GQMPROP_GET_VOTING_RESULTS,
    GQMPROP_GET_REV_DONATION,
    CCF_SET_PROPOSAL,
    CCF_CLEAR_PROPOSAL,
    CCF_GET_PROPOSALS,
    CCF_GET_SUBSCRIPTION,
    CCF_VOTE,
    CCF_GET_VOTE,
    CCF_GET_VOTING_RESULTS,
    CCF_GET_LATEST_TRANSFERS,
    CCF_GET_REGULAR_PAYMENTS,
    DUMP_CONTRACT_FILE,
    GET_TX_INFO,
    UPLOAD_FILE,
    DOWNLOAD_FILE,
    QEARN_LOCK,
    QEARN_UNLOCK,
    QEARN_GET_INFO_PER_EPOCH,
    QEARN_GET_USER_LOCKED_INFO,
    QEARN_GET_STATE_OF_ROUND,
    QEARN_GET_USER_LOCK_STATUS,
    QEARN_GET_UNLOCKING_STATUS,
    QVAULT_SUBMIT_AUTH_ADDRESS,
    QVAULT_CHANGE_AUTH_ADDRESS,
    QVAULT_SUBMIT_FEES,
    QVAULT_CHANGE_FEES,
    QVAULT_SUBMIT_REINVESTING_ADDRESS,
    QVAULT_CHANGE_REINVESTING_ADDRESS,
    QVAULT_GET_DATA,
    QVAULT_SUBMIT_ADMIN_ADDRESS,
    QVAULT_CHANGE_ADMIN_ADDRESS,
    QVAULT_SUBMIT_BANNED_ADDRESS,
    QVAULT_SAVE_BANNED_ADDRESS,
    QVAULT_SUBMIT_UNBANNED_ADDRESS,
    QVAULT_SAVE_UNBANNED_ADDRESS,
    QEARN_GET_STATS_PER_EPOCH,
    QEARN_GET_BURNED_AND_BOOSTED_STATS,
    QEARN_GET_BURNED_AND_BOOSTED_STATS_PER_EPOCH,
    QX_TRANSFER_MANAGEMENT_RIGHTS,
    MSVAULT_REGISTER_VAULT_CMD,
    MSVAULT_DEPOSIT_CMD,
    MSVAULT_RELEASE_TO_CMD,
    MSVAULT_RESET_RELEASE_CMD,
    MSVAULT_GET_VAULTS_CMD,
    MSVAULT_GET_RELEASE_STATUS_CMD,
    MSVAULT_GET_BALANCE_OF_CMD,
    MSVAULT_GET_VAULT_NAME_CMD,
    MSVAULT_GET_REVENUE_INFO_CMD,
    MSVAULT_GET_FEES_CMD,
    MSVAULT_GET_OWNERS_CMD,
    QUERY_ASSETS,
    TEST_QPI_FUNCTIONS_OUTPUT,
    SET_LOGGING_MODE,
    TEST_QPI_FUNCTIONS_OUTPUT_PAST,
    COMP_CHAT,
    TEST_GET_INCOMING_TRANSFER_AMOUNTS,
    TEST_BID_IN_IPO_THROUGH_CONTRACT,
    GET_TOTAL_NUMBER_OF_ASSET_SHARES,
    QUTIL_BURN_QUBIC,
    QUTIL_SEND_TO_MANY_V1,
    QUTIL_SEND_TO_MANY_BENCHMARK,
    QUTIL_CREATE_POLL,
    QUTIL_VOTE,
    QUTIL_GET_CURRENT_RESULT,
    QUTIL_GET_POLLS_BY_CREATOR,
    QUTIL_GET_CURRENT_POLL_ID,
    QUTIL_GET_POLL_INFO,
    QUTIL_CANCEL_POLL,
    QUTIL_DISTRIBUTE_QU_TO_SHAREHOLDERS,
    QUTIL_PRINT_FEE,
    QUTIL_BURN_QUBIC_FOR_CONTRACT,
    QUTIL_QUERY_FEE_RESERVE,
    PRINT_QSWAP_FEE,
    QSWAP_ISSUE_ASSET,
    QSWAP_TRANSFER_ASSET,
    QSWAP_TRANSFER_ASSET_RIGHTS,
    QSWAP_CREATE_POOL,
    QSWAP_GET_POOL_BASIC,
    QSWAP_ADD_LIQUIDITY,
    QSWAP_REMOVE_LIQUIDITY,
    QSWAP_GET_LIQUIDITY_OF,
    QSWAP_SWAP_EXACT_QU_FOR_ASSET,
    QSWAP_SWAP_QU_FOR_EXACT_ASSET,
    QSWAP_SWAP_EXACT_ASSET_FOR_QU,
    QSWAP_SWAP_ASSET_FOR_EXACT_QU,
    QSWAP_QUOTE,
    NOSTROMO_REGISTER_IN_TIER,
    NOSTROMO_LOGOUT_FROM_TIER,
    NOSTROMO_CREATE_PROJECT,
    NOSTROMO_VOTE_IN_PROJECT,
    NOSTROMO_CREATE_FUNDRAISING,
    NOSTROMO_INVEST_IN_FUNDRAISING,
    NOSTROMO_CLAIM_TOKEN,
    NOSTROMO_GET_STATS,
    NOSTROMO_GET_TIER_LEVEL_BY_USER,
    NOSTROMO_GET_USER_VOTE_STATUS,
    NOSTROMO_CHECK_TOKEN_CREATABILITY,
    NOSTROMO_GET_NUMBER_OF_INVESTED_PROJECTS,
    NOSTROMO_GET_PROJECT_BY_INDEX,
    NOSTROMO_GET_FUNDRAISING_BY_INDEX,
    NOSTROMO_GET_PROJECT_INDEX_LIST_BY_CREATOR,
    NOSTROMO_UPGRADE_TIER_LEVEL,
    NOSTROMO_TRANSFER_SHARE_MANAGEMENT_RIGHTS,
    NOSTROMO_GET_INFO_USER_INVESTED,
    NOSTROMO_GET_MAX_CLAIM_AMOUNT,
    MSVAULT_DEPOSIT_ASSET_CMD,
    MSVAULT_RELEASE_ASSET_TO_CMD,
    MSVAULT_RESET_ASSET_RELEASE_CMD,
    MSVAULT_GET_ASSET_BALANCES_CMD,
    MSVAULT_GET_ASSET_RELEASE_STATUS_CMD,
    MSVAULT_GET_MANAGED_ASSET_BALANCE_CMD,
    MSVAULT_REVOKE_ASSET_RIGHTS_CMD,
    MSVAULT_IS_SHAREHOLDER_CMD,
    MSVAULT_VOTE_FEE_CHANGE_CMD,
    MSVAULT_GET_FEE_VOTES_CMD,
    MSVAULT_GET_FEE_VOTES_OWNER_CMD,
    MSVAULT_GET_FEE_VOTES_SCORE_CMD,
    MSVAULT_GET_UNIQUE_FEE_VOTES_CMD,
    MSVAULT_GET_UNIQUE_FEE_VOTES_RANKING_CMD,
    QBOND_STAKE_CMD,
    QBOND_TRANSFER_CMD,
    QBOND_ADD_ASK_ORDER_CMD,
    QBOND_REMOVE_ASK_ORDER_CMD,
    QBOND_ADD_BID_ORDER_CMD,
    QBOND_REMOVE_BID_ORDER_CMD,
    QBOND_BURN_QU_CMD,
    QBOND_UPDATE_CFA_CMD,
    QBOND_GET_FEES_CMD,
    QBOND_GET_EARNED_FEES_CMD,
    QBOND_GET_EPOCH_INFO_CMD,
    QBOND_GET_ORDERS_CMD,
    QBOND_GET_USER_ORDERS_CMD,
    QBOND_GET_TABLE_CMD,
    QBOND_GET_USER_MBONDS_CMD,
    QBOND_GET_CFA_CMD,
    SAVE_SNAPSHOT,
    SHAREHOLDER_SET_PROPOSAL,
    SHAREHOLDER_CLEAR_PROPOSAL,
    SHAREHOLDER_GET_PROPOSALS,
    SHAREHOLDER_VOTE,
    SHAREHOLDER_GET_VOTE,
    SHAREHOLDER_GET_VOTING_RESULTS,
    SET_EXECUTION_FEE_MULTIPLIER,
    GET_EXECUTION_FEE_MULTIPLIER,
    GET_ORACLE_QUERY,
    SEND_ORACLE_QUERY_TX,
    TOTAL_COMMAND // DO NOT CHANGE THIS
};

struct RequestResponseHeader 
{
private:
    uint8_t _size[3];
    uint8_t _type;
    unsigned int _dejavu;

public:
    inline unsigned int size() const
    {
        if (((*((unsigned int*)_size)) & 0xFFFFFF)==0) return INT32_MAX; // size is never zero, zero means broken packets
        return (*((unsigned int*)_size)) & 0xFFFFFF;
    }

    inline void setSize(unsigned int size)
    {
        _size[0] = (uint8_t)size;
        _size[1] = (uint8_t)(size >> 8);
        _size[2] = (uint8_t)(size >> 16);
    }

    inline unsigned int dejavu() const
    {
        return _dejavu;
    }

    inline bool isDejavuZero() const
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

    inline uint8_t type() const
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

    unsigned int prevResourceTestingDigest;
    unsigned int saltedResourceTestingDigest;

    unsigned int prevTransactionBodyDigest;
    unsigned int saltedTransactionBodyDigest;

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

struct CurrentTickInfo
{
    unsigned short tickDuration;
    unsigned short epoch;
    unsigned int tick;
    unsigned short numberOfAlignedVotes;
    unsigned short numberOfMisalignedVotes;
    unsigned int initialTick;

    static constexpr unsigned char type()
    {
        return RESPOND_CURRENT_TICK_INFO;
    }
};

#pragma pack(push, 1)
struct CurrentSystemInfo
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

    unsigned long long totalSpectrumAmount;

    // Entity balances less or euqal this value will be burned if number of entites rises to 75% of spectrum capacity.
    // Starts to be meaningful if >50% of spectrum is filled but may still change after that.
    unsigned long long currentEntityBalanceDustThreshold;

    unsigned int targetTickVoteSignature;
    unsigned long long computorPacketSignature;
    unsigned long long solutionAdditionalThreshold;
    unsigned long long _reserve2;
    unsigned long long _reserve3;
    unsigned long long _reserve4;

    static constexpr unsigned char type()
    {
        return RESPOND_SYSTEM_INFO;
    }
};
#pragma pack(pop)

struct TickData
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

    unsigned char timelock[32];
    unsigned char transactionDigests[NUMBER_OF_TRANSACTIONS_PER_TICK][32];
    long long contractFees[1024];

    unsigned char signature[SIGNATURE_SIZE];

    static constexpr unsigned char type()
    {
        return BROADCAST_FUTURE_TICK_DATA;
    }
};

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
    unsigned char transactionDigest[32];
} RequestedTransactionInfo;

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
    uint8_t ptr[32];
} TxHash32Struct;

typedef struct
{
    uint8_t digest[32];
} TxDigestStruct;

typedef struct
{
    std::vector<uint8_t> vecU8;
} ExtraDataStruct;

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

struct AssetRecord
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
            long long numberOfShares;
        } ownership;

        struct
        {
            unsigned char publicKey[32];
            unsigned char type;
            char padding[1];
            unsigned short managingContractIndex;
            unsigned int ownershipIndex;
            long long numberOfShares;
        } possession;
    } varStruct;
};

typedef struct
{
    unsigned char publicKey[32];
} RequestIssuedAssets;

typedef struct
{
    AssetRecord asset;
    unsigned int tick;
    unsigned int universeIndex;
    unsigned char siblings[ASSETS_DEPTH][32];
} RespondIssuedAssets;

typedef struct
{
    unsigned char publicKey[32];
} RequestOwnedAssets;

struct RespondOwnedAssets
{
    AssetRecord asset;
    AssetRecord issuanceAsset;
    unsigned int tick;
    unsigned int universeIndex;
    unsigned char siblings[ASSETS_DEPTH][32];

    static constexpr unsigned char type()
    {
        return RESPOND_OWNED_ASSETS;
    }
};

typedef struct
{
    unsigned char publicKey[32];
} RequestPossessedAssets;

struct RespondPossessedAssets
{
    AssetRecord asset;
    AssetRecord ownershipAsset;
    AssetRecord issuanceAsset;
    unsigned int tick;
    unsigned int universeIndex;
    unsigned char siblings[ASSETS_DEPTH][32];

    static constexpr unsigned char type()
    {
        return RESPOND_POSSESSED_ASSETS;
    }
};


// Options to request assets:
// - all issued asset records, optionally with filtering by issuer and/or name
// - all ownership records of a specific asset type, optionally with filtering by owner and managing contract
// - all possession records of a specific asset type, optionally with filtering by possessor and managing contract
// - by universeIdx (set issuer and asset name to 0)
union RequestAssets
{
    static constexpr unsigned char type()
    {
        return 52;
    }

    // type of asset request
    static constexpr unsigned short requestIssuanceRecords = 0;
    static constexpr unsigned short requestOwnershipRecords = 1;
    static constexpr unsigned short requestPossessionRecords = 2;
    static constexpr unsigned short requestByUniverseIdx = 3;
    unsigned short assetReqType;

    // common flags
    static constexpr unsigned short getSiblings = 0b1;

    // flags of requestIssuanceRecords
    static constexpr unsigned short anyIssuer = 0b10;
    static constexpr unsigned short anyAssetName = 0b100;

    // flags of requestOwnershipRecords
    static constexpr unsigned short anyOwner = 0b1000;
    static constexpr unsigned short anyOwnershipManagingContract = 0b10000;

    // flags of requestOwnershipRecords and requestPossessionRecords
    static constexpr unsigned short anyPossessor = 0b100000;
    static constexpr unsigned short anyPossessionManagingContract = 0b1000000;

    // data of type requestIssuanceRecords, requestOwnershipRecords, and requestPossessionRecords
    struct
    {
        unsigned short assetReqType;
        unsigned short flags;
        unsigned short ownershipManagingContract;
        unsigned short possessionManagingContract;
        unsigned char issuer[32];
        unsigned long long assetName;
        unsigned char owner[32];
        unsigned char possessor[32];
    } byFilter;

    // data of type requestByUniverseIdx
    struct
    {
        unsigned short assetReqType;
        unsigned short flags;
        unsigned int universeIdx;
    } byUniverseIdx;
};

static_assert(sizeof(RequestAssets) == 112, "Something is wrong with the struct size.");


// Response message after RequestAssets without flag getSiblings
struct RespondAssets
{
    AssetRecord asset;
    unsigned int tick;
    unsigned int universeIndex;

    static constexpr unsigned char type()
    {
        return 53;
    }
};
static_assert(sizeof(RespondAssets) == 56, "Something is wrong with the struct size.");

// Response message after RequestAssets with flag getSiblings
struct RespondAssetsWithSiblings : public RespondAssets
{
    unsigned char siblings[ASSETS_DEPTH][32];
};
static_assert(sizeof(RespondAssetsWithSiblings) == 824, "Something is wrong with the struct size.");


typedef struct
{
    // TODO: Padding
    unsigned short epoch;
    unsigned char publicKeys[NUMBER_OF_COMPUTORS][32];
    unsigned char signature[SIGNATURE_SIZE];
} Computors;

struct BroadcastComputors
{
    Computors computors;

    static constexpr unsigned char type()
    {
        return BROADCAST_COMPUTORS;
    }
};

struct ExchangePublicPeers
{
    unsigned char peers[4][4];

    static constexpr unsigned char type()
    {
        return EXCHANGE_PUBLIC_PEERS;
    }
};

struct RequestComputors
{
    static constexpr unsigned char type()
    {
        return REQUEST_COMPUTORS;
    }
};

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

struct RespondContractIPO
{
    unsigned int contractIndex;
    unsigned int tick;
    uint8_t publicKeys[NUMBER_OF_COMPUTORS][32];
    long long prices[NUMBER_OF_COMPUTORS];

    static constexpr unsigned char type()
    {
        return RESPOND_CONTRACT_IPO;
    }
};

struct RequestActiveIPOs
{
    static constexpr unsigned char type()
    {
        return 64;
    }
};


struct RespondActiveIPO
{
    unsigned int contractIndex;
    char assetName[8];

    static constexpr unsigned char type()
    {
        return 65;
    }
};

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
    int algoType;
    int padding;
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

struct SpecialCommandSetConsoleLoggingModeRequestAndResponse
{
    unsigned long long everIncreasingNonceAndCommandType;
    unsigned char loggingMode; // 0 disabled, 1 low computational cost, 2 full logging
    unsigned char padding[7];

    static constexpr unsigned char type()
    {
        return 255;
    }
};

struct SpecialCommandSaveSnapshotRequestAndResponse
{
    enum
    {
        SAVING_TRIGGERED = 0,
        SAVING_IN_PROGRESS,
        REMOTE_SAVE_MODE_DISABLED,
        UNKNOWN_FAILURE,
    };

    unsigned long long everIncreasingNonceAndCommandType;
    unsigned int currentTick;
    unsigned char status;
    unsigned char padding[3];
    
    static constexpr unsigned char type()
    {
        return 255;
    }
};

// This struct is used as response for the get command and as request and response for the set command.
struct SpecialCommandExecutionFeeMultiplierRequestAndResponse
{
    unsigned long long everIncreasingNonceAndCommandType;
    unsigned long long multiplierNumerator;
    unsigned long long multiplierDenominator;

    static constexpr unsigned char type()
    {
        return 255;
    }
};

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

    static constexpr unsigned char type()
    {
        return RESPOND_TX_STATUS;
    }
};
#pragma pack(pop)


enum SCType
{
    SC_TYPE_Contract0State = 0,
    SC_TYPE_QX = 1,
    SC_TYPE_QTRY = 2,
    SC_TYPE_RANDOM = 3,
    SC_TYPE_QUTIL = 4,
    SC_TYPE_MLM = 5,
    SC_TYPE_GQMPROP = 6,
    SC_TYPE_SWATCH = 7,
    SC_TYPE_CCF = 8,
    SC_TYPE_MAX
};

// Generate contract related structs
constexpr uint64_t X_MULTIPLIER = 1ULL;
struct ContractDescription
{
    char assetName[8];
    // constructionEpoch needs to be set to after IPO (IPO is before construction)
    uint16_t constructionEpoch, destructionEpoch;
    uint64_t stateSize;
};

struct ContractBase
{
};

template <typename T, uint64_t L>
struct array
{
    static_assert(L && !(L & (L - 1)),
        "The capacity of the array must be 2^N."
        );
    T _values[L];
};

static const int64_t COLLECTION_NULL_INDEX = -1LL;

// Simplified collection from core
template <typename T, uint64_t L>
struct collection
{
public:

    // Return maximum number of elements that may be stored.
    static constexpr uint64_t capacity() { return L; }

    // Return total populations.
    uint64_t population() { return _population; }

    // Return element value at elementIndex.
    inline T element(int64_t elementIndex) const { return _elements[elementIndex & (L - 1)].value; }

    // Return overall number of elements.
    inline uint64_t population() const { return _population; }

    // Return point of view elementIndex belongs to (or 0 id if unused).
    void pov(int64_t elementIndex, uint8_t* povID) const
    {
        memcpy(povID, _povs[_elements[elementIndex & (L - 1)].povIndex].value, 32);
    }

    // Return priority of elementIndex (or 0 id if unused).
    int64_t priority(int64_t elementIndex) const
    {
        return _elements[elementIndex & (L - 1)].priority;
    }

private:
    static_assert(L && !(L & (L - 1)), "The capacity of the collection must be 2^N.");
    static constexpr int64_t _nEncodedFlags = L > 32 ? 32 : L;

    // Hash map of point of views = element filters, each with one priority queue (or empty)
    struct PoV
    {
        uint8_t value[32];
        uint64_t population;
        int64_t headIndex, tailIndex;
        int64_t bstRootIndex;
    } _povs[L];

    // 2 bits per element of _povs: 0b00 = not occupied; 0b01 = occupied; 0b10 = occupied but marked
    // for removal; 0b11 is unused The state "occupied but marked for removal" is needed for finding
    // the index of a pov in the hash map. Setting an entry to "not occupied" in remove() would
    // potentially undo a collision, create a gap, and mess up the entry search.
    uint64_t _povOccupationFlags[(L * 2 + 63) / 64];

    // Array of elements (filled sequentially), each belongs to one PoV / priority queue (or is
    // empty) Elements of a POV entry will be stored as a binary search tree (BST); so this
    // structure has some properties related to BST (bstParentIndex, bstLeftIndex, bstRightIndex).
    struct Element
    {
        T value;
        int64_t priority;
        int64_t povIndex;
        int64_t bstParentIndex;
        int64_t bstLeftIndex;
        int64_t bstRightIndex;
    } _elements[L];
    uint64_t _population;
    uint64_t _markRemovalCounter;
};

struct FileHeaderTransaction : public Transaction
{
    static constexpr unsigned char transactionType()
    {
        return 3; // TODO: Set actual value
    }

    static constexpr long long minAmount()
    {
        return 0;
    }

    static constexpr unsigned short minInputSize()
    {
        return sizeof(fileSize)
               + sizeof(numberOfFragments)
               + sizeof(fileFormat);
    }

    unsigned long long fileSize;
    unsigned long long numberOfFragments;
    unsigned char fileFormat[8];
    unsigned char signature[SIGNATURE_SIZE];
};

struct FileFragmentTransactionPrefix : public Transaction
{
    static constexpr unsigned char transactionType()
    {
        return 4; // TODO: Set actual value
    }

    static constexpr long long minAmount()
    {
        return 0;
    }

    static constexpr unsigned short minInputSize()
    {
        return sizeof(fragmentIndex)
               + sizeof(prevFileFragmentTransactionDigest);
    }

    unsigned long long fragmentIndex;
    uint8_t prevFileFragmentTransactionDigest[32];
};
