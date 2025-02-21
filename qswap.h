#pragma once

void qswapIssueAsset(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* assetName,
                     const char* unitOfMeasurement,
                     int64_t numberOfUnits,
                     char numberOfDecimalPlaces,
                     uint32_t scheduledTickOffset);

void qswapTransferAsset(const char* nodeIp, int nodePort,
                        const char* seed,
                        const char* pAssetName,
                        const char* pIssuerInQubicFormat,
                        const char* newOwnerIdentity,
                        long long numberOfUnits,
                        uint32_t scheduledTickOffset);

void printQswapFee(const char* nodeIp, const int nodePort);

void qswapCreatePool(const char* nodeIp, int nodePort,
                     const char* seed,
                     const char* pAssetName,
                     const char* pHexIssuer,
                     uint32_t scheduledTickOffset);

void qswapAddLiqudity(const char* nodeIp, int nodePort,
                      const char* seed,
                      const char* pAssetName,
                      const char* pHexIssuer,
                      int64_t quAmountDesired,
                      int64_t assetAmountDesired,
                      int64_t quAmountMin,
                      int64_t assetAmountMin,
                      uint32_t scheduledTickOffset);

void qswapRemoveLiqudity(const char* nodeIp, int nodePort,
                         const char* seed,
                         const char* pAssetName,
                         const char* pHexIssuer,
                         int64_t burnLiqudity,
                         int64_t quAmountMin,
                         int64_t assetAmountMin,
                         uint32_t scheduledTickOffset);

void qswapSwapExactQuForAsset(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* pAssetName,
                              const char* pHexIssuer,
                              int64_t quAmountIn,
                              int64_t assetAmountOutMin,
                              uint32_t scheduledTickOffset);

void qswapSwapQuForExactAsset(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* pAssetName,
                              const char* pHexIssuer,
                              int64_t quAmountInMax,
                              int64_t assetAmountOut,
                              uint32_t scheduledTickOffset);


void qswapSwapExactAssetForQu(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* pAssetName,
                              const char* pHexIssuer,
                              int64_t assetAmountIn,
                              int64_t quAmountOutMin,
                              uint32_t scheduledTickOffset);

void qswapSwapAssetForExactQu(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* pAssetName,
                              const char* pHexIssuer,
                              int64_t quAmountOut,
                              int64_t assetAmountInMax,
                              uint32_t scheduledTickOffset);

void qswapGetPoolBasicState(const char* nodeIp, int nodePort,
                            const char* pAssetName,
                            const char* pHexIssuer);

void qswapGetLiqudityOf(const char* nodeIp, int nodePort,
                        const char* pAssetName,
                        const char* pHexIssuer,
                        const char* pHexAccount);
                        
void qswapQuoteExactQuInput(const char* nodeIp, int nodePort,
                            const char* pAssetName,
                            const char* pHexIssuer,
                            int64_t quAmountIn);

void qswapQuoteExactQuOutput(const char* nodeIp, int nodePort,
                             const char* pAssetName,
                             const char* pHexIssuer,
                             int64_t quAmountOut);

void qswapQuoteExactAssetInput(const char* nodeIp, int nodePort,
                               const char* pAssetName,
                               const char* pHexIssuer,
                               int64_t assetAmountIn);

void qswapQuoteExactAssetOutput(const char* nodeIp, int nodePort,
                                const char* pAssetName,
                                const char* pHexIssuer,
                                int64_t assetAmountOut);