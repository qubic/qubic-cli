#include <array>

#include "stdio.h"
#include "structs.h"
#include "global.h"
#include "argparser.h"
#include "wallet_utils.h"
#include "node_utils.h"
#include "asset_utils.h"
#include "key_utils.h"
#include "sanity_check.h"
#include "sc_utils.h"
#include "quottery.h"
#include "qutil.h"
#include "qx.h"
#include "proposal.h"
#include "qearn.h"
#include "qvault.h"
#include "msvault.h"
#include "qswap.h"
#include "test_utils.h"
#include "nostromo.h"
#include "qbond.h"

int run(int argc, char* argv[])
{
#ifdef __aarch64__
    LOG("WARNING: qubic-cli (aarch64) is EXPERIMENTAL version, please use it with caution\n");
#endif
    parseArgument(argc, argv);
    switch (g_cmd)
    {
        case SHOW_KEYS:
            sanityCheckSeed(g_seed);
            printWalletInfo(g_seed);
            break;
        case GET_CURRENT_TICK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printTickInfoFromNode(g_nodeIp, g_nodePort);
            break;
        case GET_SYSTEM_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printSystemInfoFromNode(g_nodeIp, g_nodePort);
            break;
        case GET_BALANCE:
            sanityCheckIdentity(g_requestedIdentity);
            sanityCheckNode(g_nodeIp, g_nodePort);
            printBalance(g_requestedIdentity, g_nodeIp, g_nodePort);
            break;
        case GET_ASSET:
            sanityCheckIdentity(g_requestedIdentity);
            sanityCheckNode(g_nodeIp, g_nodePort);
            printOwnedAsset(g_nodeIp, g_nodePort, g_requestedIdentity);
            printPossessionAsset(g_nodeIp, g_nodePort, g_requestedIdentity);
            break;
        case QUERY_ASSETS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printAssetRecords(g_nodeIp, g_nodePort, g_paramString1, g_paramString2);
            break;
        case GET_TOTAL_NUMBER_OF_ASSET_SHARES:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilGetTotalNumberOfAssetShares(g_nodeIp, g_nodePort, g_paramString1, g_paramString2);
            break;
        case QUTIL_DISTRIBUTE_QU_TO_SHAREHOLDERS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckTxAmount(g_txAmount);
            qutilDistributeQuToShareholders(g_nodeIp, g_nodePort, g_seed, g_paramString1, g_paramString2, g_txAmount, g_offsetScheduledTick);
            break;
        case SEND_COIN:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckIdentity(g_targetIdentity);
            sanityCheckTxAmount(g_txAmount);
            makeStandardTransaction(g_nodeIp, g_nodePort, g_seed, g_targetIdentity, g_txAmount, g_offsetScheduledTick, g_waitUntilFinish);
            break;
        case SEND_COIN_IN_TICK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckIdentity(g_targetIdentity);
            sanityCheckTxAmount(g_txAmount);
            makeStandardTransactionInTick(g_nodeIp, g_nodePort, g_seed, g_targetIdentity, g_txAmount, g_txTick, g_waitUntilFinish);
            break;
        case SEND_CUSTOM_TX:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckIdentity(g_targetIdentity);
            sanityCheckTxAmount(g_txAmount);
            sanityCheckExtraDataSize(g_txExtraDataSize);
            makeCustomTransaction(g_nodeIp, g_nodePort, g_seed, g_targetIdentity,
                                  g_txType, g_txAmount, g_txExtraDataSize,
                                  g_txExtraData, g_offsetScheduledTick);

            break;
        case INVOKE_CONTRACT_PROCEDURE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckTxAmount(g_txAmount);
            invokeContractProcedure(g_nodeIp, g_nodePort, g_seed, g_contractIndex, g_txType, g_txAmount,
                                    g_invokeContractProcedureInputFormat, g_offsetScheduledTick);
            break;
        case CALL_CONTRACT_FUNCTION:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            runContractFunction(g_nodeIp, g_nodePort, g_contractIndex, g_contractFunctionNumber, g_callContractFunctionInputFormat, g_callContractFunctionOutputFormat);
            break;
        case GET_TX_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckTxHash(g_requestedTxId);
            getTxInfo(g_nodeIp, g_nodePort, g_requestedTxId);
            break;
        case CHECK_TX_ON_TICK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckTxHash(g_requestedTxId);
            checkTxOnTick(g_nodeIp, g_nodePort, g_requestedTxId, g_requestedTickNumber);
            break;
        case SEND_RAW_PACKET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckRawPacketSize(g_rawPacketSize);
            sendRawPacket(g_nodeIp, g_nodePort, g_rawPacketSize, g_rawPacket);
            break;
        case GET_TICK_DATA:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getTickDataToFile(g_nodeIp, g_nodePort, g_requestedTickNumber, g_requestedFileName);
            break;
        case GET_QUORUM_TICK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getQuorumTick(g_nodeIp, g_nodePort, g_requestedTickNumber, g_requestedFileName);
            break;
        case READ_TICK_DATA:
            sanityFileExist(g_requestedFileName);
            sanityFileExist(g_requestedFileName2);
            printTickDataFromFile(g_requestedFileName, g_requestedFileName2);
            break;
        case CHECK_TX_ON_FILE:
            sanityFileExist(g_requestedFileName);
            sanityCheckTxHash(g_requestedTxId);
            checkTxOnFile(g_requestedTxId, g_requestedFileName);
            break;
        case PUBLISH_PROPOSAL:
            printf("On development. Come back later\n");
            break;
        case VOTE_PROPOSAL:
            printf("On development. Come back later\n");
            break;
        case QX_ISSUE_ASSET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckNumberOfUnit(g_qx_issueAssetNumberOfUnit);
            sanityCheckValidAssetName(g_qx_issueAssetName);
            sanityCheckValidString(g_qx_issueUnitOfMeasurement);
            sanityCheckNumberOfDecimal(g_qx_issueAssetNumDecimal);
            qxIssueAsset(g_nodeIp, g_nodePort, g_seed,
                         g_qx_issueAssetName,
                         g_qx_issueUnitOfMeasurement,
                         g_qx_issueAssetNumberOfUnit,
                         g_qx_issueAssetNumDecimal,
                         g_offsetScheduledTick);
            break;
        case QX_TRANSFER_ASSET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckNumberOfUnit(g_qx_assetTransferAmount);
            sanityCheckValidAssetName(g_qx_assetTransferAssetName);
            sanityCheckValidString(g_qx_assetTransferIssuerInHex);
            sanityCheckIdentity(g_qx_assetTransferNewOwnerIdentity);
            qxTransferAsset(g_nodeIp, g_nodePort, g_seed,
                            g_qx_assetTransferAssetName,
                            g_qx_assetTransferIssuerInHex,
                            g_qx_assetTransferNewOwnerIdentity,
                            g_qx_assetTransferAmount,
                            g_offsetScheduledTick);
            break;
        case QX_ORDER:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qx_assetName);
            sanityCheckValidString(g_qx_command1);
            sanityCheckValidString(g_qx_command2);
            if (strcmp(g_qx_command1, "add") == 0)
            {
                if (strcmp(g_qx_command2, "bid") == 0)
                {
                    qxAddToBidOrder(g_nodeIp, g_nodePort, g_seed, g_qx_assetName, g_qx_issuer, g_qx_price, g_qx_numberOfShare, g_offsetScheduledTick);
                }
                else if (strcmp(g_qx_command2, "ask") == 0)
                {
                    qxAddToAskOrder(g_nodeIp, g_nodePort, g_seed, g_qx_assetName, g_qx_issuer, g_qx_price, g_qx_numberOfShare, g_offsetScheduledTick);
                }
            }
            else if (strcmp(g_qx_command1, "remove") == 0)
            {
                if (strcmp(g_qx_command2, "bid") == 0)
                {
                    qxRemoveToBidOrder(g_nodeIp, g_nodePort, g_seed, g_qx_assetName, g_qx_issuer, g_qx_price, g_qx_numberOfShare, g_offsetScheduledTick);
                }
                else if (strcmp(g_qx_command2, "ask") == 0)
                {
                    qxRemoveToAskOrder(g_nodeIp, g_nodePort, g_seed, g_qx_assetName, g_qx_issuer, g_qx_price, g_qx_numberOfShare, g_offsetScheduledTick);
                }
            }
            break;
        case QX_GET_ORDER:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckValidString(g_qx_command1);
            sanityCheckValidString(g_qx_command2);
            if (strcmp(g_qx_command1, "entity") == 0)
            {
                if (strcmp(g_qx_command2, "bid") == 0)
                {
                    qxGetEntityBidOrder(g_nodeIp, g_nodePort, g_qx_issuer, g_qx_offset);
                }
                else if (strcmp(g_qx_command2, "ask") == 0)
                {
                    qxGetEntityAskOrder(g_nodeIp, g_nodePort, g_qx_issuer, g_qx_offset);
                }
            }
            else if (strcmp(g_qx_command1, "asset") == 0)
            {
                sanityCheckValidAssetName(g_qx_assetName);
                if (strcmp(g_qx_command2, "bid") == 0)
                {
                    qxGetAssetBidOrder(g_nodeIp, g_nodePort, g_qx_assetName, g_qx_issuer, g_qx_offset);
                }
                else if (strcmp(g_qx_command2, "ask") == 0)
                {
                    qxGetAssetAskOrder(g_nodeIp, g_nodePort, g_qx_assetName, g_qx_issuer, g_qx_offset);
                }
            }
            break;
        case QX_TRANSFER_MANAGEMENT_RIGHTS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qx_assetName);
            sanityCheckIdentity(g_qx_issuer);
            sanityCheckNumberOfUnit(g_qx_numberOfShare);
            qxTransferAssetManagementRights(g_nodeIp, g_nodePort, g_seed, g_qx_assetName, g_qx_issuer, g_contractIndex, g_qx_numberOfShare, g_offsetScheduledTick);
            break;
        case GET_COMP_LIST:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getComputorListToFile(g_nodeIp, g_nodePort, g_requestedFileName);
            break;
        case GET_NODE_IP_LIST:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getNodeIpList(g_nodeIp, g_nodePort);
            break;
        case UPLOAD_FILE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            uploadFile(g_nodeIp, g_nodePort, g_filePath, g_seed, g_offsetScheduledTick, g_compressTool);
            break;
        case DOWNLOAD_FILE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            downloadFile(g_nodeIp, g_nodePort, g_requestedTxId, g_filePath, g_compressTool);
            break;
        case DUMP_SPECTRUM_FILE:
            sanityFileExist(g_dumpBinaryFileInput);
            sanityCheckValidString(g_dumpBinaryFileOutput);
            dumpSpectrumToCSV(g_dumpBinaryFileInput, g_dumpBinaryFileOutput);
            break;
        case DUMP_UNIVERSE_FILE:
            sanityFileExist(g_dumpBinaryFileInput);
            sanityCheckValidString(g_dumpBinaryFileOutput);
            dumpUniverseToCSV(g_dumpBinaryFileInput, g_dumpBinaryFileOutput);
            break;
        case DUMP_CONTRACT_FILE:
            sanityFileExist(g_dumpBinaryFileInput);
            sanityCheckValidString(g_dumpBinaryFileOutput);
            dumpContractToCSV(g_dumpBinaryFileInput, g_dumpBinaryContractId, g_dumpBinaryFileOutput);
            break;
        case PRINT_QX_FEE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printQxFee(g_nodeIp, g_nodePort);
            break;
        case MAKE_IPO_BID:
            sanityCheckNode(g_nodeIp, g_nodePort);
            makeIPOBid(g_nodeIp, g_nodePort, g_seed, g_IPOContractIndex, g_makeIPOBidPricePerShare, g_makeIPOBidNumberOfShare, g_offsetScheduledTick);
            break;
        case GET_IPO_STATUS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printIPOStatus(g_nodeIp, g_nodePort, g_IPOContractIndex);
            break;
        case GET_ACTIVE_IPOS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printActiveIPOs(g_nodeIp, g_nodePort);
            break;
        case QUOTTERY_ISSUE_BET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            quotteryIssueBet(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick);
            break;
        case QUOTTERY_GET_BET_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            quotteryPrintBetInfo(g_nodeIp, g_nodePort, g_quottery_betId);
            break;
        case QUOTTERY_JOIN_BET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            quotteryJoinBet(g_nodeIp, g_nodePort, g_seed, g_quottery_betId, int(g_quottery_numberBetSlot), g_quottery_amountPerBetSlot, g_quottery_pickedOption, g_offsetScheduledTick);
            break;
        case QUOTTERY_GET_BET_DETAIL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            quotteryPrintBetOptionDetail(g_nodeIp, g_nodePort, g_quottery_betId, g_quottery_optionId);
            break;
        case QUOTTERY_GET_ACTIVE_BET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            quotteryPrintActiveBet(g_nodeIp, g_nodePort);
            break;
        case QUOTTERY_GET_ACTIVE_BET_BY_CREATOR:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckIdentity(g_quottery_creatorId);
            quotteryPrintActiveBetByCreator(g_nodeIp, g_nodePort, g_quottery_creatorId);
            break;
        case QUOTTERY_GET_BASIC_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            quotteryPrintBasicInfo(g_nodeIp, g_nodePort);
            break;
        case QUOTTERY_PUBLISH_RESULT:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            quotteryPublishResult(g_nodeIp, g_nodePort, g_seed, g_quottery_betId, g_quottery_optionId, g_offsetScheduledTick);
            break;
        case QUOTTERY_CANCEL_BET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            quotteryCancelBet(g_nodeIp, g_nodePort, g_seed, g_quottery_betId, g_offsetScheduledTick);
            break;
        case TOOGLE_MAIN_AUX:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckMainAuxStatus(g_toggleMainAux0);
            sanityCheckMainAuxStatus(g_toggleMainAux1);
            toggleMainAux(g_nodeIp, g_nodePort, g_seed, g_toggleMainAux0, g_toggleMainAux1);
            break;
        case SET_SOLUTION_THRESHOLD:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            checkValidEpoch(g_setSolutionThresholdEpoch);
            checkValidSolutionThreshold(g_setSolutionThresholdValue);
            setSolutionThreshold(g_nodeIp, g_nodePort, g_seed, g_setSolutionThresholdEpoch, g_setSolutionThresholdValue);
            break;
        case SEND_SPECIAL_COMMAND:
        case REFRESH_PEER_LIST:
        case FORCE_NEXT_TICK:
        case REISSUE_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckSpecialCommand(g_requestedSpecialCommand);
            sendSpecialCommand(g_nodeIp, g_nodePort, g_seed, g_requestedSpecialCommand);
            break;
        case GET_MINING_SCORE_RANKING:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            getMiningScoreRanking(g_nodeIp, g_nodePort, g_seed);
            break;
        case GET_VOTE_COUNTER_TX:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getVoteCounterTransaction(g_nodeIp, g_nodePort, g_requestedTickNumber, g_requestedFileName);
            break;
        case SYNC_TIME:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            syncTime(g_nodeIp, g_nodePort, g_seed);
            break;
        case SAVE_SNAPSHOT:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            saveSnapshot(g_nodeIp, g_nodePort, g_seed);
            break;
        case SET_LOGGING_MODE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckLoggingMode(g_loggingMode);
            setLoggingMode(g_nodeIp, g_nodePort, g_seed, g_loggingMode);
            break;
        case COMP_CHAT:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            broadcastCompChat(g_nodeIp, g_nodePort, g_seed, g_compChatString);
            break;
        case QUTIL_SEND_TO_MANY_V1:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityFileExist(g_qutil_sendToManyV1PayoutListFile);
            qutilSendToManyV1(g_nodeIp, g_nodePort, g_seed, g_qutil_sendToManyV1PayoutListFile, g_offsetScheduledTick);
            break;
        case QUTIL_BURN_QUBIC:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qutilBurnQubic(g_nodeIp, g_nodePort, g_seed, g_txAmount, g_offsetScheduledTick);
            break;
        case QUTIL_BURN_QUBIC_FOR_CONTRACT:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qutilBurnQubicForContract(g_nodeIp, g_nodePort, g_seed, g_txAmount, g_contractIndex, g_offsetScheduledTick);
            break;
        case QUTIL_QUERY_FEE_RESERVE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilQueryFeeReserve(g_nodeIp, g_nodePort, g_contractIndex);
            break;
        case QUTIL_SEND_TO_MANY_BENCHMARK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qutilSendToManyBenchmark(g_nodeIp, g_nodePort, g_seed, uint32_t(g_qutil_sendToManyBenchmarkDestinationCount), uint32_t(g_qutil_sendToManyBenchmarkNumTransfersEach), g_offsetScheduledTick);
            break;
        case QUTIL_CREATE_POLL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidString(g_qutil_pollNameStr);
            sanityCheckTxAmount(g_qutil_minAmount);
            sanityCheckValidString(g_qutil_githubLinkStr);
            if (g_qutil_pollType == 2)
            {
                sanityCheckValidString(g_qutil_semicolonSeparatedAssets);
            }
            qutilCreatePoll(g_nodeIp, g_nodePort, g_seed, g_qutil_pollNameStr, g_qutil_pollType,
                g_qutil_minAmount, g_qutil_githubLinkStr, g_qutil_semicolonSeparatedAssets,
                g_offsetScheduledTick);
            break;
        case QUTIL_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckTxAmount(g_qutil_voteAmount);
            qutilVote(g_nodeIp, g_nodePort, g_seed, g_qutil_votePollId, g_qutil_voteAmount,
                g_qutil_voteChosenOption, g_offsetScheduledTick);
            break;
        case QUTIL_GET_CURRENT_RESULT:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilGetCurrentResult(g_nodeIp, g_nodePort, g_qutil_getResultPollId);
            break;
        case QUTIL_GET_POLLS_BY_CREATOR:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckIdentity(g_qutil_getPollsCreatorAddress);
            qutilGetPollsByCreator(g_nodeIp, g_nodePort, g_qutil_getPollsCreatorAddress);
            break;
        case QUTIL_GET_CURRENT_POLL_ID:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilGetCurrentPollId(g_nodeIp, g_nodePort);
            break;
        case QUTIL_GET_POLL_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilGetPollInfo(g_nodeIp, g_nodePort, g_qutil_getPollInfoPollId);
            break;
        case QUTIL_CANCEL_POLL: {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qutilCancelPoll(g_nodeIp, g_nodePort, g_seed, g_qutil_cancelPollId, g_offsetScheduledTick);
            break;
        }
        case QUTIL_PRINT_FEE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilPrintFees(g_nodeIp, g_nodePort);
            break;
        case GQMPROP_SET_PROPOSAL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            gqmpropSetProposal(g_nodeIp, g_nodePort, g_seed, g_proposalString, g_offsetScheduledTick, g_force);
            break;
        case GQMPROP_CLEAR_PROPOSAL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            gqmpropClearProposal(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick);
            break;
        case GQMPROP_GET_PROPOSALS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            gqmpropGetProposals(g_nodeIp, g_nodePort, g_proposalString);
            break;
        case GQMPROP_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            gqmpropVote(g_nodeIp, g_nodePort, g_seed, g_proposalString, g_voteValueString, g_offsetScheduledTick, g_force);
            break;
        case GQMPROP_GET_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            if (g_requestedIdentity)
                sanityCheckIdentity(g_requestedIdentity);
            else
                sanityCheckSeed(g_seed);
            gqmpropGetVote(g_nodeIp, g_nodePort, g_proposalString, g_requestedIdentity, g_seed);
            break;
        case GQMPROP_GET_VOTING_RESULTS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            gqmpropGetVotingResults(g_nodeIp, g_nodePort, g_proposalString);
            break;
        case GQMPROP_GET_REV_DONATION:
            sanityCheckNode(g_nodeIp, g_nodePort);
            gqmpropGetRevenueDonationTable(g_nodeIp, g_nodePort);
            break;
        case CCF_SET_PROPOSAL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            ccfSetProposal(g_nodeIp, g_nodePort, g_seed, g_proposalString, g_offsetScheduledTick, g_force);
            break;
        case CCF_CLEAR_PROPOSAL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            ccfClearProposal(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick);
            break;
        case CCF_GET_PROPOSALS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            ccfGetProposals(g_nodeIp, g_nodePort, g_proposalString);
            break;
        case CCF_GET_SUBSCRIPTION:
            sanityCheckNode(g_nodeIp, g_nodePort);
            ccfGetSubscription(g_nodeIp, g_nodePort, g_ccf_subscriptionDestination);
            break;
        case CCF_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            ccfVote(g_nodeIp, g_nodePort, g_seed, g_proposalString, g_voteValueString, g_offsetScheduledTick, g_force);
            break;
        case CCF_GET_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            if (g_requestedIdentity)
                sanityCheckIdentity(g_requestedIdentity);
            else
                sanityCheckSeed(g_seed);
            ccfGetVote(g_nodeIp, g_nodePort, g_proposalString, g_requestedIdentity, g_seed);
            break;
        case CCF_GET_VOTING_RESULTS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            ccfGetVotingResults(g_nodeIp, g_nodePort, g_proposalString);
            break;
        case CCF_GET_LATEST_TRANSFERS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            ccfGetLatestTransfers(g_nodeIp, g_nodePort);
            break;
        case CCF_GET_REGULAR_PAYMENTS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            ccfGetRegularPayments(g_nodeIp, g_nodePort);
            break;
        case QEARN_LOCK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qearnLock(g_nodeIp, g_nodePort, g_seed, g_qearn_lockAmount, g_offsetScheduledTick);
            break;
        case QEARN_UNLOCK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qearnUnlock(g_nodeIp, g_nodePort, g_seed, g_qearn_unlockAmount, g_qearn_lockedEpoch, g_offsetScheduledTick);
            break;
        case QEARN_GET_INFO_PER_EPOCH:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetInfoPerEpoch(g_nodeIp, g_nodePort, g_qearn_getInfoEpoch);
            break;
        case QEARN_GET_USER_LOCKED_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetUserLockedInfo(g_nodeIp, g_nodePort, g_requestedIdentity, g_qearn_getInfoEpoch);
            break;
        case QEARN_GET_STATE_OF_ROUND:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetStateOfRound(g_nodeIp, g_nodePort, g_qearn_getInfoEpoch);
            break;
        case QEARN_GET_USER_LOCK_STATUS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetUserLockedStatus(g_nodeIp, g_nodePort, g_requestedIdentity);
            break;
        case QEARN_GET_UNLOCKING_STATUS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetEndedStatus(g_nodeIp, g_nodePort, g_requestedIdentity);
            break;
        case QEARN_GET_STATS_PER_EPOCH:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetStatsPerEpoch(g_nodeIp, g_nodePort, g_qearn_getStatsEpoch);
            break;
        case QEARN_GET_BURNED_AND_BOOSTED_STATS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetBurnedAndBoostedStats(g_nodeIp, g_nodePort);
            break;
        case QEARN_GET_BURNED_AND_BOOSTED_STATS_PER_EPOCH:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetBurnedAndBoostedStatsPerEpoch(g_nodeIp, g_nodePort, g_qearn_getStatsEpoch);
            break;
        case QVAULT_SUBMIT_AUTH_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitAuthAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_identity);
            break;
        case QVAULT_CHANGE_AUTH_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            changeAuthAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_numberOfChangedAddress);
            break;
        case QVAULT_SUBMIT_FEES:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitFees(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_newQCAPHolderFee, g_qvault_newReinvestingFee, g_qvault_newDevFee);
            break;
        case QVAULT_CHANGE_FEES:
            sanityCheckNode(g_nodeIp, g_nodePort);
            changeFees(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_newQCAPHolderFee, g_qvault_newReinvestingFee, g_qvault_newDevFee);
            break;
        case QVAULT_SUBMIT_REINVESTING_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitReinvestingAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_identity);
            break;
        case QVAULT_CHANGE_REINVESTING_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            changeReinvestingAddress(g_nodeIp, g_nodePort, g_seed,  g_offsetScheduledTick, g_qvault_identity);
            break;
        case QVAULT_SUBMIT_ADMIN_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitAdminAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_identity);
            break;
        case QVAULT_CHANGE_ADMIN_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            changeAdminAddress(g_nodeIp, g_nodePort, g_seed,  g_offsetScheduledTick, g_qvault_identity);
            break;
        case QVAULT_GET_DATA:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getData(g_nodeIp, g_nodePort);
            break;
        case QVAULT_SUBMIT_BANNED_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitBannedAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_identity);
            break;
        case QVAULT_SAVE_BANNED_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            saveBannedAddress(g_nodeIp, g_nodePort, g_seed,  g_offsetScheduledTick, g_qvault_identity);
            break;
        case QVAULT_SUBMIT_UNBANNED_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitUnbannedannedAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_identity);
            break;
        case QVAULT_SAVE_UNBANNED_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            saveUnbannedAddress(g_nodeIp, g_nodePort, g_seed,  g_offsetScheduledTick, g_qvault_identity);
            break;
        // MSVAULT
        case MSVAULT_REGISTER_VAULT_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            msvaultRegisterVault(g_nodeIp, g_nodePort, g_seed,
                g_msvault_requiredApprovals, g_msvault_vaultName,
                g_msvault_ownersCommaSeparated,
                g_offsetScheduledTick);
            break;
        }
        case MSVAULT_DEPOSIT_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            msvaultDeposit(g_nodeIp,g_nodePort,g_seed,
                           g_msvault_id, g_txAmount, g_offsetScheduledTick);
            break;
        }
        case MSVAULT_RELEASE_TO_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckIdentity(g_msvault_destination);
            msvaultReleaseTo(g_nodeIp,g_nodePort,g_seed,
                             g_msvault_id, g_txAmount, g_msvault_destination,
                             g_offsetScheduledTick);
            break;
        }
        case MSVAULT_RESET_RELEASE_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            msvaultResetRelease(g_nodeIp,g_nodePort,g_seed,
                                g_msvault_id, g_offsetScheduledTick);
            break;
        }
        case MSVAULT_GET_VAULTS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckIdentity(g_msvault_publicId);
            msvaultGetVaults(g_nodeIp,g_nodePort,g_msvault_publicId);
            break;
        }
        case MSVAULT_GET_RELEASE_STATUS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetReleaseStatus(g_nodeIp,g_nodePort,g_msvault_id);
            break;
        }
        case MSVAULT_GET_BALANCE_OF_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetBalanceOf(g_nodeIp,g_nodePort,g_msvault_id);
            break;
        }
        case MSVAULT_GET_VAULT_NAME_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetVaultName(g_nodeIp,g_nodePort,g_msvault_id);
            break;
        }
        case MSVAULT_GET_REVENUE_INFO_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetRevenueInfo(g_nodeIp,g_nodePort);
            break;
        }
        case MSVAULT_GET_FEES_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetFees(g_nodeIp, g_nodePort);
            break;
        }
        case MSVAULT_GET_OWNERS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetVaultOwners(g_nodeIp, g_nodePort, g_msvault_id);
            break;
        }
        case MSVAULT_GET_MANAGED_ASSET_BALANCE_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckValidAssetName(g_msVaultAssetName);
            sanityCheckIdentity(g_msVaultIssuer);
            sanityCheckIdentity(g_msVaultOwner);
            msvaultGetManagedAssetBalance(g_nodeIp, g_nodePort, g_msVaultAssetName, g_msVaultIssuer, g_msVaultOwner);
            break;
        }
        case MSVAULT_REVOKE_ASSET_RIGHTS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_msVaultAssetName);
            sanityCheckIdentity(g_msVaultIssuer);
            sanityCheckTxAmount(g_txAmount);
            msvaultRevokeAssetManagementRights(g_nodeIp, g_nodePort, g_seed, g_msVaultAssetName, g_msVaultIssuer, g_txAmount, g_offsetScheduledTick);
            break;
        }
        case MSVAULT_IS_SHAREHOLDER_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckIdentity(g_msVaultCandidateIdentity);
            msvaultIsShareHolder(g_nodeIp, g_nodePort, g_msVaultCandidateIdentity);
            break;
        }
        case MSVAULT_VOTE_FEE_CHANGE_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            msvaultVoteFeeChange(g_nodeIp, g_nodePort, g_seed,
                g_msVaultNewRegisteringFee, g_msVaultNewReleaseFee,
                g_msVaultNewReleaseResetFee, g_msVaultNewHoldingFee,
                g_msVaultNewDepositFee, 0, // burn fee is 0 for now
                g_offsetScheduledTick);
            break;
        }
        case MSVAULT_GET_FEE_VOTES_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetFeeVotes(g_nodeIp, g_nodePort);
            break;
        }
        case MSVAULT_GET_FEE_VOTES_OWNER_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetFeeVotesOwner(g_nodeIp, g_nodePort);
            break;
        }
        case MSVAULT_GET_FEE_VOTES_SCORE_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetFeeVotesScore(g_nodeIp, g_nodePort);
            break;
        }
        case MSVAULT_GET_UNIQUE_FEE_VOTES_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetUniqueFeeVotes(g_nodeIp, g_nodePort);
            break;
        }
        case MSVAULT_GET_UNIQUE_FEE_VOTES_RANKING_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetUniqueFeeVotesRanking(g_nodeIp, g_nodePort);
            break;
        }
        case PRINT_QSWAP_FEE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printQswapFee(g_nodeIp, g_nodePort);
            break;
        case QSWAP_ISSUE_ASSET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckNumberOfUnit(g_qswap_issueAssetNumberOfUnit);
            sanityCheckValidAssetName(g_qswap_issueAssetName);
            sanityCheckValidString(g_qswap_issueUnitOfMeasurement);
            sanityCheckNumberOfDecimal(g_qswap_issueAssetNumDecimal);
            qswapIssueAsset(g_nodeIp, g_nodePort, g_seed,
                         g_qswap_issueAssetName,
                         g_qswap_issueUnitOfMeasurement,
                         g_qswap_issueAssetNumberOfUnit,
                         g_qswap_issueAssetNumDecimal,
                         g_offsetScheduledTick);
            break;
        case QSWAP_TRANSFER_ASSET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckNumberOfUnit(g_qswap_assetTransferAmount);
            sanityCheckValidAssetName(g_qswap_assetTransferAssetName);
            sanityCheckValidString(g_qswap_assetTransferIssuer);
            sanityCheckIdentity(g_qswap_assetTransferNewOwnerIdentity);
            qswapTransferAsset(g_nodeIp, g_nodePort, g_seed,
                            g_qswap_assetTransferAssetName,
                            g_qswap_assetTransferIssuer,
                            g_qswap_assetTransferNewOwnerIdentity,
                            g_qswap_assetTransferAmount,
                            g_offsetScheduledTick);
            break;
        case QSWAP_TRANSFER_ASSET_RIGHTS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckNumberOfUnit(g_qswap_assetTransferAmount);
            sanityCheckValidAssetName(g_qswap_assetTransferAssetName);
            sanityCheckIdentity(g_qswap_assetTransferIssuer);
            qswapTransferAssetRights(g_nodeIp, g_nodePort, g_seed,
                                        g_qswap_assetTransferAssetName,
                                        g_qswap_assetTransferIssuer,
                                        g_qswap_newContractIndex,
                                        g_qswap_assetTransferAmount,
                                        g_offsetScheduledTick);
            break;
        case QSWAP_CREATE_POOL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            qswapCreatePool(g_nodeIp, g_nodePort, g_seed,
                            g_qswap_assetName,
                            g_qswap_issuer,
                            g_offsetScheduledTick);
            break;
        case QSWAP_ADD_LIQUIDITY:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            sanityCheckNumberOfUnit(g_qswap_addLiquidityQuAmount);
            sanityCheckNumberOfUnit(g_qswap_addLiquidityAssetAmountDesired);
            sanityCheckTxAmount(g_qswap_liquidityAssetAmountMin);
            sanityCheckTxAmount(g_qswap_liquidityQuAmountMin);
            qswapAddLiquidity(g_nodeIp, g_nodePort, g_seed,
                            g_qswap_assetName,
                            g_qswap_issuer,
                            g_qswap_addLiquidityQuAmount,
                            g_qswap_addLiquidityAssetAmountDesired,
                            g_qswap_liquidityQuAmountMin,
                            g_qswap_liquidityAssetAmountMin,
                            g_offsetScheduledTick);
            break;
        case QSWAP_REMOVE_LIQUIDITY:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            sanityCheckNumberOfUnit(g_qswap_removeLiquidityBurnLiquidity);
            sanityCheckTxAmount(g_qswap_liquidityAssetAmountMin);
            sanityCheckTxAmount(g_qswap_liquidityQuAmountMin);
            qswapRemoveLiquidity(g_nodeIp, g_nodePort, g_seed,
                            g_qswap_assetName,
                            g_qswap_issuer,
                            g_qswap_removeLiquidityBurnLiquidity,
                            g_qswap_liquidityQuAmountMin,
                            g_qswap_liquidityAssetAmountMin,
                            g_offsetScheduledTick);
            break;
        case QSWAP_SWAP_EXACT_QU_FOR_ASSET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            sanityCheckTxAmount(g_qswap_swapAmountIn);
            sanityCheckTxAmount(g_qswap_swapAmountOutMin);
            qswapSwapExactQuForAsset(g_nodeIp, g_nodePort, g_seed,
                                     g_qswap_assetName,
                                     g_qswap_issuer,
                                     g_qswap_swapAmountIn,
                                     g_qswap_swapAmountOutMin,
                                     g_offsetScheduledTick);
            break;
        case QSWAP_SWAP_QU_FOR_EXACT_ASSET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            sanityCheckTxAmount(g_qswap_swapAmountOut);
            sanityCheckTxAmount(g_qswap_swapAmountInMax);
            qswapSwapQuForExactAsset(g_nodeIp, g_nodePort, g_seed,
                                     g_qswap_assetName,
                                     g_qswap_issuer,
                                     g_qswap_swapAmountInMax,
                                     g_qswap_swapAmountOut,
                                     g_offsetScheduledTick);
            break;
        case QSWAP_SWAP_EXACT_ASSET_FOR_QU:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            sanityCheckTxAmount(g_qswap_swapAmountIn);
            sanityCheckTxAmount(g_qswap_swapAmountOutMin);
            qswapSwapExactAssetForQu(g_nodeIp, g_nodePort, g_seed,
                                     g_qswap_assetName,
                                     g_qswap_issuer,
                                     g_qswap_swapAmountIn,
                                     g_qswap_swapAmountOutMin,
                                     g_offsetScheduledTick);
            break;
        case QSWAP_SWAP_ASSET_FOR_EXACT_QU:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            sanityCheckTxAmount(g_qswap_swapAmountOut);
            sanityCheckTxAmount(g_qswap_swapAmountInMax);
            qswapSwapAssetForExactQu(g_nodeIp, g_nodePort, g_seed,
                                     g_qswap_assetName,
                                     g_qswap_issuer,
                                     g_qswap_swapAmountInMax,
                                     g_qswap_swapAmountOut,
                                     g_offsetScheduledTick);
            break;
        case QSWAP_GET_POOL_BASIC:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            qswapGetPoolBasicState(g_nodeIp, g_nodePort,
                                   g_qswap_assetName,
                                   g_qswap_issuer);
            break;
        case QSWAP_GET_LIQUIDITY_OF:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            sanityCheckValidString(g_qswap_getLiquidityOfStakerIssuer);
            qswapGetLiquidityOf(g_nodeIp, g_nodePort,
                               g_qswap_assetName,
                               g_qswap_issuer,
                               g_qswap_getLiquidityOfStakerIssuer);
            break;
        case QSWAP_QUOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckValidAssetName(g_qswap_assetName);
            sanityCheckValidString(g_qswap_issuer);
            sanityCheckNumberOfUnit(g_qswap_quoteAmount);
            sanityCheckValidString(g_qswap_command1);
            if (strcmp(g_qswap_command1, "exact_qu_input")==0)
            {
                qswapQuoteExactQuInput(g_nodeIp, g_nodePort,
                                    g_qswap_assetName,
                                    g_qswap_issuer,
                                    g_qswap_quoteAmount);
            }
            else if (strcmp(g_qswap_command1, "exact_qu_output")==0)
            {
                qswapQuoteExactQuOutput(g_nodeIp, g_nodePort,
                                    g_qswap_assetName,
                                    g_qswap_issuer,
                                    g_qswap_quoteAmount);
            }
            else if (strcmp(g_qswap_command1, "exact_asset_input")==0)
            {
                qswapQuoteExactAssetInput(g_nodeIp, g_nodePort,
                                    g_qswap_assetName,
                                    g_qswap_issuer,
                                    g_qswap_quoteAmount);
            }
            else if (strcmp(g_qswap_command1, "exact_asset_output")==0)
            {
                qswapQuoteExactAssetOutput(g_nodeIp, g_nodePort,
                                    g_qswap_assetName,
                                    g_qswap_issuer,
                                    g_qswap_quoteAmount);
            }
            break;
        case NOSTROMO_REGISTER_IN_TIER:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            registerInTier(g_nodeIp,g_nodePort,g_seed,
                           g_offsetScheduledTick, g_nost_tierLevel);
            break;
        }
        case NOSTROMO_LOGOUT_FROM_TIER:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            logoutFromTier(g_nodeIp,g_nodePort,g_seed,
                           g_offsetScheduledTick);
            break;
        }
        case NOSTROMO_CREATE_PROJECT:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_nost_tokenName);
            createProject(g_nodeIp,g_nodePort,g_seed,
                           g_offsetScheduledTick,
                        g_nost_tokenName, g_nost_supply,
                        g_nost_startYear, g_nost_startMonth, g_nost_startDay, g_nost_startHour,
                        g_nost_endYear, g_nost_endMonth, g_nost_endDay, g_nost_endHour);
            break;
        }
        case NOSTROMO_VOTE_IN_PROJECT:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            voteInProject(g_nodeIp,g_nodePort,g_seed,
                           g_offsetScheduledTick,
                        g_nost_indexOfProject, g_nost_decision);
            break;
        }
        case NOSTROMO_CREATE_FUNDRAISING:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            createFundraising(g_nodeIp,g_nodePort,g_seed,
                           g_offsetScheduledTick,
                        g_nost_tokenPrice, g_nost_soldAmount, g_nost_requiredFunds,
                        g_nost_indexOfProject ,g_nost_firstPhaseStartYear, g_nost_firstPhaseStartMonth, g_nost_firstPhaseStartDay,
                        g_nost_firstPhaseStartHour, g_nost_firstPhaseEndYear, g_nost_firstPhaseEndMonth, g_nost_firstPhaseEndDay, g_nost_firstPhaseEndHour,
                        g_nost_secondPhaseStartYear, g_nost_secondPhaseStartMonth, g_nost_secondPhaseStartDay, g_nost_secondPhaseStartHour,
                        g_nost_secondPhaseEndYear, g_nost_secondPhaseEndMonth, g_nost_secondPhaseEndDay, g_nost_secondPhaseEndHour,
                        g_nost_thirdPhaseStartYear, g_nost_thirdPhaseStartMonth, g_nost_thirdPhaseStartDay, g_nost_thirdPhaseStartHour, g_nost_thirdPhaseEndYear,
                        g_nost_thirdPhaseEndMonth, g_nost_thirdPhaseEndDay, g_nost_thirdPhaseEndHour,
                        g_nost_listingStartYear, g_nost_listingStartMonth, g_nost_listingStartDay, g_nost_listingStartHour,
                        g_nost_cliffEndYear, g_nost_cliffEndMonth, g_nost_cliffEndDay, g_nost_cliffEndHour,
                        g_nost_vestingEndYear, g_nost_vestingEndMonth, g_nost_vestingEndDay, g_nost_vestingEndHour,
                        g_nost_threshold, g_nost_TGE, g_nost_stepOfVesting);
            break;
        }
        case NOSTROMO_INVEST_IN_FUNDRAISING:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            investInProject(g_nodeIp,g_nodePort,g_seed,
                           g_offsetScheduledTick,
                        g_nost_indexOfFundraising, g_nost_amount);
            break;
        }
        case NOSTROMO_CLAIM_TOKEN:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            claimToken(g_nodeIp,g_nodePort,g_seed,
                           g_offsetScheduledTick,
                        g_nost_amount, g_nost_indexOfFundraising);
            break;
        }
        case NOSTROMO_UPGRADE_TIER_LEVEL:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            upgradeTierLevel(g_nodeIp,g_nodePort,g_seed,
                           g_offsetScheduledTick,
                        g_nost_tierLevel);
            break;
        }
        case NOSTROMO_TRANSFER_SHARE_MANAGEMENT_RIGHTS:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            nostromoTransferShareManagementRights(g_nodeIp, g_nodePort, g_seed, 
                g_offsetScheduledTick, 
                g_nost_identity, 
                g_nost_tokenName, 
                g_nost_numberOfShare, 
                g_nost_newManagementContractIndex);
            break;
        }
        case NOSTROMO_GET_STATS:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getStats(g_nodeIp,g_nodePort);
            break;
        }
        case NOSTROMO_GET_TIER_LEVEL_BY_USER:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getTierLevelByUser(g_nodeIp,g_nodePort,
                        g_nost_identity);
            break;
        }
        case NOSTROMO_GET_USER_VOTE_STATUS:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getUserVoteStatus(g_nodeIp,g_nodePort,
                        g_nost_identity);
            break;
        }
        case NOSTROMO_CHECK_TOKEN_CREATABILITY:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            checkTokenCreatability(g_nodeIp,g_nodePort,
                        g_nost_tokenName);
            break;
        }
        case NOSTROMO_GET_NUMBER_OF_INVESTED_PROJECTS:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getNumberOfInvestedProjects(g_nodeIp,g_nodePort,
                        g_nost_identity);
            break;
        }
        case NOSTROMO_GET_PROJECT_BY_INDEX:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getProjectByIndex(g_nodeIp,g_nodePort,
                        g_nost_indexOfProject);
            break;
        }
        case NOSTROMO_GET_FUNDRAISING_BY_INDEX:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getFundarasingByIndex(g_nodeIp,g_nodePort,
                        g_nost_indexOfFundraising);
            break;
        }
        case NOSTROMO_GET_PROJECT_INDEX_LIST_BY_CREATOR:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getProjectIndexListByCreator(g_nodeIp,g_nodePort,
                        g_nost_identity);
            break;
        }
        case NOSTROMO_GET_INFO_USER_INVESTED:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getInfoUserInvested(g_nodeIp,g_nodePort,
                        g_nost_identity);
            break;
        }
        case NOSTROMO_GET_MAX_CLAIM_AMOUNT:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            getMaxClaimAmount(g_nodeIp,g_nodePort,
                        g_nost_identity,
                        g_nost_indexOfFundraising);
            break;
        }
        case MSVAULT_DEPOSIT_ASSET_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_msVaultAssetName);
            sanityCheckIdentity(g_msVaultIssuer);
            sanityCheckTxAmount(g_txAmount);
            msvaultDepositAsset(g_nodeIp, g_nodePort, g_seed, g_msvault_id, g_msVaultAssetName, g_msVaultIssuer, g_txAmount, g_offsetScheduledTick);
            break;
        }
        case MSVAULT_RELEASE_ASSET_TO_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_msVaultAssetName);
            sanityCheckIdentity(g_msVaultIssuer);
            sanityCheckTxAmount(g_txAmount);
            sanityCheckIdentity(g_msvault_destination);
            msvaultReleaseAssetTo(g_nodeIp, g_nodePort, g_seed, g_msvault_id, g_msVaultAssetName, g_msVaultIssuer, g_txAmount, g_msvault_destination, g_offsetScheduledTick);
            break;
        }
        case MSVAULT_RESET_ASSET_RELEASE_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            msvaultResetAssetRelease(g_nodeIp, g_nodePort, g_seed, g_msvault_id, g_offsetScheduledTick);
            break;
        }
        case MSVAULT_GET_ASSET_BALANCES_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetVaultAssetBalances(g_nodeIp, g_nodePort, g_msvault_id);
            break;
        }
        case MSVAULT_GET_ASSET_RELEASE_STATUS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetAssetReleaseStatus(g_nodeIp, g_nodePort, g_msvault_id);
            break;
        }
        case QBOND_STAKE_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qbondStake(g_nodeIp, g_nodePort, g_seed, g_qbond_millionsOfQu, g_offsetScheduledTick);
            break;
        }
        case QBOND_TRANSFER_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qbondTransfer(g_nodeIp, g_nodePort, g_seed, g_qbond_targetIdentity, g_qbond_epoch, g_qbond_mbondsAmount, g_offsetScheduledTick);
            break;
        }
        case QBOND_ADD_ASK_ORDER_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qbondAddAskOrder(g_nodeIp, g_nodePort, g_seed, g_qbond_epoch, g_qbond_mbondPrice, g_qbond_mbondsAmount, g_offsetScheduledTick);
            break;
        }
        case QBOND_REMOVE_ASK_ORDER_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qbondRemoveAskOrder(g_nodeIp, g_nodePort, g_seed, g_qbond_epoch, g_qbond_mbondPrice, g_qbond_mbondsAmount, g_offsetScheduledTick);
            break;
        }
        case QBOND_ADD_BID_ORDER_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qbondAddBidOrder(g_nodeIp, g_nodePort, g_seed, g_qbond_epoch, g_qbond_mbondPrice, g_qbond_mbondsAmount, g_offsetScheduledTick);
            break;
        }
        case QBOND_REMOVE_BID_ORDER_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qbondRemoveBidOrder(g_nodeIp, g_nodePort, g_seed, g_qbond_epoch, g_qbond_mbondPrice, g_qbond_mbondsAmount, g_offsetScheduledTick);
            break;
        }
        case QBOND_BURN_QU_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qbondBurn(g_nodeIp, g_nodePort, g_seed, g_qbond_burnAmount, g_offsetScheduledTick);
            break;
        }
        case QBOND_UPDATE_CFA_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qbondUpdateCFA(g_nodeIp, g_nodePort, g_seed, g_qbond_targetIdentity, g_qbond_updateCFAOperation, g_offsetScheduledTick);
            break;
        }
        case QBOND_GET_FEES_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            qbondGetFees(g_nodeIp, g_nodePort);
            break;
        }
        case QBOND_GET_EARNED_FEES_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            qbondGetEarnedFees(g_nodeIp, g_nodePort);
            break;
        }
        case QBOND_GET_EPOCH_INFO_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            qbondGetInfoPerEpoch(g_nodeIp, g_nodePort, g_qbond_epoch);
            break;
        }
        case QBOND_GET_ORDERS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            qbondGetOrders(g_nodeIp, g_nodePort, g_qbond_epoch, g_qbond_asksOffset, g_qbond_bidsOffset);
            break;
        }
        case QBOND_GET_USER_ORDERS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            qbondGetUserOrders(g_nodeIp, g_nodePort, g_qbond_owner, g_qbond_asksOffset, g_qbond_bidsOffset);
            break;
        }
        case QBOND_GET_TABLE_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            qbondGetTable(g_nodeIp, g_nodePort);
            break;
        }
        case QBOND_GET_USER_MBONDS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            qbondGetUserMBonds(g_nodeIp, g_nodePort, g_qbond_owner);
            break;
        }
        case QBOND_GET_CFA_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            qbondGetCFA(g_nodeIp, g_nodePort);
            break;
        }
        case SHAREHOLDER_SET_PROPOSAL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            shareholderSetProposal(g_nodeIp, g_nodePort, g_seed, g_contractIndex, g_proposalString, g_offsetScheduledTick, g_force);
            break;
        case SHAREHOLDER_CLEAR_PROPOSAL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            shareholderClearProposal(g_nodeIp, g_nodePort, g_seed, g_contractIndex, g_offsetScheduledTick);
            break;
        case SHAREHOLDER_GET_PROPOSALS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            shareholderGetProposals(g_nodeIp, g_nodePort, g_contractIndex, g_proposalString);
            break;
        case SHAREHOLDER_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            shareholderVote(g_nodeIp, g_nodePort, g_seed, g_contractIndex, g_proposalString, g_voteValueString, g_offsetScheduledTick, g_force);
            break;
        case SHAREHOLDER_GET_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            if (g_requestedIdentity)
                sanityCheckIdentity(g_requestedIdentity);
            else
                sanityCheckSeed(g_seed);
            shareholderGetVote(g_nodeIp, g_nodePort, g_contractIndex, g_proposalString, g_requestedIdentity, g_seed);
            break;
        case SHAREHOLDER_GET_VOTING_RESULTS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            shareholderGetVotingResults(g_nodeIp, g_nodePort, g_contractIndex, g_proposalString);
            break;
        case TEST_QPI_FUNCTIONS_OUTPUT:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            testQpiFunctionsOutput(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick);
            break;
        }
        case TEST_QPI_FUNCTIONS_OUTPUT_PAST:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            testQpiFunctionsOutputPast(g_nodeIp, g_nodePort);
            break;
        }
        case TEST_GET_INCOMING_TRANSFER_AMOUNTS:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            testGetIncomingTransferAmounts(g_nodeIp, g_nodePort, g_paramString1);
            break;
        }
        case TEST_BID_IN_IPO_THROUGH_CONTRACT:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            testBidInIpoThroughContract(g_nodeIp, g_nodePort, g_seed, g_paramString1, g_IPOContractIndex, g_makeIPOBidPricePerShare, g_makeIPOBidNumberOfShare, g_offsetScheduledTick);
            break;
        }
        default:
            printf("Unexpected command!\n");
            break;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        return run(argc, argv);
    }
    catch (std::exception & ex)
    {
        printf("%s\n", ex.what());
        return -1;
    }
}
