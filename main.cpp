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
#include "nostromo.h"
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
        case GET_TOTAL_NUMBER_OF_ASSET_SHARES:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilGetTotalNumberOfAssetShares(g_nodeIp, g_nodePort, g_paramString1, g_paramString2);
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
        case QUTIL_CREATE_POLL:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckValidString(g_qutil_poll_name_str);
            sanityCheckTxAmount(g_qutil_min_amount);
            sanityCheckValidString(g_qutil_github_link_str);
            if (g_qutil_poll_type == 2)
            {
                sanityCheckValidString(g_qutil_semicolon_separated_assets);
            }
            qutilCreatePoll(g_nodeIp, g_nodePort, g_seed, g_qutil_poll_name_str, g_qutil_poll_type,
                g_qutil_min_amount, g_qutil_github_link_str, g_qutil_semicolon_separated_assets,
                g_offsetScheduledTick);
            break;
        case QUTIL_VOTE:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckTxAmount(g_qutil_vote_amount);
            qutilVote(g_nodeIp, g_nodePort, g_seed, g_qutil_vote_poll_id, g_qutil_vote_amount,
                g_qutil_vote_chosen_option, g_offsetScheduledTick);
            break;
        case QUTIL_GET_CURRENT_RESULT:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilGetCurrentResult(g_nodeIp, g_nodePort, g_qutil_get_result_poll_id);
            break;
        case QUTIL_GET_POLLS_BY_CREATOR:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckIdentity(g_qutil_get_polls_creator_address);
            qutilGetPollsByCreator(g_nodeIp, g_nodePort, g_qutil_get_polls_creator_address);
            break;
        case QUTIL_GET_CURRENT_POLL_ID:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilGetCurrentPollId(g_nodeIp, g_nodePort);
            break;
        case QUTIL_GET_POLL_INFO:
            sanityCheckNode(g_nodeIp, g_nodePort);
            qutilGetPollInfo(g_nodeIp, g_nodePort, g_qutil_get_poll_info_poll_id);
            break;
        case QUTIL_CANCEL_POLL: {
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            qutilCancelPoll(g_nodeIp, g_nodePort, g_seed, g_qutil_cancel_poll_id, g_offsetScheduledTick);
            break;
        }
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
        case QVAULT_SUBMIT_AUTH_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitAuthAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvaultIdentity);
            break;
        case QVAULT_CHANGE_AUTH_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            changeAuthAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_numberOfChangedAddress);
            break;
        case QVAULT_SUBMIT_FEES:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitFees(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_newQCAPHolder_fee, g_qvault_newreinvesting_fee, g_qvault_newdev_fee);
            break;
        case QVAULT_CHANGE_FEES:
            sanityCheckNode(g_nodeIp, g_nodePort);
            changeFees(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvault_newQCAPHolder_fee, g_qvault_newreinvesting_fee, g_qvault_newdev_fee);
            break;
        case QVAULT_SUBMIT_REINVESTING_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitReinvestingAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvaultIdentity);
            break;
        case QVAULT_CHANGE_REINVESTING_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            changeReinvestingAddress(g_nodeIp, g_nodePort, g_seed,  g_offsetScheduledTick, g_qvaultIdentity);
            break;
        case QVAULT_SUBMIT_ADMIN_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitAdminAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvaultIdentity);
            break;
        case QVAULT_CHANGE_ADMIN_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            changeAdminAddress(g_nodeIp, g_nodePort, g_seed,  g_offsetScheduledTick, g_qvaultIdentity);
            break;
        case QVAULT_GET_DATA:
            sanityCheckNode(g_nodeIp, g_nodePort);
            getData(g_nodeIp, g_nodePort);
            break;
        case QVAULT_SUBMIT_BANNED_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitBannedAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvaultIdentity);
            break;
        case QVAULT_SAVE_BANNED_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            saveBannedAddress(g_nodeIp, g_nodePort, g_seed,  g_offsetScheduledTick, g_qvaultIdentity);
            break;
        case QVAULT_SUBMIT_UNBANNED_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            submitUnbannedannedAddress(g_nodeIp, g_nodePort, g_seed, g_offsetScheduledTick, g_qvaultIdentity);
            break;
        case QVAULT_SAVE_UNBANNED_ADDRESS:
            sanityCheckNode(g_nodeIp, g_nodePort);
            saveUnbannedAddress(g_nodeIp, g_nodePort, g_seed,  g_offsetScheduledTick, g_qvaultIdentity);
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
            testBidInIpoThroughContract(g_nodeIp, g_nodePort, g_seed, g_paramString1, g_ipo_contract_index, g_make_ipo_bid_price_per_share, g_make_ipo_bid_number_of_share, g_offsetScheduledTick);
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
