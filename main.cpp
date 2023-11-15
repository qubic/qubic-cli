#include "stdio.h"
#include "structs.h"
#include "global.h"
#include "argparser.h"
#include "sanityCheck.h"
#include "walletUtils.h"
#include "nodeUtils.h"
#include "assetUtil.h"

int main(int argc, char *argv[])
{
	parseArgument(argc, argv);
	switch (g_cmd){
		case SHOW_KEYS:
            sanityCheckSeed(g_seed);
			printWalletInfo(g_seed);
			break;
		case GET_CURRENT_TICK:
			sanityCheckNode(g_nodeIp, g_nodePort);
			printTickInfoFromNode(g_nodeIp, g_nodePort);
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
			break;
		case SEND_COIN:
			sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
			sanityCheckIdentity(g_targetIdentity);
			sanityCheckTxAmount(g_TxAmount);
			makeStandardTransaction(g_nodeIp, g_nodePort, g_seed, g_targetIdentity, g_TxAmount, g_offsetScheduledTick);
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
        case CHECK_TX_ON_TICK:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckTxHash(g_requestedTxId);
            checkTxOnTick(g_nodeIp, g_nodePort, g_requestedTxId, g_requestedTickNumber);
            break;
		case SEND_SPECIAL_COMMAND:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckSpecialCommand(g_requestedSpecialCommand);
            sendSpecialCommand(g_nodeIp, g_nodePort, g_seed, g_requestedSpecialCommand);
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
        case READ_TICK_DATA:
            sanityFileExist(g_requestedFileName);
            printTickDataFromFile(g_requestedFileName);
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
		case QX_TRANSFER_ASSET:
            sanityCheckNode(g_nodeIp, g_nodePort);
            sanityCheckSeed(g_seed);
            sanityCheckIdentity(g_qx_share_transfer_possessed_identity);
            sanityCheckIdentity(g_qx_share_transfer_new_owner_identity);
            sanityCheckAmountTransferAsset(g_qx_share_transfer_amount);
            transferQxAsset(g_nodeIp, g_nodePort, g_seed,
                g_qx_share_transfer_possessed_identity,
                g_qx_share_transfer_new_owner_identity,
                  g_qx_share_transfer_amount,
              g_offsetScheduledTick);
			break;
		default:
			printf("Unexpected command!\n");
			break;
	}
    return 0;
}