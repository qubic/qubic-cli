#pragma once

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
                     const char* pIssuerInQubicFormat,
                     const char* newOwnerIdentity,
                     long long numberOfUnits,
                     uint32_t scheduledTickOffset);

void printQxFee(const char* nodeIp, const int nodePort);

void qxAddToAskOrder(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* pAssetName,
                     const char* pHexIssuer,
                     const long long price,
                     const long long numberOfShares,
                     uint32_t scheduledTickOffset);

void qxAddToBidOrder(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* pAssetName,
                     const char* pIssuerInQubicFormat,
                     const long long price,
                     const long long numberOfShares,
                     uint32_t scheduledTickOffset);

void qxRemoveToAskOrder(const char* nodeIp, int nodePort,
                        const char* seed,
                        const char* pAssetName,
                        const char* pHexIssuer,
                        const long long price,
                        const long long numberOfShares,
                        uint32_t scheduledTickOffset);

void qxRemoveToBidOrder(const char* nodeIp, int nodePort,
                        const char* seed,
                        const char* pAssetName,
                        const char* pHexIssuer,
                        const long long price,
                        const long long numberOfShares,
                        uint32_t scheduledTickOffset);

void qxGetAssetAskOrder(const char* nodeIp, int nodePort,
                        const char* pAssetName,
                        const char* pHexIssuer,
                        const long long offset);

void qxGetAssetBidOrder(const char* nodeIp, int nodePort,
                        const char* pAssetName,
                        const char* pHexIssuer,
                        const long long offset);

void qxGetEntityBidOrder(const char* nodeIp, int nodePort,
                         const char* pHexEntity,
                         const long long offset);
void qxGetEntityAskOrder(const char* nodeIp, int nodePort,
                         const char* pHexEntity,
                         const long long offset);

void qxTransferAssetManagementRights(const char* nodeIp, int nodePort,
    const char* seed,
    const char* pAssetName,
    const char* pIssuerInQubicFormat,
    uint32_t newManagingContractIndex,
    int64_t numberOfShares,
    uint32_t scheduledTickOffset);
