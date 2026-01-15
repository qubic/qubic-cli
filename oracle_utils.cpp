#include <cstdlib>
#include <cinttypes>

#include "core/src/network_messages/common_def.h"
#include "core/src/network_messages/oracles.h"

#include "oracle_utils.h"
#include "logger.h"
#include "utils.h"
#include "structs.h"
#include "connection.h"
#include "defines.h"
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
    exit(1);
}

static std::vector<int64_t> receiveQueryIds(QCPtr qc)
{   
    struct {
        RequestResponseHeader header;
        RequestOracleData req;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestOracleData::type());
    memset(&packet.req, 0, sizeof(packet.req));
    packet.req.reqType = RequestOracleData::requestPendingQueryIds;

    qc->sendData((uint8_t*)&packet, packet.header.size());

    std::vector<int64_t> queryIds;

    constexpr unsigned long long bufferSize = sizeof(RequestResponseHeader) + sizeof(RespondOracleData) + maxQueryIdCount * sizeof(int64_t);
    uint8_t buffer[bufferSize];
    int recvByte = qc->receiveData(buffer, sizeof(RequestResponseHeader));

    while (recvByte == sizeof(RequestResponseHeader))
    {
        auto header = (RequestResponseHeader*)buffer;
        if (header->type() == RespondOracleData::type())
        {
            recvByte = qc->receiveAllDataOrThrowException(buffer + sizeof(RequestResponseHeader), header->size() - sizeof(RequestResponseHeader));
            auto resp = (RespondOracleData*)(buffer + sizeof(RequestResponseHeader));
            if (resp->resType == RespondOracleData::respondQueryIds)
            {
                long long payloadNumBytes = header->size() - sizeof(RequestResponseHeader) - sizeof(RespondOracleData);
                if (payloadNumBytes <= 0 || payloadNumBytes % 8 != 0)
                    break;
                const uint8_t* queryIdBuffer = buffer + sizeof(RequestResponseHeader) + sizeof(RespondOracleData);
                queryIds.insert(queryIds.end(),
                    (int64_t*)queryIdBuffer, (int64_t*)(queryIdBuffer + payloadNumBytes));
            }
        }
        else if (header->type() == END_RESPOND)
        {
            break;
        }
        recvByte = qc->receiveData(buffer, sizeof(RequestResponseHeader));
    }

    return queryIds;
}

static void receiveQueryInformation(QCPtr qc, int64_t queryId, RespondOracleDataQueryMetadata& metadata, std::vector<uint8_t>& query, std::vector<uint8_t>& reply)
{
    struct {
        RequestResponseHeader header;
        RequestOracleData req;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestOracleData::type());
    memset(&packet.req, 0, sizeof(packet.req));
    packet.req.reqType = RequestOracleData::requestQueryAndResponse;
    packet.req.reqTickOrId = queryId;

    qc->sendData((uint8_t*)&packet, packet.header.size());

    uint8_t buffer[payloadBufferSize];

    memset(&metadata, 0, sizeof(RespondOracleDataQueryMetadata));
    query.clear();
    reply.clear();

    // receive metadata
    constexpr unsigned long long packetSize = sizeof(RequestResponseHeader) + sizeof(RespondOracleData) + sizeof(RespondOracleDataQueryMetadata);
    int recvByte = qc->receiveAllDataOrThrowException(buffer, packetSize);

    auto header = (RequestResponseHeader*)buffer;
    auto respOracleData = (RespondOracleData*)(buffer + sizeof(RequestResponseHeader));
    if (header->type() == RespondOracleData::type() && respOracleData->resType == RespondOracleData::respondQueryMetadata)
    {
        metadata = *(RespondOracleDataQueryMetadata*)(buffer + sizeof(RequestResponseHeader) + sizeof(RespondOracleData));
    }
    else
        return;

    // receive query data
    recvByte = qc->receiveData(buffer, sizeof(RequestResponseHeader));
    if (recvByte == sizeof(RequestResponseHeader))
    {
        header = (RequestResponseHeader*)buffer;
        if (header->type() == RespondOracleData::type())
        {
            recvByte = qc->receiveAllDataOrThrowException(buffer + sizeof(RequestResponseHeader), header->size() - sizeof(RequestResponseHeader));
            respOracleData = (RespondOracleData*)(buffer + sizeof(RequestResponseHeader));
            if (respOracleData->resType == RespondOracleData::respondQueryData)
            {
                query.insert(query.end(),
                    buffer + sizeof(RequestResponseHeader) + sizeof(RespondOracleData),
                    buffer + header->size());
            }
            else if (respOracleData->resType == RespondOracleData::respondReplyData)
            {
                // if sending the query failed for some reason, the reply might follow the metadata immediately
                goto receive_reply;
            }
            else
                return;
        }
        else
            return;
    }
    else
        return;

    // receive reply data
    recvByte = qc->receiveData(buffer, sizeof(RequestResponseHeader));
    if (recvByte == sizeof(RequestResponseHeader))
    {
        header = (RequestResponseHeader*)buffer;
        if (header->type() == RespondOracleData::type())
        {
            recvByte = qc->receiveAllDataOrThrowException(buffer + sizeof(RequestResponseHeader), header->size() - sizeof(RequestResponseHeader));
            respOracleData = (RespondOracleData*)(buffer + sizeof(RequestResponseHeader));
            if (respOracleData->resType == RespondOracleData::respondReplyData)
            {
            receive_reply:
                reply.insert(reply.end(),
                    buffer + sizeof(RequestResponseHeader) + sizeof(RespondOracleData),
                    buffer + header->size());
            }
            else
                return;
        }
        else
            return;
    }
    else
        return;
}

static void printQueryInformation(const RespondOracleDataQueryMetadata& metadata, const std::vector<uint8_t>& query, const std::vector<uint8_t>& reply)
{
    LOG("Query ID: %" PRIi64 "\n", metadata.queryId);
    LOG("Type: %" PRIu8 "\n", metadata.type);
    LOG("Status: %" PRIu8 "\n", metadata.status);
    LOG("Status Flags: %" PRIu16 "\n", metadata.statusFlags);
    LOG("Query Tick: %" PRIu32 "\n", metadata.queryTick);

    char queryingIdentity[128] = { 0 };
    getIdentityFromPublicKey(metadata.queryingEntity.m256i_u8, queryingIdentity, /*isLowerCase=*/false);
    LOG("Querying Entity: %s\n", queryingIdentity);

    LOG("Timeout: %" PRIu64 "\n", metadata.timeout);
    LOG("Interface Index: %" PRIu32 "\n", metadata.interfaceIndex);
    LOG("Subscription ID: %" PRIi32 "\n", metadata.subscriptionId);
    LOG("Reveal Tick: %" PRIu32 "\n", metadata.revealTick);
    LOG("Total Commits: %" PRIu16 "\n", metadata.totalCommits);
    LOG("Agreeing Commits: %" PRIu16 "\n", metadata.agreeingCommits);

    std::vector<char> hexQuery(2 * query.size() + 1, 0);
    byteToHex(query.data(), hexQuery.data(), static_cast<int>(query.size()));
    LOG("Query: %s\n", hexQuery.data());

    std::vector<char> hexReply(2 * reply.size() + 1, 0);
    byteToHex(reply.data(), hexReply.data(), static_cast<int>(reply.size()));
    LOG("Reply: %s\n", hexReply.data());
}

void processGetOracleQuery(const char* nodeIp, const int nodePort, const char* requestType)
{
    if (strcasecmp(requestType, "") == 0)
        printGetOracleQueryHelpAndExit();

    if (strcasecmp(requestType, "pending") == 0)
    {
        auto qc = make_qc(nodeIp, nodePort);
        std::vector<int64_t> recQueryIds = receiveQueryIds(qc);

        LOG("Pending query ids:\n");
        for (const int64_t& id : recQueryIds)
        {
            LOG("- %" PRIi64 "\n");
        }
    }
    else if (strcasecmp(requestType, "pending+") == 0)
    {
        auto qc = make_qc(nodeIp, nodePort);
        std::vector<int64_t> recQueryIds = receiveQueryIds(qc);

        RespondOracleDataQueryMetadata metadata;
        std::vector<uint8_t> query, reply;
        for (const int64_t& id : recQueryIds)
        {
            receiveQueryInformation(qc, id, metadata, query, reply);
            printQueryInformation(metadata, query, reply);
            LOG("\n");
        }
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
