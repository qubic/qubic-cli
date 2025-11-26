#pragma once

bool getSubseedFromSeed(const uint8_t* seed, uint8_t* subseed);
void getPrivateKeyFromSubSeed(const uint8_t* seed, uint8_t* privateKey);
void getPublicKeyFromPrivateKey(const uint8_t* privateKey, uint8_t* publicKey);
void getIdentityFromPublicKey(const uint8_t* pubkey, char* identity, bool isLowerCase);
void getTxHashFromDigest(const uint8_t* digest, char* txHash);
void getPublicKeyFromIdentity(const char* identity, uint8_t* publicKey);
bool checkSumIdentity(const char* identity);

// Compute the digest (root hash) from siblings of Merkle tree
template <unsigned int hashByteLen>
void getDigestFromSiblings(
    unsigned int depth,
    const uint8_t *input,
    unsigned int inputByteLen,
    unsigned int inputIndex,
    const uint8_t (*siblings)[hashByteLen],
    uint8_t *output);
