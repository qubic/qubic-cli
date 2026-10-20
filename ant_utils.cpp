// Submodule headers FIRST (before structs.h/defines.h): the NetworkMessageType enum must parse
// before qubic-cli's #defines shadow its identifiers - the same ordering oracle_utils.cpp relies on.
#include "core/src/network_messages/common_def.h"
#include "core/src/network_messages/ant_colony_message.h"

#include <cstring>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <span>
#include <map>
#include <string>
#include <algorithm>
#include <functional>

#include "structs.h"
#include "connection.h"
#include "key_utils.h"
#include "k12_and_key_utils.h"
#include "logger.h"
#include "utils.h"
#include "node_utils.h"
#include "wallet_utils.h"
#include "ant_utils.h"

static constexpr uint32_t ROOT_TICK_OFFSET = 0u;
static constexpr uint32_t ROOT_INDEX_IN_TICK = 0xFFFFFFFFu;

// Mirrors core: ANT_COLONY_MINING_SOLUTION_INPUT_TYPE (mining.h) and SOLUTION_SECURITY_DEPOSIT
// (public_settings.h). MAX_ANCHOR_WALK_BACK matches AntMiner's resolveAnchorDigest bound.
static constexpr uint16_t ANT_SOLUTION_TX_TYPE = 12u;
static constexpr uint64_t ANT_SOLUTION_DEPOSIT = 1000000ull;
static constexpr uint32_t MAX_ANCHOR_WALK_BACK = 16u;

using Ref = std::pair<uint32_t, uint32_t>;

// --- epoch context (public) ---

static bool fetchAntEpochContext(QCPtr qc, RespondAntEpochContext& ctx)
{
    struct
    {
        RequestResponseHeader header;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestAntEpochContext::type());
    qc->sendData(packet);

    RequestResponseHeader respHeader;
    if (qc->receiveData(respHeader) != (int)sizeof(respHeader)
        || respHeader.type() != RespondAntEpochContext::type())
    {
        return false;
    }
    // Contains an m256i (not trivially copyable) -> receive as raw bytes.
    return qc->receiveData(std::span<uint8_t>((uint8_t*)&ctx, sizeof(ctx)), sizeof(ctx)) == (int)sizeof(ctx);
}

void printAntEpochContext(const char* nodeIp, int nodePort)
{
    QCPtr qc = make_qc(nodeIp, nodePort);
    RespondAntEpochContext ctx;
    if (!fetchAntEpochContext(qc, ctx))
    {
        LOG("Failed to receive ant epoch context.\n");
        return;
    }

    char digest[65] = {0};
    char topo[65] = {0};
    char data[65] = {0};
    byteToHex((const uint8_t*)&ctx.spectrumDigest, digest, 32);
    byteToHex((const uint8_t*)&ctx.topologyHash, topo, 32);
    byteToHex((const uint8_t*)&ctx.dataHash, data, 32);
    LOG("Ant epoch context (epoch %u):\n", ctx.epoch);
    LOG("  threshold            : %u\n", ctx.threshold);
    LOG("  freshnessWindow      : %u ticks\n", ctx.freshnessWindow);
    LOG("  solutionCount        : %u\n", ctx.solutionCount);
    LOG("  freeAnnSlotsCount    : %u\n", ctx.freeAnnSlotsCount);
    LOG("  maxChildrenPerParent : %u (0 = unbounded)\n", ctx.maxChildrenPerParent);
    LOG("  spectrumDigest       : %s\n", digest);
    LOG("  topologyHash         : %s\n", topo);
    LOG("  dataHash             : %s\n", data);
}

// --- identity tree (operator-signed) ---

// Reach first, then errors: a higher shift outranks any lower one, whatever the error count.
static bool ranksAbove(const AntIdentityTreeNode& a, const AntIdentityTreeNode& b)
{
    if (a.shift != b.shift)
    {
        return a.shift > b.shift;
    }
    return a.score < b.score;
}

static int findBest(const std::vector<AntIdentityTreeNode>& nodes)
{
    int best = -1;
    for (int i = 0; i < (int)nodes.size(); i++)
    {
        if (best < 0 || ranksAbove(nodes[i], nodes[best]))
        {
            best = i;
        }
    }
    return best;
}

static std::map<Ref, std::vector<int>> buildChildren(const std::vector<AntIdentityTreeNode>& nodes)
{
    std::map<Ref, std::vector<int>> children;
    for (int i = 0; i < (int)nodes.size(); i++)
    {
        children[Ref(nodes[i].parentTick, nodes[i].parentSolutionIndexInTick)].push_back(i);
    }
    for (auto& entry : children)
    {
        std::sort(entry.second.begin(), entry.second.end(),
            [&](int a, int b) { return ranksAbove(nodes[a], nodes[b]); });   // best first
    }
    return children;
}

// Page the whole operator-signed tree into 'out'. Returns true on success.
static bool fetchAntIdentityTree(QCPtr qc, const uint8_t* subseed, const uint8_t* publicKey,
    const m256i& pubkey, std::vector<AntIdentityTreeNode>& out)
{
    // Hard bound so a misbehaving cursor can't loop forever. A full per-epoch forest is at most
    // ANT_MAX_NODES_PER_EPOCH (2^23) nodes at ANT_IDENTITY_TREE_NODES_PER_RESPONSE per page.
    static constexpr int MAX_TREE_PAGES = (1 << 23) / (int)ANT_IDENTITY_TREE_NODES_PER_RESPONSE + 1;

    uint32_t fromIndex = 0;
    for (int page = 0; page < MAX_TREE_PAGES; page++)
    {
        struct
        {
            RequestResponseHeader header;
            RequestAntIdentityTree request;
            uint8_t signature[64];
        } packet;
        packet.header.setSize(sizeof(packet));
        packet.header.randomizeDejavu();
        packet.header.setType(RequestAntIdentityTree::type());
        packet.request.pubkey = pubkey;
        packet.request.fromIndex = fromIndex;
        packet.request.padding = 0;

        uint8_t digest[32];
        KangarooTwelve((uint8_t*)&packet.request, sizeof(RequestAntIdentityTree), digest, 32);
        // FourQ encode() writes the signature with an aligned 32-byte store; sign into a 32-byte-aligned
        // buffer, then copy into the (possibly unaligned) packet field.
        alignas(32) uint8_t sig[64];
        sign(subseed, publicKey, digest, sig);
        memcpy(packet.signature, sig, 64);

        // The packet holds an m256i (not trivially copyable), so send it as raw bytes.
        qc->sendData(std::span<const uint8_t>((const uint8_t*)&packet, sizeof(packet)), sizeof(packet));

        RequestResponseHeader respHeader;
        if (qc->receiveData(respHeader) != (int)sizeof(respHeader)
            || respHeader.type() != RespondAntIdentityTreeHeader::type())
        {
            return false;
        }

        const uint32_t payloadSize = respHeader.size() - (uint32_t)sizeof(RequestResponseHeader);
        std::vector<uint8_t> buffer(payloadSize > 0 ? payloadSize : 1);
        if (payloadSize > 0 && qc->receiveData(std::span<uint8_t>(buffer.data(), payloadSize), payloadSize) != (int)payloadSize)
        {
            return false;
        }
        if (payloadSize < sizeof(RespondAntIdentityTreeHeader))
        {
            return true;
        }

        const RespondAntIdentityTreeHeader* rh = (const RespondAntIdentityTreeHeader*)buffer.data();
        if (rh->itemSize != sizeof(AntIdentityTreeNode))
        {
            LOG("Tree node itemSize mismatch (node %u, cli %u) - struct out of sync.\n",
                rh->itemSize, (unsigned)sizeof(AntIdentityTreeNode));
            return false;
        }
        const AntIdentityTreeNode* nodes =
            (const AntIdentityTreeNode*)(buffer.data() + sizeof(RespondAntIdentityTreeHeader));
        for (uint32_t i = 0; i < rh->count; i++)
        {
            out.push_back(nodes[i]);
        }
        if (rh->nextIndex == 0 || rh->nextIndex <= fromIndex)
        {
            return true;
        }
        fromIndex = rh->nextIndex;
    }
    return true;
}

void printAntIdentityTree(const char* nodeIp, int nodePort, const char* identity, const char* seed)
{
    m256i pubkey;
    getPublicKeyFromIdentity(identity, (uint8_t*)&pubkey);

    // The read is operator-signed: 'seed' must be the node operator's seed.
    uint8_t subseed[32];
    uint8_t privateKey[32];
    uint8_t publicKey[32];
    getSubseedFromSeed((const uint8_t*)seed, subseed);
    getPrivateKeyFromSubSeed(subseed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, publicKey);

    QCPtr qc = make_qc(nodeIp, nodePort);

    RespondAntEpochContext ctx;
    const bool hasCtx = fetchAntEpochContext(qc, ctx);

    std::vector<AntIdentityTreeNode> nodes;
    if (!fetchAntIdentityTree(qc, subseed, publicKey, pubkey, nodes))
    {
        LOG("Tree read failed - is the seed the node operator's? (reads verify against operatorPublicKey)\n");
        return;
    }

    const int best = findBest(nodes);
    const std::map<Ref, std::vector<int>> children = buildChildren(nodes);
    const uint32_t cap = hasCtx ? ctx.maxChildrenPerParent : 0;

    LOG("identity %s  nodes: %u", identity, (unsigned)nodes.size());
    if (best >= 0)
    {
        LOG("  best: %u (depth %u)", nodes[best].score, nodes[best].depth);
    }
    LOG("\n");
    if (hasCtx)
    {
        LOG("epoch %u  threshold=%u  child-cap=%s\n", ctx.epoch, ctx.threshold,
            cap == 0 ? "unbound" : std::to_string(cap).c_str());
    }
    LOG("root  depth 0\n");

    // Build each line into a buffer (indent + node) and LOG it once, so indentation stays intact.
    std::function<void(Ref, int)> walk = [&](Ref ref, int indent)
    {
        auto it = children.find(ref);
        if (it == children.end())
        {
            return;
        }
        for (int idx : it->second)
        {
            const AntIdentityTreeNode& n = nodes[idx];
            char line[256];
            int off = 0;
            for (int s = 0; s < indent && off + 2 < (int)sizeof(line); s++)
            {
                line[off++] = ' ';
                line[off++] = ' ';
            }
            snprintf(line + off, sizeof(line) - off, "err=%u shift=%u d%u kids=%u ref=%u:%u anchor=%u%s%s",
                n.score, n.shift, n.depth, n.childCount, n.selfTick, n.selfSolutionIndexInTick, n.anchorTick,
                (cap && n.childCount >= cap) ? "  FULL" : "",
                (idx == best) ? "  * best" : "");
            LOG("%s\n", line);
            walk(Ref(n.selfTick, n.selfSolutionIndexInTick), indent + 1);
        }
    };
    walk(Ref(ROOT_TICK_OFFSET, ROOT_INDEX_IN_TICK), 1);
}

void sendAntSolution(const char* nodeIp, int nodePort, const char* seed,
    const uint8_t nonce[32], uint32_t claimedScore,
    uint32_t parentTick, uint32_t parentIndex, uint32_t scheduledTickOffset)
{
    QCPtr qc = make_qc(nodeIp, nodePort);

    const uint32_t currentTick = getTickNumberFromNode(qc);
    if (currentTick == 0)
    {
        LOG("Could not read current tick from node.\n");
        return;
    }

    // The node records an anchor digest only for non-empty ticks, so an empty anchor is rejected as
    // stale. Walk back from currentTick-1 to the nearest tick the node holds TickData for - the same
    // approach AntMiner uses. getTickData zeroes result (result.tick == 0) for an empty tick.
    uint32_t anchorTick = 0;
    const uint32_t fromTick = currentTick - 1;
    TickData td;
    for (uint32_t tick = fromTick; tick > 0 && (fromTick - tick) < MAX_ANCHOR_WALK_BACK; tick--)
    {
        if (getTickData(qc, tick, td) && td.tick == tick)
        {
            anchorTick = tick;
            break;
        }
    }
    if (anchorTick == 0)
    {
        LOG("No non-empty tick found in the last %u ticks to anchor to; retry once the network has activity.\n",
            MAX_ANCHOR_WALK_BACK);
        return;
    }

    // 48-byte payload matching AntColonyMiningSolutionTransaction's input:
    // parentTick | parentSolutionIndexInTick | anchorTick | claimedScore | nonce.
    uint8_t payload[48];
    memcpy(payload + 0, &parentTick, 4);
    memcpy(payload + 4, &parentIndex, 4);
    memcpy(payload + 8, &anchorTick, 4);
    memcpy(payload + 12, &claimedScore, 4);
    memcpy(payload + 16, nonce, 32);

    // dest = the zero-pubkey identity
    uint8_t zeroPub[32] = {0};
    char zeroId[128] = {0};
    getIdentityFromPublicKey(zeroPub, zeroId, false);

    char nonceHex[65] = {0};
    byteToHex(nonce, nonceHex, 32);
    LOG("Ant solution -> anchor %u (%u back), parent %s, claimedScore %u, nonce %s\n",
        anchorTick, fromTick - anchorTick,
        (parentIndex == ROOT_INDEX_IN_TICK) ? "ROOT" : "node",
        claimedScore, nonceHex);

    uint32_t scheduledTick = 0;
    makeCustomTransaction(nodeIp, nodePort, seed, zeroId,
        ANT_SOLUTION_TX_TYPE, ANT_SOLUTION_DEPOSIT, 48, payload, scheduledTickOffset, &scheduledTick);
}
