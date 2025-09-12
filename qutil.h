#pragma once

#include "structs.h"
#include "stdint.h"

// Constants for poll operations
constexpr uint64_t QUTIL_POLL_TYPE_QUBIC = 1;
constexpr uint64_t QUTIL_POLL_TYPE_ASSET = 2;
constexpr uint64_t QUTIL_MAX_ASSETS_PER_POLL = 16;
constexpr uint64_t QUTIL_MAX_POLL = 128;
constexpr uint64_t QUTIL_MAX_OPTIONS = 64;
constexpr int64_t QUTIL_VOTE_FEE = 100LL;
constexpr int64_t QUTIL_POLL_CREATION_FEE = 10000000LL;

namespace qpi
{
    struct Asset
    {
        uint8_t issuer[32];
        uint64_t assetName;
    };
}

struct CreatePoll_input
{
    uint8_t poll_name[32];
    uint64_t poll_type;
    uint64_t min_amount;
    uint8_t github_link[256];
    qpi::Asset allowed_assets[QUTIL_MAX_ASSETS_PER_POLL];
    uint64_t num_assets;
};

struct QUtilPoll {
    uint8_t poll_name[32];
    uint64_t poll_type;
    uint64_t min_amount;
    uint64_t is_active;
    uint8_t creator[32];
    qpi::Asset allowed_assets[QUTIL_MAX_ASSETS_PER_POLL];
    uint64_t num_assets;
};

struct Vote_input
{
    uint64_t poll_id;
    uint8_t address[32];
    uint64_t amount;
    uint64_t chosen_option;
};

struct GetCurrentResult_input
{
    uint64_t poll_id;
};

struct GetPollsByCreator_input
{
    uint8_t creator[32];
};

struct GetPollInfo_input {
    uint64_t poll_id;
};

struct GetCurrentResult_output
{
    uint64_t votes[QUTIL_MAX_OPTIONS];
    uint64_t voter_count[QUTIL_MAX_OPTIONS];
    uint64_t is_active;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetPollsByCreator_output
{
    uint64_t poll_ids[QUTIL_MAX_POLL];
    uint64_t num_polls;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetCurrentPollId_output {
    uint64_t current_poll_id;
    uint64_t active_poll_ids[QUTIL_MAX_POLL];
    uint64_t active_count;
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetPollInfo_output
{
    uint64_t found;
    QUtilPoll poll_info;
    uint8_t poll_link[256];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct CancelPoll_input {
    uint64_t poll_id;
};

enum qutilProcedureId
{
    SendToManyV1 = 1,
    BurnQubic = 2,
    SendToManyBenchmark = 3,
    CreatePoll = 4,
    Vote = 5,
    CancelPoll = 6,
    DistributeQuToShareholders = 7,
};

enum qutilFunctionId
{
    GetSendToManyV1Fee = 1,
    GetTotalNumberOfAssetShares = 2,
    GetCurrentResult = 3,
    GetPollsByCreator = 4,
    GetCurrentPollId = 5,
    GetPollInfo = 6,
};

struct GetSendToManyV1Fee_output
{
    long long fee; // Number of billionths
    static constexpr unsigned char type()
    {
        return 43;
    }
};


void qutilSendToManyV1(const char* nodeIp, int nodePort, const char* seed, const char* payoutListFile, uint32_t scheduledTickOffset);
void qutilBurnQubic(const char* nodeIp, int nodePort, const char* seed, long long amount, uint32_t scheduledTickOffset);
void qutilSendToManyBenchmark(const char* nodeIp, int nodePort, const char* seed, uint32_t destinationCount, uint32_t numTransfersEach, uint32_t scheduledTickOffset);
void qutilGetTotalNumberOfAssetShares(const char* nodeIp, int nodePort, const char* issuerIdentity, const char* assetName);
void qutilDistributeQuToShareholders(const char* nodeIp, int nodePort, const char* seed,
    const char* issuerIdentity, const char* assetName, long long amount,
    uint32_t scheduledTickOffset);

void qutilCreatePoll(const char* nodeIp, int nodePort, const char* seed,
    const char* poll_name, uint64_t poll_type, uint64_t min_amount,
    const char* github_link, const char* semicolon_separated_assets,
    uint32_t scheduledTickOffset);

void qutilVote(const char* nodeIp, int nodePort, const char* seed,
    uint64_t poll_id, uint64_t amount, uint64_t chosen_option,
    uint32_t scheduledTickOffset);

void qutilGetCurrentResult(const char* nodeIp, int nodePort, uint64_t poll_id);

void qutilGetPollsByCreator(const char* nodeIp, int nodePort, const char* creator_address);

void qutilGetCurrentPollId(const char* nodeIp, int nodePort);

void qutilGetPollInfo(const char* nodeIp, int nodePort, uint64_t poll_id);

void qutilCancelPoll(const char* nodeIp, int nodePort, const char* seed, uint64_t poll_id, uint32_t scheduledTickOffset);
