#pragma once

#include "structs.h"
#include "stdint.h"

// Constants for poll operations
constexpr uint64_t QUTIL_POLL_TYPE_QUBIC = 1;
constexpr uint64_t QUTIL_POLL_TYPE_ASSET = 2;
constexpr uint64_t QUTIL_MAX_ASSETS_PER_POLL = 16;
constexpr uint64_t QUTIL_MAX_POLL = 64;
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

struct GetCurrentResult_output
{
    uint64_t poll_id;
    uint64_t votes[QUTIL_MAX_OPTIONS];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

struct GetPollsByCreator_output
{
    uint64_t num_polls;
    uint64_t poll_ids[QUTIL_MAX_POLL];
    static constexpr unsigned char type() {
        return RespondContractFunction::type();
    }
};

enum qutilProcedureId
{
    SendToManyV1 = 1,
    BurnQubic = 2,
    SendToManyBenchmark = 3,
    CreatePoll = 4,
    Vote = 5,
};

enum qutilFunctionId
{
    GetSendToManyV1Fee = 1,
    GetCurrentResult = 2,
    GetPollsByCreator = 3,
};


void qutilSendToManyV1(const char* nodeIp, int nodePort, const char* seed, const char* payoutListFile, uint32_t scheduledTickOffset);
void qutilBurnQubic(const char* nodeIp, int nodePort, const char* seed, long long amount, uint32_t scheduledTickOffset);
void qutilSendToManyBenchmark(const char* nodeIp, int nodePort, const char* seed, uint32_t destinationCount, uint32_t numTransfersEach, uint32_t scheduledTickOffset);

void qutilCreatePoll(const char* nodeIp, int nodePort, const char* seed,
    const char* poll_name, uint64_t poll_type, uint64_t min_amount,
    const char* github_link, const char* comma_separated_asset_names,
    const char* comma_separated_asset_issuers, uint32_t scheduledTickOffset);

void qutilVote(const char* nodeIp, int nodePort, const char* seed,
    uint64_t poll_id, uint64_t amount, uint64_t chosen_option,
    uint32_t scheduledTickOffset);

void qutilGetCurrentResult(const char* nodeIp, int nodePort, uint64_t poll_id);

void qutilGetPollsByCreator(const char* nodeIp, int nodePort, const char* creator_address);
