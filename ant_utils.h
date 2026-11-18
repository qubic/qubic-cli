#pragma once

// -getantepochcontext : print the public epoch context.
void printAntEpochContext(const char* nodeIp, int nodePort);

// -getanttree <identity> : page and print one identity's tree. Operator-signed with 'seed'.
void printAntIdentityTree(const char* nodeIp, int nodePort, const char* identity, const char* seed);

// -sendantsolution : craft an ant-colony solution as an inputType-12 transaction (dest=zero,
// amount=deposit) and submit it. Walks back to a non-empty anchor tick, signs with 'seed' (the
// solution lands in the signer's tree). nonce is 32 bytes; parentIndex 0xFFFFFFFF means ROOT.
void sendAntSolution(const char* nodeIp, int nodePort, const char* seed,
    const uint8_t nonce[32], uint32_t claimedScore,
    uint32_t parentTick, uint32_t parentIndex, uint32_t scheduledTickOffset);
