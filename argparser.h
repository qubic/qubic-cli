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
    printf("\t-showkeys\n");
    printf("\t\tGenerating identity, pubkey key from private key. Private key must be passed either from params or configuration file.\n");
    printf("\t-getcurrenttick\n");
    printf("\t\tShow current tick information of a node\n");
    printf("\t-gettickdata <TICK_NUMBER> <OUTPUT_FILE_NAME>\n");
    printf("\t\tGet tick data and write it to a file. Use -readtickdata to examine the file. valid node ip/port are required.\n");
    printf("\t-getcomputorlist <OUTPUT_FILE_NAME>\n");
    printf("\t\tGet of the current epoch. Feed this data to -readtickdata to verify tick data. valid node ip/port are required.\n");
    printf("\t-checktxontick <TICK_NUMBER> <TX_ID>\n");
    printf("\t\tCheck if a transaction is included in a tick. valid node ip/port are required.\n");
    printf("\t-checktxonfile <TX_ID> <TICK_DATA_FILE>\n");
    printf("\t\tCheck if a transaction is included in a tick (tick data from a file). valid node ip/port are required.\n");
    printf("\t-readtickdata <FILE_NAME> <COMPUTOR_LIST>\n");
    printf("\t\tRead tick data from a file, print the output on screen, COMPUTOR_LIST is required if you need to verify block data\n");
    printf("\t-getbalance <IDENTITY>\n");
    printf("\t\tBalance of an identity (amount of qubic, number of in/out txs)\n");
    printf("\t-getownedasset <IDENTITY>\n");
    printf("\t\tPrint OWNED asset of an identity\n");
    printf("\t-sendtoaddress <TARGET_IDENTITY> <AMOUNT>\n");
    printf("\t\tPerform a normal transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY>. valid private key and node ip/port are required.\n");
    printf("\t-sendcustomtransaction <TARGET_IDENTITY> <TX_TYPE> <AMOUNT> <EXTRA_BYTE_SIZE> <EXTRA_BYTE_IN_HEX>\n");
    printf("\t\tPerform a custom transaction (IPO, querying smart contract), valid private key and node ip/port are required.\n");
    printf("\t-sendspecialcommand <COMMAND_IN_NUMBER> \n");
    printf("\t\tPerform a special command to node, valid private key and node ip/port are required.\t\n");
    printf("\t-sendrawpacket <DATA_IN_HEX> <SIZE>\n");
    printf("\t\tSend a raw packet to nodeip. Valid node ip/port are required.\n");
    printf("\t-publishproposal \n");
    printf("\t\t(on development)\n");
    printf("\t-qxtransfershare <POSSESSED_IDENTITY> <NEW_OWNER_IDENTITY> <AMOUNT_OF_SHARE>\n");
    printf("\t\tTransfer Qx's shares to new owner. valid private key and node ip/port, POSSESSED_IDENTITY are required.\n");
    printf("\t\t(if you set -scheduletick larger than 50000, the tool will be forced to send the tx at that tick)\n");
}

static long long charToNumber(char* a)
{
    long long retVal = 0;
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
    // -showkeys, -getcurrenttick, -gettickdata, -checktxontick, -checktxontickfile, -readtickdata, -getbalance, -getownedasset, -sendtoaddress, -sendcustomtransaction, -sendspecialcommand, -sendrawpacket, -publishproposal, -qxtransfershare
    int i = 1;
    g_cmd = TOTAL_COMMAND;
    while (i < argc)
    {
        if(strcmp(argv[i], "-help") == 0) {print_help(); exit(0);}
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
        if(strcmp(argv[i], "-showkeys") == 0)
        {
            g_cmd = SHOW_KEYS;
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
        if(strcmp(argv[i], "-gettickdata") == 0)
        {
            g_cmd = GET_TICK_DATA;
            g_requestedTickNumber = charToNumber(argv[i+1]);
            g_requestedFileName = argv[i + 2];
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
        if(strcmp(argv[i], "-getbalance") == 0)
        {
            g_cmd = GET_BALANCE;
            g_requestedIdentity = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if(strcmp(argv[i], "-getownedasset") == 0)
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
        if(strcmp(argv[i], "-sendspecialcommand") == 0)
        {
            g_cmd = SEND_SPECIAL_COMMAND;
            g_requestedSpecialCommand = int(charToNumber(argv[i+1]));
            i+=2;
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
        if(strcmp(argv[i], "-publishproposal") == 0)
        {
            LOG("On development\n");
            exit(0);
        }
        if(strcmp(argv[i], "-qxtransfershare") == 0)
        {
            g_cmd = QX_TRANSFER_ASSET;
            g_qx_share_transfer_possessed_identity = argv[i+1];
            g_qx_share_transfer_new_owner_identity = argv[i+2];
            g_qx_share_transfer_amount = charToNumber(argv[i+3]);
            i+=4;
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