#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sstream>
#include "logger.h"

#define CHECK_OVER_PARAMETERS if (i < argc)\
{ \
    LOG("Not accept any parameters after main COMMAND, unexpected %s\n", argv[i]); \
    exit(1); \
}

void print_help(){
    printf("./qubic-cli [basic config] [Command] [command extra parameters]\n");
    printf("-help print this message\n");
    printf("Basic config:\n");
    printf("\t-conf <file>\n");
    printf("\t\tSpecify configuration file. Relative paths will be prefixed by datadir location. See qubic.conf.example.\n");
    printf("\t\tNotice: variables in qubic.conf will be overrided by values on parameters.\n");
    printf("\t-seed <SEED>\n");
    printf("\t\t55-char seed private key\n");
    printf("\t-nodeip <IPv4_ADDRESS>\n");
    printf("\t\tIP address of the target node for querying blockchain information (default: 127.0.0.1)\n");
    printf("\t-nodeport <PORT>\n");
    printf("\t\tPort of the target node for querying blockchain information (default: 21841)\n");
    printf("\t-scheduletick <TICK_OFFSET>\n");
    printf("\t\tOffset number of scheduled tick that will perform a transaction (default: 20)\n");
    printf("Command:\n");
    printf("[WALLET COMMAND]\n");
    printf("\t-showkeys\n");
    printf("\t\tGenerating identity, pubkey key from private key. Private key must be passed either from params or configuration file.\n");
    printf("\t-getbalance <IDENTITY>\n");
    printf("\t\tBalance of an identity (amount of qubic, number of in/out txs)\n");
    printf("\t-getasset <IDENTITY>\n");
    printf("\t\tPrint a list of assets of an identity\n");
    printf("\t-sendtoaddress <TARGET_IDENTITY> <AMOUNT>\n");
    printf("\t\tPerform a standard transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY>. valid private key and node ip/port are required.\n");
    printf("\t-qutilsendtomanyv1 <FILE>\n");
    printf("\t\tPerforms multiple transaction within in one tick. <FILE> must contain one ID and amount (space seperated) per line. Max 25 transaction. Fees apply! valid private key and node ip/port are required.\n");
    printf("\n[BLOCKCHAIN/PROTOCOL COMMAND]\n");
    printf("\t-gettickdata <TICK_NUMBER> <OUTPUT_FILE_NAME>\n");
    printf("\t\tGet tick data and write it to a file. Use -readtickdata to examine the file. valid node ip/port are required.\n");
    printf("\t-getquorumtick <COMP_LIST_FILE> <TICK_NUMBER>\n");
    printf("\t\tGet quorum tick data, the summary of quorum tick will be printed, <COMP_LIST_FILE> is fetched by command -getcomputorlist. valid node ip/port are required.\n");
    printf("\t-getcomputorlist <OUTPUT_FILE_NAME>\n");
    printf("\t\tGet of the current epoch. Feed this data to -readtickdata to verify tick data. valid node ip/port are required.\n");
    printf("\t-getnodeiplist\n");
    printf("\t\tPrint a list of node ip from a seed node ip. Valid node ip/port are required.\n");
    printf("\t-checktxontick <TICK_NUMBER> <TX_ID>\n");
    printf("\t\tCheck if a transaction is included in a tick. valid node ip/port are required.\n");
    printf("\t-checktxonfile <TX_ID> <TICK_DATA_FILE>\n");
    printf("\t\tCheck if a transaction is included in a tick (tick data from a file). valid node ip/port are required.\n");
    printf("\t-readtickdata <FILE_NAME> <COMPUTOR_LIST>\n");
    printf("\t\tRead tick data from a file, print the output on screen, COMPUTOR_LIST is required if you need to verify block data\n");
    printf("\t-sendcustomtransaction <TARGET_IDENTITY> <TX_TYPE> <AMOUNT> <EXTRA_BYTE_SIZE> <EXTRA_BYTE_IN_HEX>\n");
    printf("\t\tPerform a custom transaction (IPO, querying smart contract), valid private key and node ip/port are required.\n");
    printf("\t-dumpspectrumfile <SPECTRUM_BINARY_FILE> <OUTPUT_CSV_FILE>\n");
    printf("\t\tDump spectrum file into csv.\n");
    printf("\t-dumpuniversefile <UNIVERSE_BINARY_FILE> <OUTPUT_CSV_FILE>\n");
    printf("\t\tDump spectrum file into csv.\n");
    printf("\t-makeipobid <CONTRACT_INDEX> <NUMBER_OF_SHARE> <PRICE_PER_SHARE>\n");
    printf("\t\tParticipating IPO (dutch auction). valid private key and node ip/port, CONTRACT_INDEX are required.\n");
    printf("\t-getipostatus <CONTRACT_INDEX>\n");
    printf("\t\tView IPO status. valid node ip/port, CONTRACT_INDEX are required.\n");
    printf("\t-getsysteminfo\n");
    printf("\t\tView Current System Status. Includes initial tick, random mining seed, epoch info.\n");
    printf("\t-publishproposal \n");
    printf("\t\t(on development)\n");

    printf("\n[NODE COMMAND]\n");
    printf("\t-getcurrenttick\n");
    printf("\t\tShow current tick information of a node\n");
    printf("\t-sendspecialcommand <COMMAND_IN_NUMBER> \n");
    printf("\t\tPerform a special command to node, valid private key and node ip/port are required.\t\n");
    printf("\t-tooglemainaux <MODE_0> <Mode_1> \n");
    printf("\t\tRemotely toogle Main/Aux mode on node,valid private key and node ip/port are required.\t\n");
    printf("\t\t<MODE_0> and <MODE_1> value are: MAIN or AUX\t\n");
    printf("\t-setsolutionthreshold <EPOCH> <SOLUTION_THRESHOLD> \n");
    printf("\t\tRemotely set solution threshold for future epoch,valid private key and node ip/port are required.\t\n");
    printf("\t-refreshpeerlist\n");
    printf("\t\t(equivalent to F4) Remotely refresh the peer list of node, all current connections will be closed after this command is sent, valid private key and node ip/port are required.\t\n");
    printf("\t-forcenexttick\n");
    printf("\t\t(equivalent to F5) Remotely force next tick on node to be empty, valid private key and node ip/port are required.\t\n");
    printf("\t-reissuevote\n");
    printf("\t\t(equivalent to F9) Remotely re-issue (re-send) vote on node, valid private key and node ip/port are required.\t\n");
    printf("\t-sendrawpacket <DATA_IN_HEX> <SIZE>\n");
    printf("\t\tSend a raw packet to nodeip. Valid node ip/port are required.\n");
    printf("\t-getlogfromnode <PASSCODE_0> <PASSCODE_1> <PASSCODE_2> <PASSCODE_3>\n");
    printf("\t\tFetch a single log line from the node. Valid node ip/port, passcodes are required.\n");
    printf("\t-synctime\n");
    printf("\t\tSync node time with local time, valid private key and node ip/port are required. Make sure that your local time is synced (with NTP)!\t\n");

    printf("\n[QX COMMAND]\n");
    printf("\t-qxgetfee\n");
    printf("\t\tShow current Qx fee.\n");
    printf("\t-qxissueasset <ASSET_NAME> <NUMBER_OF_UNIT> <UNIT_OF_MEASUREMENT> <NUM_DECIMAL>\n");
    printf("\t\tCreate an asset via Qx contract.\n");
    printf("\t-qxtransferasset <ASSET_NAME> <ISSUER_IN_HEX> <NEW_OWNER_IDENTITY> <AMOUNT_OF_SHARE>\n");
    printf("\t\tTransfer an asset via Qx contract.\n");
    printf("\t-qxorder add/remove bid/ask [ISSUER (in qubic format)] [ASSET_NAME] [PRICE] [NUMBER_OF_SHARE]\n");
    printf("\t\tSet order on Qx.\n");
    printf("\t-qxgetorder entity/asset bid/ask [ISSUER/ENTITY (in qubic format)] [ASSET_NAME (NULL for requesting entity)] [OFFSET]\n");
    printf("\t\tGet orders on Qx\n");

    printf("\n[QTRY COMMAND]\n");
    printf("\t-qtrygetfee\n");
    printf("\t\tShow current qtry fee.\n");
    printf("\t-qtryissuebet\n");
    printf("\t\tIssue a bet (prompt mode)\n");
    printf("\t-qtrygetactivebet\n");
    printf("\t\tShow all active bet id.\n");
    printf("\t-qtrygetactivebetbycreator <BET_CREATOR_ID>\n");
    printf("\t\tShow all active bet id of an ID.\n");
    printf("\t-qtrygetbetinfo <BET_ID>\n");
    printf("\t\tGet meta information of a bet\n");
    printf("\t-qtrygetbetdetail <BET_ID> <OPTION_ID>\n");
    printf("\t\tGet a list of IDs that bet on <OPTION_ID> of the bet <BET_ID>\n");
    printf("\t-qtryjoinbet <BET_ID> <NUMBER_OF_BET_SLOT> <AMOUNT_PER_SLOT> <PICKED_OPTION>\n");
    printf("\t\tJoin a bet\n");
    printf("\t-qtrypublishresult <BET_ID> <WIN_OPTION>\n");
    printf("\t\t(Oracle providers only) publish a result for a bet\n");
    printf("\t-qtrycancelbet <BET_ID>\n");
    printf("\t\t(Game operator only) cancel a bet\n");
}

static long long charToNumber(char* a)
{
    long long retVal = 0;
    char *endptr = nullptr;
    retVal = strtoll(a, &endptr, 10);
    return retVal;
}
static uint64_t charToUnsignedNumber(char* a)
{
    uint64_t retVal = 0;
    char *endptr = nullptr;
    retVal = strtoll(a, &endptr, 10);
    return retVal;
}

void readConfigFile(const char* path)
{
    FILE *file = fopen(path, "r");
    if (file == nullptr) {
        LOG("Error opening config file %s\n", path);
        return;
    }
    char line[1000] = {0};
    while (fgets(line, 1000, file))
    {
        std::vector<std::string> v;
        std::stringstream ss(line);
        while (ss.good()) {
            std::string substr;
            getline(ss, substr, '=');
            v.push_back(substr);
        }
        memset(line, 0, 1000);
        if (v[0] == "node_ip"){
            if ( strcmp(g_nodeIp, DEFAULT_NODE_IP) == 0 ){ // override when node ip is default value
                g_nodeIp = (char*) malloc(64);
                memset(g_nodeIp, 0, 64);
                memcpy(g_nodeIp, v[1].c_str(), v[1].size());
                if (g_nodeIp[v[1].size()-1] == '\n') g_nodeIp[v[1].size()-1] =0;
            }
        }
        if (v[0] == "seed"){
            if ( strcmp(g_seed, DEFAULT_SEED) == 0 ){ // override when seed is default value
                g_seed = (char*) malloc(55);
                memset(g_seed, 0, 55);
                memcpy(g_seed, v[1].c_str(), 55);
            }
        }
        if (v[0] == "node_port"){
            if ( g_nodePort == DEFAULT_NODE_PORT ){ // override when node port is default value
                g_nodePort = std::atoi(v[1].c_str());
            }
        }
        if (v[0] == "schedule_tick_offset"){
            if ( g_offsetScheduledTick == DEFAULT_SCHEDULED_TICK_OFFSET ){ // override when node port is default value
                g_offsetScheduledTick = std::atoi(v[1].c_str());
            }
        }
    }
    fclose(file);
}


void parseArgument(int argc, char** argv){
    //./qubic-cli [basic config] [Command] [command extra parameters]
    // basic config:
    // -conf , -seed, -nodeip, -nodeport, -scheduletick
    // command:
    // -showkeys, -getcurrenttick, -gettickdata, -checktxontick, -checktxontickfile, -readtickdata, -getbalance, -getasset, -sendtoaddress, -sendcustomtransaction, -sendspecialcommand, -sendrawpacket, -publishproposal
    int i = 1;
    g_cmd = TOTAL_COMMAND;
    while (i < argc)
    {
        /**********************
         ******BASIC CONFIG****
         **********************/
        if(strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0) {print_help(); exit(0);}
        if(strcmp(argv[i], "-conf") == 0)
        {
            g_configFile = argv[i+1];
            i+=2;
            continue;
        }
        if(strcmp(argv[i], "-seed") == 0)
        {
            g_seed = argv[i+1];
            i+=2;
            continue;
        }
        if(strcmp(argv[i], "-nodeip") == 0)
        {
            g_nodeIp = argv[i+1];
            i+=2;
            continue;
        }
        if(strcmp(argv[i], "-nodeport") == 0)
        {
            g_nodePort = int(charToNumber(argv[i+1]));
            i+=2;
            continue;
        }
        if(strcmp(argv[i], "-scheduletick") == 0)
        {
            g_offsetScheduledTick = int(charToNumber(argv[i+1]));
            i+=2;
            continue;
        }

        if(strcmp(argv[i], "-waituntilfinish") == 0)
        {
            g_waitUntilFinish = int(charToNumber(argv[i+1]));
            i+=2;
            continue;
        }

         /**********************
         ****WALLET COMMAND****
         **********************/

        if(strcmp(argv[i], "-showkeys") == 0)
        {
            g_cmd = SHOW_KEYS;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-getbalance") == 0)
        {
            g_cmd = GET_BALANCE;
            g_requestedIdentity = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-getasset") == 0)
        {
            g_cmd = GET_ASSET;
            g_requestedIdentity = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-sendtoaddress") == 0)
        {
            g_cmd = SEND_COIN;
            g_targetIdentity = argv[i+1];
            g_TxAmount = charToNumber(argv[i+2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }

        /********************************************
         *********BLOCKCHAIN/PROTOCOL COMMAND********
         ********************************************/
        if(strcmp(argv[i], "-gettickdata") == 0)
        {
            g_cmd = GET_TICK_DATA;
            g_requestedTickNumber = charToNumber(argv[i+1]);
            g_requestedFileName = argv[i + 2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-getquorumtick") == 0)
        {
            g_cmd = GET_QUORUM_TICK;
            g_requestedFileName = argv[i + 1];
            g_requestedTickNumber = charToNumber(argv[i+2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-getcomputorlist") == 0)
        {
            g_cmd = GET_COMP_LIST;
            g_requestedFileName = argv[i + 1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-getnodeiplist") == 0)
        {
            g_cmd = GET_NODE_IP_LIST;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-checktxontick") == 0)
        {
            g_cmd = CHECK_TX_ON_TICK;
            g_requestedTickNumber = charToNumber(argv[i+1]);
            g_requestedTxId = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-checktxonfile") == 0)
        {
            g_cmd = CHECK_TX_ON_FILE;
            g_requestedTxId = argv[i+1];
            g_requestedFileName = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-readtickdata") == 0)
        {
            g_cmd = READ_TICK_DATA;
            g_requestedFileName = argv[i+1];
            g_requestedFileName2 = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-sendcustomtransaction") == 0)
        {
            g_cmd = SEND_CUSTOM_TX;
            g_targetIdentity = argv[i+1];
            g_TxType = charToNumber(argv[i+2]);
            g_TxAmount = charToNumber(argv[i+3]);
            g_txExtraDataSize = int(charToNumber(argv[i+4]));
            hexToByte(argv[i+5], g_txExtraData, g_txExtraDataSize);
            i+=6;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-dumpspectrumfile") == 0)
        {
            g_cmd = DUMP_SPECTRUM_FILE;
            g_dump_binary_file_input = argv[i+1];
            g_dump_binary_file_output = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-dumpuniversefile") == 0)
        {
            g_cmd = DUMP_UNIVERSE_FILE;
            g_dump_binary_file_input = argv[i+1];
            g_dump_binary_file_output = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-makeipobid") == 0)
        {
            g_cmd = MAKE_IPO_BID;
            g_ipo_contract_index = charToNumber(argv[i + 1]);
            g_make_ipo_bid_number_of_share = charToNumber(argv[i+2]);
            g_make_ipo_bid_price_per_share = charToNumber(argv[i+3]);
            i+=4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-getipostatus") == 0)
        {
            g_cmd = GET_IPO_STATUS;
            g_ipo_contract_index = charToNumber(argv[i + 1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-publishproposal") == 0)
        {
            LOG("On development\n");
            exit(0);
        }

        /**********************
         *****NODE COMMAND*****
         **********************/

        if(strcmp(argv[i], "-getsysteminfo") == 0)
        {
            g_cmd = GET_SYSTEM_INFO;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-getcurrenttick") == 0)
        {
            g_cmd = GET_CURRENT_TICK;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-sendspecialcommand") == 0)
        {
            g_cmd = SEND_SPECIAL_COMMAND;
            g_requestedSpecialCommand = int(charToNumber(argv[i+1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-tooglemainaux") == 0)
        {
            g_cmd = TOOGLE_MAIN_AUX;
            g_requestedSpecialCommand = SPECIAL_COMMAND_TOGGLE_MAIN_MODE_REQUEST;
            g_toogle_main_aux_0 = argv[i+1];
            g_toogle_main_aux_1 = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-setsolutionthreshold") == 0)
        {
            g_cmd = SET_SOLUTION_THRESHOLD;
            g_requestedSpecialCommand = SPECIAL_COMMAND_SET_SOLUTION_THRESHOLD_REQUEST;
            g_set_solution_threshold_epoch = charToNumber(argv[i+1]);
            g_set_solution_threshold_value = charToNumber(argv[i+2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-refreshpeerlist") == 0)
        {
            g_cmd = REFRESH_PEER_LIST;
            g_requestedSpecialCommand = SPECIAL_COMMAND_REFRESH_PEER_LIST;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-forcenexttick") == 0)
        {
            g_cmd = FORCE_NEXT_TICK;
            g_requestedSpecialCommand = SPECIAL_COMMAND_FORCE_NEXT_TICK;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-reissuevote") == 0)
        {
            g_cmd = REISSUE_VOTE;
            g_requestedSpecialCommand = SPECIAL_COMMAND_REISSUE_VOTE;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-synctime") == 0)
        {
            g_cmd = SYNC_TIME;
            g_requestedSpecialCommand = SPECIAL_COMMAND_SEND_TIME;
            i += 1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-sendrawpacket") == 0)
        {
            g_cmd = SEND_RAW_PACKET;
            g_rawPacketSize = int(charToNumber(argv[i+1]));
            hexToByte(argv[i+2], g_rawPacket, g_rawPacketSize);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-getlogfromnode") == 0)
        {
            g_cmd = GET_LOG_FROM_NODE;
            g_get_log_passcode[0] = charToUnsignedNumber(argv[i+1]);
            g_get_log_passcode[1] = charToUnsignedNumber(argv[i+2]);
            g_get_log_passcode[2] = charToUnsignedNumber(argv[i+3]);
            g_get_log_passcode[3] = charToUnsignedNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-getminingscoreranking") == 0)
        {
            g_cmd = GET_MINING_SCORE_RANKING;
            g_requestedSpecialCommand = SPECIAL_COMMAND_GET_MINING_SCORE_RANKING;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }

        /**********************
         ******QX COMMAND******
         **********************/

        if(strcmp(argv[i], "-qxissueasset") == 0)
        {
            g_cmd = QX_ISSUE_ASSET;
            g_qx_issue_asset_name = argv[i+1];
            g_qx_issue_asset_number_of_unit = charToNumber(argv[i+2]);
            g_qx_issue_unit_of_measurement = argv[i+3];
            g_qx_issue_asset_num_decimal = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qxtransferasset") == 0)
        {
            g_cmd = QX_TRANSFER_ASSET;
            g_qx_asset_transfer_asset_name = argv[i+1];
            g_qx_asset_transfer_issuer_in_hex = argv[i+2];
            g_qx_asset_transfer_new_owner_identity = argv[i+3];
            g_qx_asset_transfer_amount = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-qxgetfee") == 0)
        {
            g_cmd = PRINT_QX_FEE;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }

        if(strcmp(argv[i], "-qxorder") == 0)
        {
            g_cmd = QX_ORDER;
            g_qx_command_1 = argv[i+1];
            g_qx_command_2 = argv[i+2];
            g_qx_issuer = argv[i+3];
            g_qx_asset_name = argv[i+4];
            g_qx_price = charToNumber(argv[i+5]);
            g_qx_number_of_share = charToNumber(argv[i+6]);
            i+=7;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qxgetorder") == 0)
        {
            g_cmd = QX_GET_ORDER;
            g_qx_command_1 = argv[i+1];
            g_qx_command_2 = argv[i+2];
            g_qx_issuer = argv[i+3];
            g_qx_asset_name = argv[i+4];
            g_qx_offset = charToNumber(argv[i+5]);
            i+=6;
            CHECK_OVER_PARAMETERS
            break;
        }

        /**********************
         ******QTRY COMMAND****
         **********************/

        if(strcmp(argv[i], "-qtryissuebet") == 0)
        {
            g_cmd = QUOTTERY_ISSUE_BET;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qtryjoinbet") == 0)
        {
            g_cmd = QUOTTERY_JOIN_BET;
            g_quottery_bet_id = charToNumber(argv[i + 1]);
            g_quottery_number_bet_slot = charToNumber(argv[i+2]);
            g_quottery_amount_per_bet_slot = charToNumber(argv[i+3]);
            g_quottery_picked_option = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qtrygetbetinfo") == 0)
        {
            g_cmd = QUOTTERY_GET_BET_INFO;
            g_quottery_bet_id = charToNumber(argv[i + 1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qtrygetbetdetail") == 0)
        {
            g_cmd = QUOTTERY_GET_BET_DETAIL;
            g_quottery_bet_id = charToNumber(argv[i + 1]);
            g_quottery_option_id = charToNumber(argv[i + 2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qtrygetactivebet") == 0)
        {
            g_cmd = QUOTTERY_GET_ACTIVE_BET;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qtrygetactivebetbycreator") == 0)
        {
            g_cmd = QUOTTERY_GET_ACTIVE_BET_BY_CREATOR;
            g_quottery_creator_id = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qtrygetfee") == 0)
        {
            g_cmd = QUOTTERY_GET_BET_FEE;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qtrypublishresult") == 0)
        {
            g_cmd = QUOTTERY_PUBLISH_RESULT;
            g_quottery_bet_id = charToNumber(argv[i + 1]);
            g_quottery_option_id = charToNumber(argv[i + 2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qtrycancelbet") == 0)
        {
            g_cmd = QUOTTERY_CANCEL_BET;
            g_quottery_bet_id = charToNumber(argv[i + 1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-qutilsendtomanyv1") == 0)
        {
            g_cmd = QUTIL_SEND_TO_MANY_V1;
            g_qutil_sendtomanyv1_payout_list_file = argv[i + 1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }

        i++;
    }
    if (g_configFile != nullptr)
    {
        readConfigFile(g_configFile);
    }
}
