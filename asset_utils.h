#pragma once

namespace qpi
{
    struct Asset
    {
        uint8_t issuer[32];
        uint64_t assetName;
    };
}

uint64_t assetNameFromString(const char* assetName);
void assetNameToString(uint64_t assetNameInt, char* outAssetName);

void printOwnedAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity);
void printPossessionAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity);
void printAssetRecords(const char* nodeIp, const int nodePort, const char* requestType, const char* requestQueryString);
