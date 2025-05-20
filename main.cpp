#include <array>
#include "stdio.h"
#include "structs.h"
#include "global.h"
#include "argparser.h"
#include "walletUtils.h"
#include "nodeUtils.h"
#include "assetUtil.h"
#include "keyUtils.h"
#include "sanityCheck.h"
#include "SCUtils.h"
#include "quottery.h"
#include "qutil.h"
#include "qx.h"
#include "proposal.h"
#include "qearn.h"
#include "qvault.h"
#include "msvault.h"
#include "testUtils.h"

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
        case SEND_COIN:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckIdentity(g_targetIdentity);
            sanityCheckTxAmount(g_TxAmount);
            makeStandardTransaction(g_nodeIp, g_nodePort, g_seed, g_targetIdentity, g_TxAmount, g_offsetScheduledTick, g_waitUntilFinish);
            break;
        case SEND_COIN_IN_TICK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckIdentity(g_targetIdentity);
            sanityCheckTxAmount(g_TxAmount);
            makeStandardTransactionInTick(g_nodeIp, g_nodePort, g_seed, g_targetIdentity, g_TxAmount, g_TxTick, g_waitUntilFinish);
            break;
        case SEND_CUSTOM_TX:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckTxType(g_TxType);
            sanityCheckIdentity(g_targetIdentity);
            sanityCheckTxAmount(g_TxAmount);
            sanityCheckExtraDataSize(g_txExtraDataSize);
            makeCustomTransaction(g_nodeIp, g_nodePort, g_seed, g_targetIdentity,
                                  g_TxType, g_TxAmount, g_txExtraDataSize,
                                  g_txExtraData, g_offsetScheduledTick);

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
            sanityCheckNumberOfUnit(g_qx_issue_asset_number_of_unit);
            sanityCheckValidAssetName(g_qx_issue_asset_name);
            sanityCheckValidString(g_qx_issue_unit_of_measurement);
            sanityCheckNumberOfDecimal(g_qx_issue_asset_num_decimal);
            qxIssueAsset(g_nodeIp, g_nodePort, g_seed,
                         g_qx_issue_asset_name,
                         g_qx_issue_unit_of_measurement,
                         g_qx_issue_asset_number_of_unit,
                         g_qx_issue_asset_num_decimal,
                         g_offsetScheduledTick);
            break;
        case QX_TRANSFER_ASSET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckNumberOfUnit(g_qx_asset_transfer_amount);
            sanityCheckValidAssetName(g_qx_asset_transfer_asset_name);
            sanityCheckValidString(g_qx_asset_transfer_issuer_in_hex);
            sanityCheckIdentity(g_qx_asset_transfer_new_owner_identity);
            qxTransferAsset(g_nodeIp, g_nodePort, g_seed,
                            g_qx_asset_transfer_asset_name,
                            g_qx_asset_transfer_issuer_in_hex,
                            g_qx_asset_transfer_new_owner_identity,
                            g_qx_asset_transfer_amount,
                            g_offsetScheduledTick);
            break;
        case QX_ORDER:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qx_asset_name);
            sanityCheckValidString(g_qx_command_1);
            sanityCheckValidString(g_qx_command_2);
            if (strcmp(g_qx_command_1, "add") == 0)
            {
                if (strcmp(g_qx_command_2, "bid") == 0)
                {
                    qxAddToBidOrder(g_nodeIp, g_nodePort, g_seed, g_qx_asset_name, g_qx_issuer, g_qx_price, g_qx_number_of_share, g_offsetScheduledTick);
                }
                else if (strcmp(g_qx_command_2, "ask") == 0)
                {
                    qxAddToAskOrder(g_nodeIp, g_nodePort, g_seed, g_qx_asset_name, g_qx_issuer, g_qx_price, g_qx_number_of_share, g_offsetScheduledTick);
                }
            }
            else if (strcmp(g_qx_command_1, "remove") == 0)
            {
                if (strcmp(g_qx_command_2, "bid") == 0)
                {
                    qxRemoveToBidOrder(g_nodeIp, g_nodePort, g_seed, g_qx_asset_name, g_qx_issuer, g_qx_price, g_qx_number_of_share, g_offsetScheduledTick);
                }
                else if (strcmp(g_qx_command_2, "ask") == 0)
                {
                    qxRemoveToAskOrder(g_nodeIp, g_nodePort, g_seed, g_qx_asset_name, g_qx_issuer, g_qx_price, g_qx_number_of_share, g_offsetScheduledTick);
                }
            }
            break;
        case QX_GET_ORDER:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckValidString(g_qx_command_1);
            sanityCheckValidString(g_qx_command_2);
            if (strcmp(g_qx_command_1, "entity") == 0)
            {
                if (strcmp(g_qx_command_2, "bid") == 0)
                {
                    qxGetEntityBidOrder(g_nodeIp, g_nodePort, g_qx_issuer, g_qx_offset);
                }
                else if (strcmp(g_qx_command_2, "ask") == 0)
                {
                    qxGetEntityAskOrder(g_nodeIp, g_nodePort, g_qx_issuer, g_qx_offset);
                }
            }
            else if (strcmp(g_qx_command_1, "asset") == 0)
            {
                sanityCheckValidAssetName(g_qx_asset_name);
                if (strcmp(g_qx_command_2, "bid") == 0)
                {
                    qxGetAssetBidOrder(g_nodeIp, g_nodePort, g_qx_asset_name, g_qx_issuer, g_qx_offset);
                }
                else if (strcmp(g_qx_command_2, "ask") == 0)
                {
                    qxGetAssetAskOrder(g_nodeIp, g_nodePort, g_qx_asset_name, g_qx_issuer, g_qx_offset);
                }
            }
            break;
        case QX_TRANSFER_MANAGEMENT_RIGHTS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidAssetName(g_qx_asset_name);
            sanityCheckIdentity(g_qx_issuer);
            sanityCheckNumberOfUnit(g_qx_number_of_share);
            qxTransferAssetManagementRights(g_nodeIp, g_nodePort, g_seed, g_qx_asset_name, g_qx_issuer, g_contract_index, g_qx_number_of_share, g_offsetScheduledTick);
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
            uploadFile(g_nodeIp, g_nodePort, g_file_path, g_seed, g_offsetScheduledTick, g_compress_tool);
            break;
        case DOWNLOAD_FILE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            downloadFile(g_nodeIp, g_nodePort, g_requestedTxId, g_file_path, g_compress_tool);
            break;
        case DUMP_SPECTRUM_FILE:
            sanityFileExist(g_dump_binary_file_input);
            sanityCheckValidString(g_dump_binary_file_output);
            dumpSpectrumToCSV(g_dump_binary_file_input, g_dump_binary_file_output);
            break;
        case DUMP_UNIVERSE_FILE:
            sanityFileExist(g_dump_binary_file_input);
            sanityCheckValidString(g_dump_binary_file_output);
            dumpUniverseToCSV(g_dump_binary_file_input, g_dump_binary_file_output);
            break;
        case DUMP_CONTRACT_FILE:
            sanityFileExist(g_dump_binary_file_input);
            sanityCheckValidString(g_dump_binary_file_output);
            dumpContractToCSV(g_dump_binary_file_input, g_dump_binary_contract_id, g_dump_binary_file_output);
            break;
        case PRINT_QX_FEE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printQxFee(g_nodeIp, g_nodePort);
            break;
        case MAKE_IPO_BID:
            sanityCheckNode(g_nodeIp, g_nodePort);
            makeIPOBid(g_nodeIp, g_nodePort, g_seed, g_ipo_contract_index, g_make_ipo_bid_price_per_share, g_make_ipo_bid_number_of_share, g_offsetScheduledTick);
            break;
        case GET_IPO_STATUS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            printIPOStatus(g_nodeIp, g_nodePort, g_ipo_contract_index);
            break;
        case QUOTTERY_ISSUE_BET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            quotteryIssueBet(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick);
            break;
        case QUOTTERY_GET_BET_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            quotteryPrintBetInfo(g_nodeIp, g_nodePort, g_quottery_bet_id);
            break;
        case QUOTTERY_JOIN_BET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            quotteryJoinBet(g_nodeIp, g_nodePort, g_seed, g_quottery_bet_id, int(g_quottery_number_bet_slot), g_quottery_amount_per_bet_slot, g_quottery_picked_option, g_offsetScheduledTick);
            break;
        case QUOTTERY_GET_BET_DETAIL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            quotteryPrintBetOptionDetail(g_nodeIp, g_nodePort, g_quottery_bet_id, g_quottery_option_id);
            break;
        case QUOTTERY_GET_ACTIVE_BET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            quotteryPrintActiveBet(g_nodeIp, g_nodePort);
            break;
        case QUOTTERY_GET_ACTIVE_BET_BY_CREATOR:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckIdentity(g_quottery_creator_id);
            quotteryPrintActiveBetByCreator(g_nodeIp, g_nodePort, g_quottery_creator_id);
            break;
        case QUOTTERY_GET_BASIC_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            quotteryPrintBasicInfo(g_nodeIp, g_nodePort);
            break;
        case QUOTTERY_PUBLISH_RESULT:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            quotteryPublishResult(g_nodeIp, g_nodePort, g_seed, g_quottery_bet_id, g_quottery_option_id, g_offsetScheduledTick);
            break;
        case QUOTTERY_CANCEL_BET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            quotteryCancelBet(g_nodeIp, g_nodePort, g_seed, g_quottery_bet_id, g_offsetScheduledTick);
            break;
        case TOOGLE_MAIN_AUX:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckMainAuxStatus(g_toggle_main_aux_0);
            sanityCheckMainAuxStatus(g_toggle_main_aux_1);
            toggleMainAux(g_nodeIp, g_nodePort, g_seed, g_toggle_main_aux_0, g_toggle_main_aux_1);
            break;
        case SET_SOLUTION_THRESHOLD:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            checkValidEpoch(g_set_solution_threshold_epoch);
            checkValidSolutionThreshold(g_set_solution_threshold_value);
            setSolutionThreshold(g_nodeIp, g_nodePort, g_seed, g_set_solution_threshold_epoch, g_set_solution_threshold_value);
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
            sanityFileExist(g_qutil_sendtomanyv1_payout_list_file);
            qutilSendToManyV1(g_nodeIp, g_nodePort, g_seed, g_qutil_sendtomanyv1_payout_list_file, g_offsetScheduledTick);
            break;
        case QUTIL_BURN_QUBIC:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qutilBurnQubic(g_nodeIp, g_nodePort, g_seed, g_TxAmount, g_offsetScheduledTick);
            break;
        case QUTIL_SEND_TO_MANY_BENCHMARK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qutilSendToManyBenchmark(g_nodeIp, g_nodePort, g_seed, uint32_t(g_qutil_sendtomanybenchmark_destination_count), uint32_t(g_qutil_sendtomanybenchmark_num_transfers_each), g_offsetScheduledTick);
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
        case QEARN_LOCK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qearnLock(g_nodeIp, g_nodePort, g_seed, g_qearn_lock_amount, g_offsetScheduledTick);
            break;
        case QEARN_UNLOCK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qearnUnlock(g_nodeIp, g_nodePort, g_seed, g_qearn_unlock_amount, g_qearn_locked_epoch, g_offsetScheduledTick);
            break;
        case QEARN_GET_INFO_PER_EPOCH:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetInfoPerEpoch(g_nodeIp, g_nodePort, g_qearn_getinfo_epoch);
            break;
        case QEARN_GET_USER_LOCKED_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetUserLockedInfo(g_nodeIp, g_nodePort, g_requestedIdentity, g_qearn_getinfo_epoch);
            break;
        case QEARN_GET_STATE_OF_ROUND:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetStateOfRound(g_nodeIp, g_nodePort, g_qearn_getinfo_epoch);
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
            qearnGetStatsPerEpoch(g_nodeIp, g_nodePort, g_qearn_getstats_epoch);
            break;
        case QEARN_GET_BURNED_AND_BOOSTED_STATS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetBurnedAndBoostedStats(g_nodeIp, g_nodePort);
            break;
        case QEARN_GET_BURNED_AND_BOOSTED_STATS_PER_EPOCH:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qearnGetBurnedAndBoostedStatsPerEpoch(g_nodeIp, g_nodePort, g_qearn_getstats_epoch);
            break;
        case QVAULT_COMMAND_STAKE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            stake(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_stake_amount);
            break;
        case QVAULT_COMMAND_UNSTAKE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            unStake(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_stake_amount);
            break;
        case QVAULT_COMMAND_SUBMIT_GP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitGP(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick);
            break;
        case QVAULT_COMMAND_SUBMIT_QCP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitQCP(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_permille);
            break;
        case QVAULT_COMMAND_SUBMIT_IPOP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitIPOP(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_ipo_contract_index);
            break;
        case QVAULT_COMMAND_SUBMIT_QEARNP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitQEarnP(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_amount_of_qubic, g_qvault_number_of_epoch);
            break;
        case QVAULT_COMMAND_SUBMIT_FUNDP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitFundP(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_price_of_qcap, g_qvault_amount_of_qcap);
            break;
        case QVAULT_COMMAND_SUBMIT_MKTP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitMKTP(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_amount_of_qubic, g_qvault_share_name, g_qvault_amount_of_qcap, g_qvault_index_of_share, g_qvault_amount_of_share);
            break;
        case QVAULT_COMMAND_SUBMIT_ALLOP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitAlloP(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_reinvested, g_qvault_team, g_qvault_burn, g_qvault_distribute);
            break;
        case QVAULT_COMMAND_SUBMIT_MSP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitMSP(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_share_index);
            break;
        case QVAULT_COMMAND_VOTE_IN_PROPOSAL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            voteInProposal(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_price_of_ipo, g_qvault_proposal_type, g_qvault_proposal_id, g_qvault_yes);
            break;
        case QVAULT_COMMAND_BUY_QCAP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            buyQcap(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_amount_of_qcap, g_qvault_price_of_qcap);
            break;
        case QVAULT_COMMAND_TRANSFER_SHARE_MANAGEMENT_RIGHTS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            TransferShareManagementRights(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvaultIdentity, g_qvault_assetname, g_qvault_number_of_share, g_qvault_newmanagement_contract_index);
            break;
        case QVAULT_COMMAND_SUBMIT_MUSLIMID:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            submitMuslimId(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick);
            break;
        case QVAULT_COMMAND_CANCEL_MUSLIMID:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            cancelMuslimId(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick);
            break;
        case QVAULT_COMMAND_GETDATA:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getData(g_nodeIp, g_nodePort);
            break;
        case QVAULT_COMMAND_GET_STAKED_AMOUNT_AND_VOTING_POWER:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getStakedAmountAndVotingPower(g_nodeIp, g_nodePort, g_qvaultIdentity);
            break;
        case QVAULT_COMMAND_GET_GP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getGP(g_nodeIp, g_nodePort, g_qvault_proposal_id);
            break;
        case QVAULT_COMMAND_GET_QCP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getQCP(g_nodeIp, g_nodePort, g_qvault_proposal_id);
            break;
        case QVAULT_COMMAND_GET_IPOP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getIPOP(g_nodeIp, g_nodePort, g_qvault_proposal_id);
            break;
        case QVAULT_COMMAND_GET_QEARNP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getQEarnP(g_nodeIp, g_nodePort, g_qvault_proposal_id);
            break;
        case QVAULT_COMMAND_GET_FUNDP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getFundP(g_nodeIp, g_nodePort, g_qvault_proposal_id);
            break;
        case QVAULT_COMMAND_GET_MKTP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getMKTP(g_nodeIp, g_nodePort, g_qvault_proposal_id);
            break;
        case QVAULT_COMMAND_GET_ALLOP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getAlloP(g_nodeIp, g_nodePort, g_qvault_proposal_id);
            break;
        case QVAULT_COMMAND_GET_MSP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getMSP(g_nodeIp, g_nodePort, g_qvault_proposal_id);
            break;
        case QVAULT_COMMAND_GET_IDENTITIES_HV_VT_PW:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getIdentitiesHvVtPw(g_nodeIp, g_nodePort, g_qvault_offset, g_qvault_count);
            break;
        case QVAULT_COMMAND_GET_PP_CREATION_POWER:
            sanityCheckNode(g_nodeIp, g_nodePort);
            ppCreationPower(g_nodeIp, g_nodePort, g_qvaultIdentity);
            break;
        case QVAULT_COMMAND_GET_QCAP_BURNT_AMOUNT_IN_LAST_EPOCHES:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getQcapBurntAmountInLastEpoches(g_nodeIp, g_nodePort, g_qvault_number_of_epoch);
            break;
        case QVAULT_COMMAND_GET_AMOUNT_TO_BE_SOLD_PER_YEAR:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getAmountToBeSoldPerYear(g_nodeIp, g_nodePort, g_qvault_year);
            break;
        case QVAULT_COMMAND_GET_TOTAL_REVENUE_IN_QCAP:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getTotalRevenueInQcap(g_nodeIp, g_nodePort);
            break;
        case QVAULT_COMMAND_GET_REVENUE_IN_QCAP_PER_EPOCH:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getRevenueInQcapPerEpoch(g_nodeIp, g_nodePort, g_qvault_epoch);
            break;
        case QVAULT_COMMAND_GET_REVENUE_PER_SHARE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getRevenuePerShare(g_nodeIp, g_nodePort, g_qvault_contract_index);
            break;
        case QVAULT_COMMAND_GET_AMOUNT_OF_SHARE_QVAULT_HOLD:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getAmountOfShareQvaultHold(g_nodeIp, g_nodePort, g_qvault_assetname, g_qvaultIdentity);
            break;
        case QVAULT_COMMAND_GET_NUMBER_OF_HOLDER_AND_AVG_AM:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getNumberOfHolderAndAvgAm(g_nodeIp, g_nodePort);
            break;
        case QVAULT_COMMAND_GET_AMOUNT_FOR_QEARN_IN_UPCOMING_EPOCH:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getAmountForQearnInUpcomingEpoch(g_nodeIp, g_nodePort, g_qvault_epoch);
            break;

        // MSVAULT
        case MSVAULT_REGISTER_VAULT_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            msvaultRegisterVault(g_nodeIp, g_nodePort, g_seed,
                g_msVaultRequiredApprovals, g_msVaultVaultName,
                g_msVaultOwnersCommaSeparated,
                g_offsetScheduledTick);
            break;
        }
        case MSVAULT_DEPOSIT_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            msvaultDeposit(g_nodeIp,g_nodePort,g_seed,
                           g_msVaultID, g_TxAmount, g_offsetScheduledTick);
            break;
        }
        case MSVAULT_RELEASE_TO_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckIdentity(g_msVaultDestination);
            msvaultReleaseTo(g_nodeIp,g_nodePort,g_seed,
                             g_msVaultID, g_TxAmount, g_msVaultDestination,
                             g_offsetScheduledTick);
            break;
        }
        case MSVAULT_RESET_RELEASE_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            msvaultResetRelease(g_nodeIp,g_nodePort,g_seed,
                                g_msVaultID, g_offsetScheduledTick);
            break;
        }
        case MSVAULT_GET_VAULTS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckIdentity(g_msVaultPublicId);
            msvaultGetVaults(g_nodeIp,g_nodePort,g_msVaultPublicId);
            break;
        }
        case MSVAULT_GET_RELEASE_STATUS_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetReleaseStatus(g_nodeIp,g_nodePort,g_msVaultID);
            break;
        }
        case MSVAULT_GET_BALANCE_OF_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetBalanceOf(g_nodeIp,g_nodePort,g_msVaultID);
            break;
        }
        case MSVAULT_GET_VAULT_NAME_CMD:
        {
            sanityCheckNode(g_nodeIp, g_nodePort);
            msvaultGetVaultName(g_nodeIp,g_nodePort,g_msVaultID);
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
            msvaultGetVaultOwners(g_nodeIp, g_nodePort, g_msVaultID);
            break;
        }
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
