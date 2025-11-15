#pragma once

#include "defines.h"

COMMAND g_cmd;
char* g_seed = (char*)DEFAULT_SEED;
char* g_nodeIp = (char*)DEFAULT_NODE_IP;
char* g_targetIdentity = nullptr;
char* g_configFile = nullptr;
char* g_requestedFileName = nullptr;
char* g_requestedFileName2 = nullptr;
char* g_requestedTxId  = nullptr;
char* g_requestedIdentity  = nullptr;
char* g_qx_shareTransferPossessedIdentity = nullptr;
char* g_qx_shareTransferNewOwnerIdentity = nullptr;
int64_t g_qx_shareTransferAmount = 0;
const char* g_proposalString = "";
const char* g_voteValueString = "";
const char* g_paramString1 = "";
const char* g_paramString2 = "";
const char* g_invokeContractProcedureInputFormat = "";
const char* g_callContractFunctionInputFormat = "";
const char* g_callContractFunctionOutputFormat = "";
bool g_force = false;
bool g_enableTestContracts = false;

int64_t g_txAmount = 0;
uint16_t g_txType = 0;
uint32_t g_txTick = 0;
uint16_t g_contractFunctionNumber = 0;
int g_nodePort = DEFAULT_NODE_PORT;
int g_txExtraDataSize = 0;
int g_rawPacketSize = 0;
int g_requestedSpecialCommand = -1;
char* g_toggleMainAux0 = nullptr;
char* g_toggleMainAux1 = nullptr;
int g_setSolutionThresholdEpoch = -1;
int g_setSolutionThresholdValue = -1;
char* g_filePath = nullptr;
char* g_compressTool = nullptr;
uint32_t g_contractIndex = 0;
char g_loggingMode = 0;
char* g_compChatString = nullptr;

char* g_dumpBinaryFileInput = nullptr;
char* g_dumpBinaryFileOutput = nullptr;
uint32_t g_dumpBinaryContractId = 0;

// IPO bid
uint32_t g_IPOContractIndex = 0;
uint16_t g_makeIPOBidNumberOfShare = 0;
uint64_t g_makeIPOBidPricePerShare = 0;

// qx
uint32_t g_requestedTickNumber = 0;
uint32_t g_offsetScheduledTick = DEFAULT_SCHEDULED_TICK_OFFSET;
int g_waitUntilFinish = 0;
uint8_t g_txExtraData[1024] = {0};
uint8_t g_rawPacket[1024] = {0};

char* g_qx_issueAssetName = nullptr;
char* g_qx_issueUnitOfMeasurement = nullptr;
int64_t g_qx_issueAssetNumberOfUnit = -1;
char g_qx_issueAssetNumDecimal = 0;

char* g_qx_command1 = nullptr;
char* g_qx_command2 = nullptr;
char* g_qx_issuer = nullptr;
char* g_qx_assetName = nullptr;
long long g_qx_offset = -1;
long long g_qx_price = -1;
long long g_qx_numberOfShare = -1;

char* g_qx_assetTransferPossessedIdentity = nullptr;
char* g_qx_assetTransferNewOwnerIdentity = nullptr;
int64_t g_qx_assetTransferAmount = -1;
char* g_qx_assetTransferAssetName = nullptr;
char* g_qx_assetTransferIssuerInHex = nullptr;

// quottery
uint32_t g_quottery_betId = 0;
uint32_t g_quottery_optionId = 0;
char* g_quottery_creatorId = nullptr;
uint64_t g_quottery_numberBetSlot = 0;
uint64_t g_quottery_amountPerBetSlot = 0;
uint32_t g_quottery_pickedOption = 0;

// qutil
char* g_qutil_sendToManyV1PayoutListFile = nullptr;
int64_t g_qutil_sendToManyBenchmarkDestinationCount = 0;
int64_t g_qutil_sendToManyBenchmarkNumTransfersEach = 0;

char* g_qutil_pollNameStr = nullptr;
uint64_t g_qutil_pollType = 0; // 1 for Qubic, 2 for Asset
uint64_t g_qutil_minAmount = 0;
char* g_qutil_githubLinkStr = nullptr;
char* g_qutil_semicolonSeparatedAssets = nullptr;
uint64_t g_qutil_votePollId = 0;
uint64_t g_qutil_voteAmount = 0;
uint64_t g_qutil_voteChosenOption = 64;
uint64_t g_qutil_getResultPollId = 0;
char* g_qutil_getPollsCreatorAddress = nullptr;
uint64_t g_qutil_getPollInfoPollId = 0;
uint64_t g_qutil_cancelPollId = 0;

// qearn
uint64_t g_qearn_lockAmount = 0;
uint64_t g_qearn_unlockAmount = 0;
uint32_t g_qearn_lockedEpoch = 0;
uint32_t g_qearn_getInfoEpoch = 0;
uint32_t g_qearn_getStatsEpoch = 0;

// qvault
char* g_qvault_identity = nullptr;
uint32_t g_qvault_numberOfChangedAddress = 0;
uint32_t g_qvault_newQCAPHolderFee = 0;
uint32_t g_qvault_newReinvestingFee = 0;
uint32_t g_qvault_newDevFee = 0;

// msvault
uint64_t g_msvault_id = 0;
uint64_t g_msvault_requiredApprovals = 0;
uint8_t g_msvault_vaultName[32] = { 0 };
char* g_msvault_destination = nullptr;
char* g_msvault_publicId = nullptr;
char* g_msvault_ownersCommaSeparated = nullptr;

// qswap
char* g_qswap_issueAssetName = nullptr;
char* g_qswap_issueUnitOfMeasurement = nullptr;
int64_t g_qswap_issueAssetNumberOfUnit = -1;
char g_qswap_issueAssetNumDecimal = 0;

char* g_qswap_command1 = nullptr;
char* g_qswap_assetName = nullptr;
char* g_qswap_issuer = nullptr;

char* g_qswap_assetTransferPossessedIdentity = nullptr;
char* g_qswap_assetTransferNewOwnerIdentity = nullptr;
int64_t g_qswap_assetTransferAmount = -1;
char* g_qswap_assetTransferAssetName = nullptr;
char* g_qswap_assetTransferIssuer = nullptr;
uint32_t g_qswap_newContractIndex = 0;

int64_t g_qswap_addLiquidityQuAmount = -1;
int64_t g_qswap_addLiquidityAssetAmountDesired = -1;
int64_t g_qswap_removeLiquidityBurnLiquidity = -1;

int64_t g_qswap_liquidityQuAmountMin = -1;
int64_t g_qswap_liquidityAssetAmountMin = -1;

char* g_qswap_getLiquidityOfStakerIssuer = nullptr;

int64_t g_qswap_swapAmountOut = -1;
int64_t g_qswap_swapAmountOutMin = -1;
int64_t g_qswap_swapAmountIn = -1;
int64_t g_qswap_swapAmountInMax = -1;

int64_t g_qswap_quoteAmount = -1;

// nostromo
char* g_nost_tokenName = nullptr;
uint64_t g_nost_supply = 0;
uint32_t g_nost_tierLevel = 0;
uint32_t g_nost_startYear = 0;
uint32_t g_nost_startMonth = 0;
uint32_t g_nost_startDay = 0;
uint32_t g_nost_startHour = 0;
uint32_t g_nost_endYear = 0;
uint32_t g_nost_endMonth = 0;
uint32_t g_nost_endDay = 0;
uint32_t g_nost_endHour = 0;
uint32_t g_nost_indexOfProject = 0;
bool g_nost_decision = 0;
uint64_t g_nost_tokenPrice = 0;
uint64_t g_nost_soldAmount = 0;
uint64_t g_nost_requiredFunds = 0;

uint32_t g_nost_firstPhaseStartYear = 0;
uint32_t g_nost_firstPhaseStartMonth = 0;
uint32_t g_nost_firstPhaseStartDay = 0;
uint32_t g_nost_firstPhaseStartHour = 0;
uint32_t g_nost_firstPhaseEndYear = 0;
uint32_t g_nost_firstPhaseEndMonth = 0;
uint32_t g_nost_firstPhaseEndDay = 0;
uint32_t g_nost_firstPhaseEndHour = 0;

uint32_t g_nost_secondPhaseStartYear = 0;
uint32_t g_nost_secondPhaseStartMonth = 0;
uint32_t g_nost_secondPhaseStartDay = 0;
uint32_t g_nost_secondPhaseStartHour = 0;
uint32_t g_nost_secondPhaseEndYear = 0;
uint32_t g_nost_secondPhaseEndMonth = 0;
uint32_t g_nost_secondPhaseEndDay = 0;
uint32_t g_nost_secondPhaseEndHour = 0;

uint32_t g_nost_thirdPhaseStartYear = 0;
uint32_t g_nost_thirdPhaseStartMonth = 0;
uint32_t g_nost_thirdPhaseStartDay = 0;
uint32_t g_nost_thirdPhaseStartHour = 0;
uint32_t g_nost_thirdPhaseEndYear = 0;
uint32_t g_nost_thirdPhaseEndMonth = 0;
uint32_t g_nost_thirdPhaseEndDay = 0;
uint32_t g_nost_thirdPhaseEndHour = 0;

uint32_t g_nost_listingStartYear = 0;
uint32_t g_nost_listingStartMonth = 0;
uint32_t g_nost_listingStartDay = 0;
uint32_t g_nost_listingStartHour = 0;

uint32_t g_nost_cliffEndYear = 0;
uint32_t g_nost_cliffEndMonth = 0;
uint32_t g_nost_cliffEndDay = 0;
uint32_t g_nost_cliffEndHour = 0;
char* g_msVaultAssetName = nullptr;
char* g_msVaultIssuer = nullptr;
char* g_msVaultOwner = nullptr;
char* g_msVaultCandidateIdentity = nullptr;
uint64_t g_msVaultNewRegisteringFee = 0;
uint64_t g_msVaultNewReleaseFee = 0;
uint64_t g_msVaultNewReleaseResetFee = 0;
uint64_t g_msVaultNewHoldingFee = 0;
uint64_t g_msVaultNewDepositFee = 0;

uint32_t g_nost_vestingEndYear = 0;
uint32_t g_nost_vestingEndMonth = 0;
uint32_t g_nost_vestingEndDay = 0;
uint32_t g_nost_vestingEndHour = 0;

uint8_t g_nost_threshold = 0;
uint8_t g_nost_TGE = 0;
uint8_t g_nost_stepOfVesting = 0;

uint64_t g_nost_amount = 0;
uint32_t g_nost_indexOfFundraising = 0;
uint32_t g_nost_newManagementContractIndex = 0;
int64_t g_nost_numberOfShare = 0;

char* g_nost_identity = nullptr;

// qbond
int64_t g_qbond_millionsOfQu = 0;
int64_t g_qbond_epoch = 0;
int64_t g_qbond_asksOffset = 0;
int64_t g_qbond_bidsOffset = 0;
char* g_qbond_targetIdentity = nullptr;
int64_t g_qbond_mbondsAmount = 0;
int64_t g_qbond_mbondPrice = 0;
int64_t g_qbond_burnAmount = 0;
char* g_qbond_owner = nullptr;
bool g_qbond_updateCFAOperation = false;
