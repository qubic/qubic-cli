#include <cstdlib>
#include <cinttypes>

#include "core/src/network_messages/common_def.h"
#include "core/src/network_messages/oracles.h"

#include "oracle_interface_adapter.h"

#include "oracle_utils.h"
#include "logger.h"
#include "utils.h"
#include "structs.h"
#include "connection.h"
#include "key_utils.h"

// make sure these variables match the local variables from the core code 
// in OracleEngine::processRequestOracleData(...) defined in core/src/oracle_core/net_msg_impl.h
constexpr int maxQueryIdCount = 2;
constexpr int payloadBufferSize = std::max((int)std::max(MAX_ORACLE_QUERY_SIZE, MAX_ORACLE_REPLY_SIZE), maxQueryIdCount * 8);
static_assert(payloadBufferSize >= sizeof(RespondOracleDataQueryMetadata), "Buffer too small.");
static_assert(payloadBufferSize < 32 * 1024, "Large alloc in stack may need reconsideration.");

void printGetOracleQueryHelpAndExit()
{
    LOG("qubic-cli [...] -getoraclequery [QUERY_ID]\n");
    LOG("    Print the oracle query, metadata, and reply if available.\n");
    LOG("qubic-cli [...] -getoraclequery pending\n");
    LOG("    Print the query IDs for all pending queries.\n");
    LOG("qubic-cli [...] -getoraclequery pending+\n");
    LOG("    Print the oracle query, metadata, and reply if available for each query ID received by pending.\n");
    LOG("qubic-cli [...] -getoraclequery all [TICK]\n");
    LOG("    Print the query IDs of all queries started in the given tick.\n");
    LOG("qubic-cli [...] -getoraclequery all+ [TICK]\n");
    LOG("    Print the oracle query, metadata, and reply if available for each query ID received by all.\n");
    LOG("qubic-cli [...] -getoraclequery user [TICK]\n");
    LOG("    Print the query IDs of user queries started in the given tick.\n");
    LOG("qubic-cli [...] -getoraclequery user+ [TICK]\n");
    LOG("    Print the oracle query, metadata, and reply if available for each query ID received by user.\n");
    LOG("qubic-cli [...] -getoraclequery contract [TICK]\n");
    LOG("    Print the query IDs of contract one-time queries started in the given tick.\n");
    LOG("qubic-cli [...] -getoraclequery contract+ [TICK]\n");
    LOG("    Print the oracle query, metadata, and reply if available for each query ID received by contract.\n");
    LOG("qubic-cli [...] -getoraclequery subscription [TICK]\n");
    LOG("    Print the query IDs of contract subscription queries started in the given tick.\n");
    LOG("qubic-cli [...] -getoraclequery subscription+ [TICK]\n");
    LOG("    Print the oracle query, metadata, and reply if available for each query ID received by subscription.\n");
    LOG("qubic-cli [...] -getoraclequery stats\n");
    LOG("    Print the oracle query statistics of the core node.\n");
    exit(1);
}

static std::vector<int64_t> receiveQueryIds(QCPtr qc, unsigned int reqType, long long reqTickOrId = 0)
{   
    struct {
        RequestResponseHeader header;
        RequestOracleData req;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestOracleData::type());
    memset(&packet.req, 0, sizeof(packet.req));
    packet.req.reqType = reqType;
    packet.req.reqTickOrId = reqTickOrId;

    qc->sendData((uint8_t*)&packet, packet.header.size());

    std::vector<int64_t> queryIds;

    constexpr unsigned long long bufferSize = sizeof(RequestResponseHeader) + sizeof(RespondOracleData) + maxQueryIdCount * sizeof(int64_t);
    uint8_t buffer[bufferSize];
    int recvByte = qc->receiveData(buffer, sizeof(RequestResponseHeader));

    while (recvByte == sizeof(RequestResponseHeader))
    {
        auto header = (RequestResponseHeader*)buffer;
        if (header->dejavu() != packet.header.dejavu())
        {
            throw std::runtime_error("Unexpected dejavue!");
        }
        if (header->type() == RespondOracleData::type())
        {
            recvByte = qc->receiveAllDataOrThrowException(buffer + sizeof(RequestResponseHeader), header->size() - sizeof(RequestResponseHeader));
            auto resp = (RespondOracleData*)(buffer + sizeof(RequestResponseHeader));
            if (resp->resType == RespondOracleData::respondQueryIds)
            {
                long long payloadNumBytes = header->size() - sizeof(RequestResponseHeader) - sizeof(RespondOracleData);
                if (payloadNumBytes % 8 != 0)
                {
                    throw std::runtime_error("Malformatted RespondOracleData::respondQueryIds messge!");

                }
                else if (payloadNumBytes > 0)
                {
                    const uint8_t* queryIdBuffer = buffer + sizeof(RequestResponseHeader) + sizeof(RespondOracleData);
                    queryIds.insert(queryIds.end(),
                        (int64_t*)queryIdBuffer, (int64_t*)(queryIdBuffer + payloadNumBytes));
                }
            }
        }
        else if (header->type() == END_RESPOND)
        {
            return queryIds;
        }
        else
        {
            throw std::runtime_error("Unexpected packet type!");
        }
        recvByte = qc->receiveData(buffer, sizeof(RequestResponseHeader));
    }

    throw ConnectionTimeout();
}

static void receiveQueryInformation(QCPtr qc, int64_t queryId, RespondOracleDataQueryMetadata& metadata, std::vector<uint8_t>& query, std::vector<uint8_t>& reply)
{
    // send request
    struct {
        RequestResponseHeader header;
        RequestOracleData req;
    } request;
    request.header.setSize(sizeof(request));
    request.header.randomizeDejavu();
    request.header.setType(RequestOracleData::type());
    memset(&request.req, 0, sizeof(request.req));
    request.req.reqType = RequestOracleData::requestQueryAndResponse;
    request.req.reqTickOrId = queryId;
    qc->sendData((uint8_t*)&request, request.header.size());

    // reset output
    memset(&metadata, 0, sizeof(RespondOracleDataQueryMetadata));
    query.clear();
    reply.clear();

    // prepare output buffer
    uint8_t buffer[sizeof(RequestResponseHeader) + payloadBufferSize];
    auto responseHeader = (RequestResponseHeader*)buffer;
    auto responsePayload = buffer + sizeof(RequestResponseHeader);

    // receive query data
    int recvHeaderBytes = qc->receiveData(buffer, sizeof(RequestResponseHeader));
    while (recvHeaderBytes == sizeof(RequestResponseHeader))
    {
        // get remaining part of the response
        const unsigned int responsePayloadSize = responseHeader->size() - sizeof(RequestResponseHeader);
        qc->receiveAllDataOrThrowException(responsePayload, responsePayloadSize);

        // only process if dejavu matches (response is to current request, skip otherwise)
        if (responseHeader->dejavu() == request.header.dejavu())
        {
            if (responseHeader->type() == RespondOracleData::type())
            {
                // Oracle data response
                if (responsePayloadSize < sizeof(RespondOracleData))
                {
                    throw std::runtime_error("Malformatted RespondOracleData reply");
                }
                auto respOracleData = (RespondOracleData*)responsePayload;
                auto responseInnerPayload = responsePayload + sizeof(RespondOracleData);
                auto responseInnerPayloadSize = responsePayloadSize - sizeof(RespondOracleData);

                if (respOracleData->resType == RespondOracleData::respondQueryMetadata
                    && responsePayloadSize == sizeof(RespondOracleData) + sizeof(RespondOracleDataQueryMetadata))
                {
                    // Query metadata
                    metadata = *(RespondOracleDataQueryMetadata*)(responseInnerPayload);
                }
                else if (respOracleData->resType == RespondOracleData::respondQueryData)
                {
                    // Oracle query
                    query.insert(query.end(),
                        responseInnerPayload,
                        responseInnerPayload + responseInnerPayloadSize);
                }
                else if (respOracleData->resType == RespondOracleData::respondReplyData)
                {
                    // Oracle reply
                    reply.insert(reply.end(),
                        responseInnerPayload,
                        responseInnerPayload + responseInnerPayloadSize);
                }
                else
                {
                    throw std::runtime_error("Unexpected RespondOracleData message sub-type");
                }
            }
            else if (responseHeader->type() == END_RESPOND)
            {
                // End of output packages for this request
                if (metadata.queryId != queryId)
                    throw std::logic_error("Unknown query ID!");
                else
                    break;
            }

            // try to get next message header
            recvHeaderBytes = qc->receiveData(buffer, sizeof(RequestResponseHeader));
        }
    }
    if (recvHeaderBytes < sizeof(RequestResponseHeader))
    {
        throw std::logic_error("Error receiving message header.");
    }
}

static const char* getOracleQueryTypeStr(uint8_t type)
{
    switch (type)
    {
    case ORACLE_QUERY_TYPE_CONTRACT_QUERY:
        return "contract one-time query";
    case ORACLE_QUERY_TYPE_CONTRACT_SUBSCRIPTION:
        return "contract subscription";
    case ORACLE_QUERY_TYPE_USER_QUERY:
        return "user query";
    default:
        return "unknown";
    }
}

static const char* getOracleQueryStatusStr(uint8_t type)
{
    switch (type)
    {
    case ORACLE_QUERY_STATUS_PENDING:
        return "pending";
    case ORACLE_QUERY_STATUS_COMMITTED:
        return "committed";
    case ORACLE_QUERY_STATUS_SUCCESS:
        return "success";
    case ORACLE_QUERY_STATUS_UNRESOLVABLE:
        return "unresolvable";
    case ORACLE_QUERY_STATUS_TIMEOUT:
        return "timeout";
    default:
        return "unknown";
    }
}

static std::string getOracleQueryStatusFlagsStr(uint16_t flags)
{
    std::string str;
    if (flags & ORACLE_FLAG_REPLY_RECEIVED)
        str += "-> core node received valid reply from the oracle machine";
    if (flags & ORACLE_FLAG_INVALID_ORACLE)
        str += "\n\t- oracle machine reported that oracle (data source) in query was invalid";
    if (flags & ORACLE_FLAG_ORACLE_UNAVAIL)
        str += "\n\t- oracle machine reported that oracle (data source) isn't available at the moment";
    if (flags & ORACLE_FLAG_INVALID_TIME)
        str += "\n\t- oracle machine reported that time in query was invalid";
    if (flags & ORACLE_FLAG_INVALID_PLACE)
        str += "\n\t- oracle machine reported that place in query was invalid";
    if (flags & ORACLE_FLAG_INVALID_ARG)
        str += "\n\t- oracle machine reported that an argument in query was invalid";
    if (flags & ORACLE_FLAG_BAD_SIZE_REPLY)
        str += "\n\t- core node got reply of wrong size from the oracle machine";
    if (flags & ORACLE_FLAG_OM_DISAGREE)
        str += "\n\t- core node got different replies from oracle machine nodes";
    if (flags & ORACLE_FLAG_BAD_SIZE_REVEAL)
        str += "\n\t- weren't enough reply commit tx with the same digest before timeout (< 451)";

    return str;
}

static void printQueryInformation(const RespondOracleDataQueryMetadata& metadata, const std::vector<uint8_t>& query, const std::vector<uint8_t>& reply)
{
    LOG("Query ID: %" PRIi64 "\n", metadata.queryId);
    LOG("Type: %s (%" PRIu8 ")\n", getOracleQueryTypeStr(metadata.type), metadata.type);
    LOG("Status: %s (%" PRIu8 ")\n", getOracleQueryStatusStr(metadata.status), metadata.status);
    LOG("Status Flags: %" PRIu16 " %s\n", metadata.statusFlags, getOracleQueryStatusFlagsStr(metadata.statusFlags).c_str());
    LOG("Query Tick: %" PRIu32 "\n", metadata.queryTick);

    char queryingIdentity[128] = { 0 };
    getIdentityFromPublicKey(metadata.queryingEntity.m256i_u8, queryingIdentity, /*isLowerCase=*/false);
    LOG("Querying Entity: %s\n", queryingIdentity);

    LOG("Timeout: %s\n", toString(*(QPI::DateAndTime*)&metadata.timeout).c_str());
    LOG("Interface Index: %" PRIu32 "\n", metadata.interfaceIndex);
    if (metadata.type == ORACLE_QUERY_TYPE_CONTRACT_SUBSCRIPTION)
        LOG("Subscription ID: %" PRIi32 "\n", metadata.subscriptionId);
    if (metadata.status == ORACLE_QUERY_STATUS_SUCCESS)
    {
        LOG("Reveal Tick: %" PRIu32 "\n", metadata.revealTick);
    }
    else
    {
        LOG("Total Commits: %" PRIu16 "\n", metadata.totalCommits);
        LOG("Agreeing Commits: %" PRIu16 "\n", metadata.agreeingCommits);
    }

    std::string queryStr = oracleQueryToString(metadata.interfaceIndex, query);
    if (queryStr.find("error") != std::string::npos)
    {
        std::vector<char> hexQuery(2 * query.size() + 1, 0);
        byteToHex(query.data(), hexQuery.data(), static_cast<int>(query.size()));
        LOG("Query: %s %s\n", hexQuery.data(), queryStr.c_str());
    }
    else
    {
        LOG("Query: %s\n", queryStr.c_str());
    }

    if (reply.size() > 0)
    {
        std::string replyStr = oracleReplyToString(metadata.interfaceIndex, reply);
        if (replyStr.find("error") != std::string::npos)
        {
            std::vector<char> hexReply(2 * reply.size() + 1, 0);
            byteToHex(reply.data(), hexReply.data(), static_cast<int>(reply.size()));
            LOG("Reply: %s %s\n", hexReply.data(), replyStr.c_str());
        }
        else
        {
            LOG("Reply: %s\n", replyStr.c_str());
        }
    }
}

static void receiveQueryStats(QCPtr& qc, RespondOracleDataQueryStatistics& stats)
{
    struct {
        RequestResponseHeader header;
        RequestOracleData req;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestOracleData::type());
    memset(&packet.req, 0, sizeof(packet.req));
    packet.req.reqType = RequestOracleData::requestQueryStatistics;
    qc->sendData((uint8_t*)&packet, packet.header.size());

    constexpr unsigned long long responseSize = sizeof(RequestResponseHeader) + sizeof(RespondOracleData) + sizeof(RespondOracleDataQueryStatistics);
    uint8_t buffer[responseSize];
    int recvByte = qc->receiveData(buffer, responseSize);
    if (recvByte != responseSize)
    {
        throw std::logic_error("Connection closed.");
    }
    const auto* header = (RequestResponseHeader*)buffer;
    const auto* header2 = (RespondOracleData*)(buffer + sizeof(RequestResponseHeader));
    if (header->type() != RespondOracleData::type() || header2->resType != RespondOracleData::respondQueryStatistics)
    {
        throw std::logic_error("Unexpected response message.");
    }
    const auto* receivedStats = (RespondOracleDataQueryStatistics*)(buffer + sizeof(RequestResponseHeader) + sizeof(RespondOracleData));
    stats = *receivedStats;
}

static void printQueryStats(const RespondOracleDataQueryStatistics& stats)
{
    const float revealPerSuccess = (stats.successfulCount) ? float(stats.revealTxCount) / stats.successfulCount : 0;
    LOG("successful:   % " PRIu64 " queries (takes %.3f ticks on average, %.1f reveal tx / success)\n", stats.successfulCount, float(stats.successAvgMilliTicksPerQuery) / 1000.0f, revealPerSuccess);
    LOG("timeout:      % " PRIu64 " queries in total (average timeout is %.3f ticks)\n", stats.timeoutCount, float(stats.timeoutAvgMilliTicksPerQuery) / 1000.0f);
    LOG("              % " PRIu64 " queries before OM reply\n", stats.timeoutNoReplyCount);
    LOG("              % " PRIu64 " queries before commit quorum\n", stats.timeoutNoCommitCount);
    LOG("              % " PRIu64 " queries before reveal\n", stats.timeoutNoRevealCount);
    LOG("unresolvable: % " PRIu64 " queries\n", stats.unresolvableCount);
    LOG("pending:      % " PRIu64 " queries in total\n", stats.pendingCount);
    LOG("              % " PRIu64 " queries before OM reply (takes %.3f ticks on average)\n", stats.pendingOracleMachineCount, float(stats.oracleMachineReplyAvgMilliTicksPerQuery) / 1000.0f);
    LOG("              % " PRIu64 " queries before commit quorum (takes %.3f ticks on average)\n", stats.pendingCommitCount, float(stats.commitAvgMilliTicksPerQuery) / 1000.0f);
    LOG("              % " PRIu64 " queries before reveal / success\n", stats.pendingRevealCount);
    if (stats.oracleMachineRepliesDisagreeCount > 0)
    {
        LOG("OM issues:    % " PRIu64 " queries had OM replies that differ between OMs\n", stats.oracleMachineRepliesDisagreeCount);
    }
}

static bool parseTick(const char* tickStr, long long& tickFrom, long long& tickTo)
{
    char* writabletickStr = STRDUP(tickStr);
    std::string part1 = strtok2string(writabletickStr, "-");
    std::string part2 = strtok2string(NULL, "-");
    std::string part3 = strtok2string(NULL, "-");
    free(writabletickStr);
    if (!part3.empty())
    {
        LOG("Failed to parse tick string \"%s\"! Does not follow syntax N1-N2!", tickStr);
        return false;
    }

    bool okay = true;
    try
    {
        tickFrom = std::stoll(part1);
        if (tickFrom <= 0)
            okay = false;
        if (!part2.empty())
        {
            tickTo = std::stoll(part2);
            if (tickTo <= 0)
                okay = false;
        }
        else
            tickTo = tickFrom;
    }
    catch (std::exception e)
    {
        okay = false;
    }

    if (!okay)
    {
        LOG("Failed to parse tick string \"%s\"! Tick must be a positive number N or rane N1-N2!", tickStr);
    }
    return okay;
}

void processGetOracleQueryWithTick(const char* nodeIp, const int nodePort, unsigned int reqType, const char* reqParam, bool getAllDetails)
{
    long long tickFrom = 0, tickTo = 0;
    if (!parseTick(reqParam, tickFrom, tickTo))
        return;

    const long long tickCount = tickTo - tickFrom + 1;
    if (tickCount < 1)
    {
        LOG("In range N1-N2, N2 should be greater than N2.");
        return;
    }
    if (tickCount > 1000)
    {
        LOG("Range of ticks is too large. Skipped for node performance reasons.");
        return;
    }

    auto qc = make_qc(nodeIp, nodePort);
    for (long long tick = tickFrom; tick <= tickTo; ++tick)
    {
        std::vector<int64_t> recQueryIds = receiveQueryIds(qc, reqType, tick);

        if (!getAllDetails)
        {
            LOG("Query IDs in tick %" PRIi64 ":\n", tick);
            for (const int64_t& id : recQueryIds)
            {
                LOG("- %" PRIi64 "\n", id);
            }
        }
        else
        {
            LOG("Number of query IDs in tick %" PRIi64 ": %d\n\n", tick, (int)recQueryIds.size());

            RespondOracleDataQueryMetadata metadata;
            std::vector<uint8_t> query, reply;
            for (const int64_t& id : recQueryIds)
            {
                receiveQueryInformation(qc, id, metadata, query, reply);
                if (metadata.queryId == 0)
                {
                    LOG("Error getting query metadata! Stopping now.\n");
                    return;
                }
                printQueryInformation(metadata, query, reply);
                LOG("\n");
            }
        }
    }
}

void processGetOracleQuery(const char* nodeIp, const int nodePort, const char* requestType, const char* reqParam)
{
    if (strcasecmp(requestType, "") == 0)
        printGetOracleQueryHelpAndExit();

    if (strcasecmp(requestType, "pending") == 0)
    {
        auto qc = make_qc(nodeIp, nodePort);
        std::vector<int64_t> recQueryIds = receiveQueryIds(qc, RequestOracleData::requestPendingQueryIds);

        LOG("Pending query ids:\n");
        for (const int64_t& id : recQueryIds)
        {
            LOG("- %" PRIi64 "\n", id);
        }
    }
    else if (strcasecmp(requestType, "pending+") == 0)
    {
        auto qc = make_qc(nodeIp, nodePort);
        std::vector<int64_t> recQueryIds = receiveQueryIds(qc, RequestOracleData::requestPendingQueryIds);
        LOG("Number of pending query IDs: %d\n\n", (int)recQueryIds.size());

        RespondOracleDataQueryMetadata metadata;
        std::vector<uint8_t> query, reply;
        for (const int64_t& id : recQueryIds)
        {
            receiveQueryInformation(qc, id, metadata, query, reply);
            printQueryInformation(metadata, query, reply);
            LOG("\n");
        }
    }
    else if (strcasecmp(requestType, "all") == 0)
    {
        processGetOracleQueryWithTick(nodeIp, nodePort,
            RequestOracleData::requestAllQueryIdsByTick, reqParam,
            /*getAllDetails=*/false);
    }
    else if (strcasecmp(requestType, "all+") == 0)
    {
        processGetOracleQueryWithTick(nodeIp, nodePort,
            RequestOracleData::requestAllQueryIdsByTick, reqParam,
            /*getAllDetails=*/true);
    }
    else if (strcasecmp(requestType, "user") == 0)
    {
        processGetOracleQueryWithTick(nodeIp, nodePort,
            RequestOracleData::requestUserQueryIdsByTick, reqParam,
            /*getAllDetails=*/false);
    }
    else if (strcasecmp(requestType, "user+") == 0)
    {
        processGetOracleQueryWithTick(nodeIp, nodePort,
            RequestOracleData::requestUserQueryIdsByTick, reqParam,
            /*getAllDetails=*/true);
    }
    else if (strcasecmp(requestType, "contract") == 0)
    {
        processGetOracleQueryWithTick(nodeIp, nodePort,
            RequestOracleData::requestContractDirectQueryIdsByTick, reqParam,
            /*getAllDetails=*/false);
    }
    else if (strcasecmp(requestType, "contract+") == 0)
    {
        processGetOracleQueryWithTick(nodeIp, nodePort,
            RequestOracleData::requestContractDirectQueryIdsByTick, reqParam,
            /*getAllDetails=*/true);
    }
    else if (strcasecmp(requestType, "subscription") == 0)
    {
        processGetOracleQueryWithTick(nodeIp, nodePort,
            RequestOracleData::requestContractSubscriptionQueryIdsByTick, reqParam,
            /*getAllDetails=*/false);
    }
    else if (strcasecmp(requestType, "subscription+") == 0)
    {
        processGetOracleQueryWithTick(nodeIp, nodePort,
            RequestOracleData::requestContractSubscriptionQueryIdsByTick, reqParam,
            /*getAllDetails=*/true);
    }
    else if (strcasecmp(requestType, "stats") == 0)
    {
        auto qc = make_qc(nodeIp, nodePort);
        RespondOracleDataQueryStatistics stats;
        receiveQueryStats(qc, stats);
        printQueryStats(stats);
    }
    else
    {
        // no known command, try to interpret param string as query id (8-byte int64_t number)
        int64_t queryId;
        try
        {
            queryId = std::stoll(std::string(requestType));
        }
        catch (std::exception e)
        {
            LOG(e.what());
            return;
        }

        auto qc = make_qc(nodeIp, nodePort);

        RespondOracleDataQueryMetadata metadata;
        std::vector<uint8_t> query, reply;
        receiveQueryInformation(qc, queryId, metadata, query, reply);
        printQueryInformation(metadata, query, reply);
    }
}
