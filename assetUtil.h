#pragma once
void printOwnedAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity);
void printPossessionAsset(const char * nodeIp, const int nodePort, const char* requestedIdentity);
void transferQxShare(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* possessorIdentity,
                     const char* newOwnerIdentity,
                     long long numberOfUnits,
                     uint32_t scheduledTickOffset);

void qxIssueAsset(const char* nodeIp, int nodePort,
                  const char* seed,
                  const char* assetName,
                  const char* unitOfMeasurement,
                  int64_t numberOfUnits,
                  char numberOfDecimalPlaces,
                  uint32_t scheduledTickOffset);

void qxTransferAsset(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* pAssetName,
                     const char* pIssuer,
                     const char* newOwnerIdentity,
                     long long numberOfUnits,
                     uint32_t scheduledTickOffset);
