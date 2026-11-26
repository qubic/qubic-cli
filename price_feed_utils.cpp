// Submodule headers FIRST (before structs.h/defines.h): the NetworkMessageType enum must parse
// before qubic-cli's #defines shadow its identifiers.
#include "core/src/network_messages/common_def.h"
#include "core/src/network_messages/price_feed.h"

#include <cstring>
#include <cstdio>
#include <cstdint>
#include <ctime>
#include <memory>
#include <span>

#include "structs.h"
#include "connection.h"
#include "k12_and_key_utils.h"
#include "logger.h"
#include "utils.h"
#include "node_utils.h"
#include "price_feed_utils.h"

// Mirrors core src/mining/price_feed.h: step 0 is 2022-04-13 12:00:00 UTC; point flags and sources.
static constexpr int64_t BTC_FEED_HOUR_ZERO_UNIX_SEC = 1649851200ll;
static constexpr uint32_t POINT_FLAG_FILLED = 1u;
static constexpr uint32_t POINT_FLAG_BOOTSTRAP = 2u;

static void formatHourTime(uint32_t hour, uint32_t periodSec, char* out, size_t outSize)
{
    const time_t seconds = (time_t)(BTC_FEED_HOUR_ZERO_UNIX_SEC + (int64_t)hour * (int64_t)periodSec);
    struct tm utc;
    memset(&utc, 0, sizeof(utc));
#ifdef _MSC_VER
    gmtime_s(&utc, &seconds);
#else
    gmtime_r(&seconds, &utc);
#endif
    strftime(out, outSize, "%Y-%m-%d %H:%M:%S", &utc);
}

static const char* sourceName(uint32_t source)
{
    switch (source)
    {
    case 1: return "bootstrap file";
    case 2: return "epoch file";
    case 3: return "F6 file";
    case 4: return "snapshot";
    default: return "none";
    }
}

// One page of the series. Returns false when the node did not answer with a well-formed response.
static bool fetchBtcPriceSeries(QCPtr qc, uint32_t fromHour, uint32_t count, RespondPriceSeries& out)
{
    struct
    {
        RequestResponseHeader header;
        RequestPriceSeries request;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestPriceSeries::type());
    packet.request.fromHour = fromHour;
    packet.request.count = count;
    qc->sendData(packet);

    RequestResponseHeader respHeader;
    if (qc->receiveData(respHeader) != (int)sizeof(respHeader)
        || respHeader.type() != RespondPriceSeries::type())
    {
        return false;
    }
    const uint32_t payloadSize = respHeader.size() - (uint32_t)sizeof(respHeader);
    if (payloadSize < RESPOND_PRICE_SERIES_HEADER_SIZE || payloadSize > sizeof(out))
    {
        return false;
    }
    memset(&out, 0, sizeof(out));
    if (qc->receiveData(std::span<uint8_t>((uint8_t*)&out, payloadSize), payloadSize) != (int)payloadSize)
    {
        return false;
    }
    return payloadSize == RESPOND_PRICE_SERIES_HEADER_SIZE + out.count * sizeof(RespondPriceSeriesPoint);
}

void printBtcPriceSeries(const char* nodeIp, int nodePort, uint32_t fromHour, uint32_t count, uint32_t periodSec)
{
    QCPtr qc = make_qc(nodeIp, nodePort);
    auto page = std::make_unique<RespondPriceSeries>();

    // The state alone first: it tells which hours exist.
    if (!fetchBtcPriceSeries(qc, 0, 0, *page))
    {
        LOG("Failed to receive the BTC price series (the node may not support this request).\n");
        return;
    }

    char digest[65] = {0};
    char timeText[32] = {0};
    byteToHex(page->seriesDigest, digest, 32);
    LOG("BTC price series (epoch %u, node tick %u):\n", (unsigned int)page->epoch, page->tick);
    LOG("  synced          : %s\n", page->synced ? "yes" : "NO - this node cannot verify mining solutions");
    LOG("  loaded from     : %s\n", sourceName(page->source));
    formatHourTime(page->baseHour, periodSec, timeText, sizeof(timeText));
    LOG("  base hour       : %u (%s UTC, last hour known when the epoch began)\n", page->baseHour, timeText);
    formatHourTime(page->nextHour, periodSec, timeText, sizeof(timeText));
    LOG("  next hour       : %u (%s UTC, first hour not stored yet)\n", page->nextHour, timeText);
    LOG("  hours this epoch: %u\n", (page->nextHour > page->baseHour) ? (page->nextHour - page->baseHour - 1) : 0u);
    LOG("  query in flight : %s\n", page->bound ? "yes" : "no");
    LOG("  failed attempts : %u for the next hour\n", page->failedAttempts);
    LOG("  series digest   : %s\n", digest);
    LOG("  replies         : %llu stored, %llu timed out, %llu unresolvable, %llu invalid, %llu not started, %llu hours filled\n",
        (unsigned long long)page->numSuccess, (unsigned long long)page->numTimeout,
        (unsigned long long)page->numUnresolvable, (unsigned long long)page->numInvalidReply,
        (unsigned long long)page->numStartFailed, (unsigned long long)page->numFilled);

    if (count == 0)
    {
        return;
    }
    const uint32_t nextHour = page->nextHour;
    if (fromHour == BTC_PRICE_SERIES_LATEST)
    {
        fromHour = (nextHour > count) ? (nextHour - count) : 0;
    }

    LOG("hour,utc,price,flags,appendTick\n");
    uint32_t printed = 0;
    uint32_t hour = fromHour;
    while (printed < count && hour < nextHour)
    {
        uint32_t want = count - printed;
        if (want > PRICE_SERIES_MAX_POINTS_PER_RESPONSE)
        {
            want = PRICE_SERIES_MAX_POINTS_PER_RESPONSE;
        }
        if (!fetchBtcPriceSeries(qc, hour, want, *page))
        {
            LOG("Failed to receive the points from hour %u.\n", hour);
            return;
        }
        for (uint32_t i = 0; i < page->count; i++)
        {
            const RespondPriceSeriesPoint& point = page->points[i];
            const char* flags = "";
            if (point.flags & POINT_FLAG_FILLED)
            {
                flags = "filled";
            }
            else if (point.flags & POINT_FLAG_BOOTSTRAP)
            {
                flags = "bootstrap";
            }
            formatHourTime(point.hour, periodSec, timeText, sizeof(timeText));
            LOG("%u,%s,%lld.%08lld,%s,%u\n", point.hour, timeText,
                (long long)(point.priceE8 / 100000000ll), (long long)(point.priceE8 % 100000000ll),
                flags, point.appendTick);
        }
        // The node skips hours it does not hold, so advance by the request, not by the reply.
        printed += want;
        hour += want;
    }
}

void printBpp9000TaskData(const char* nodeIp, int nodePort, uint32_t anchorTick)
{
    QCPtr qc = make_qc(nodeIp, nodePort);
    if (anchorTick == 0)
    {
        const uint32_t currentTick = getTickNumberFromNode(qc);
        if (currentTick == 0)
        {
            LOG("Failed to get the current tick.\n");
            return;
        }
        anchorTick = currentTick - 1;
    }

    struct
    {
        RequestResponseHeader header;
        RequestBpp9000TaskData request;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestBpp9000TaskData::type());
    packet.request.anchorTick = anchorTick;
    qc->sendData(packet);

    auto data = std::make_unique<RespondBpp9000TaskData>();
    memset(data.get(), 0, sizeof(RespondBpp9000TaskData));
    RequestResponseHeader respHeader;
    if (qc->receiveData(respHeader) != (int)sizeof(respHeader)
        || respHeader.type() != RespondBpp9000TaskData::type())
    {
        LOG("Failed to receive the bpp9000 task data (the node may not support this request).\n");
        return;
    }
    const uint32_t payloadSize = respHeader.size() - (uint32_t)sizeof(respHeader);
    if (payloadSize < RESPOND_BPP9000_TASK_DATA_HEADER_SIZE || payloadSize > sizeof(RespondBpp9000TaskData)
        || qc->receiveData(std::span<uint8_t>((uint8_t*)data.get(), payloadSize), payloadSize) != (int)payloadSize)
    {
        LOG("Malformed bpp9000 task data response.\n");
        return;
    }
    // The targets follow the header: one frame plus the shift limit, none if the node has no data.
    static constexpr uint32_t WINDOW = 24 * 365;
    const uint32_t targetCount = payloadSize - RESPOND_BPP9000_TASK_DATA_HEADER_SIZE;
    if (targetCount != (data->valid ? WINDOW + data->shiftLimit : 0))
    {
        LOG("Malformed bpp9000 task data response.\n");
        return;
    }

    LOG("bpp9000 task data for anchor tick %u (epoch %u, node tick %u):\n", data->anchorTick,
        (unsigned int)data->epoch, data->tick);
    if (!data->valid)
    {
        LOG("  valid           : NO - the node has no price series, or the tick is outside the processed ticks of this epoch\n");
        return;
    }
    char hash[65] = {0};
    LOG("  valid           : yes\n");
    LOG("  base hour       : %u\n", data->baseHour);
    LOG("  shift limit     : %u (hours stored before the anchor tick; the frame may slide this far)\n", data->shiftLimit);
    LOG("  targets         : %u\n", targetCount);
    uint8_t allHash[32];
    KangarooTwelve(data->targets, WINDOW, allHash, 32);
    byteToHex(allHash, hash, 32);
    LOG("  first frame hash: %s\n", hash);
    KangarooTwelve(data->targets, targetCount, allHash, 32);
    byteToHex(allHash, hash, 32);
    LOG("  all targets hash: %s\n", hash);

    static constexpr uint32_t TAIL = 48;
    const uint32_t tail = (targetCount < TAIL) ? targetCount : TAIL;
    char bits[TAIL + 1] = {0};
    for (uint32_t i = 0; i < tail; i++)
    {
        bits[i] = data->targets[targetCount - tail + i] ? '1' : '0';
    }
    LOG("  last %u targets : %s (1 = price not below the hour before)\n", tail, bits);
}
