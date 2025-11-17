#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sstream>

#include "logger.h"

#define CHECK_OVER_PARAMETERS                                                           \
    if (i < argc)                                                                       \
    {                                                                                   \
        LOG("Not accept any parameters after main COMMAND, unexpected %s\n", argv[i]);  \
        exit(1);                                                                        \
    }

#define CHECK_NUMBER_OF_PARAMETERS(numParams)                                           \
    if (i + numParams >= argc)                                                          \
    {                                                                                   \
        LOG("Not enough parameters provided for command, expected %d.\nRun qubic-cli -h to display help.\n", numParams);    \
        exit(1);                                                                        \
    }

void print_help()
{
    printf("./qubic-cli [basic config] [command] [command extra parameters]\n");
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
    printf("\t-force\n");
    printf("\t\tDo action although an error has been detected. Currently only implemented for proposals.\n");

    printf("Command:\n");
    printf("[WALLET COMMANDS]\n");
    printf("\t-showkeys\n");
    printf("\t\tGenerating identity, pubkey key from private key. Private key must be passed either from params or configuration file.\n");
    printf("\t-getbalance <IDENTITY>\n");
    printf("\t\tBalance of an identity (amount of qubic, number of in/out txs)\n");
    printf("\t-getasset <IDENTITY>\n");
    printf("\t\tPrint a list of assets of an identity\n");
    printf("\t-queryassets <QUERY_TYPE> <QUERY_STING>\n");
    printf("\t\tQuery and print assets information. Skip arguments to get detailed documentation.\n");
    printf("\t-sendtoaddress <TARGET_IDENTITY> <AMOUNT>\n");
    printf("\t\tPerform a standard transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY>. A valid private key and node ip/port are required.\n");
    printf("\t-sendtoaddressintick <TARGET_IDENTITY> <AMOUNT> <TICK>\n");
    printf("\t\tPerform a standard transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY> in a specific <TICK>. A valid private key and node ip/port are required.\n");
    printf("\t-qutilsendtomanyv1 <FILE>\n");
    printf("\t\tPerforms multiple transaction within in one tick. <FILE> must contain one ID and amount (space seperated) per line. Max 25 transaction. Fees apply! valid private key and node ip/port are required.\n");
    printf("\t-qutilburnqubic <AMOUNT>\n");
    printf("\t\tPerforms burning qubic, valid private key and node ip/port are required.\n");
    printf("\t-qutilsendtomanybenchmark <DESTINATION_COUNT> <NUM_TRANSFERS_EACH>\n");
    printf("\t\tSends <NUM_TRANSFERS_EACH> transfers of 1 qu to <DESTINATION_COUNT> addresses in the spectrum. Max 16.7M transfers total. Valid private key and node ip/port are required.\n");

    printf("\n[BLOCKCHAIN/PROTOCOL COMMANDS]\n");
    printf("\t-gettickdata <TICK_NUMBER> <OUTPUT_FILE_NAME>\n");
    printf("\t\tGet tick data and write it to a file. Use -readtickdata to examine the file. valid node ip/port are required.\n");
    printf("\t-getquorumtick <COMP_LIST_FILE> <TICK_NUMBER>\n");
    printf("\t\tGet quorum tick data, the summary of quorum tick will be printed, <COMP_LIST_FILE> is fetched by command -getcomputorlist. valid node ip/port are required.\n");
    printf("\t-getcomputorlist <OUTPUT_FILE_NAME>\n");
    printf("\t\tGet computor list of the current epoch. Feed this data to -readtickdata to verify tick data. valid node ip/port are required.\n");
    printf("\t-getnodeiplist\n");
    printf("\t\tPrint a list of node ip from a seed node ip. Valid node ip/port are required.\n");
    printf("\t-gettxinfo <TX_ID>\n");
    printf("\t\tGet tx infomation, will print empty if there is no tx or invalid tx. valid node ip/port are required.\n");
    printf("\t-uploadfile <FILE_PATH> [COMPRESS_TOOL]\n");
    printf("\t\tUpload a file to qubic network. valid node ip/port and seed are required. optional COMPRESS_TOOL is used to compress the file (support: zip(Unix), tar(Win, Unix)) \n");
    printf("\t-downloadfile <TX_ID> <FILE_PATH> [DECOMPRESS_TOOL]\n");
    printf("\t\tDownload a file to qubic network. valid node ip/port are required. optional DECOMPRESS_TOOL is used to decompress the file (support: zip(Unix), tar(Win, Unix)) \n");
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
    printf("\t\tDump universe file into csv.\n");
    printf("\t-dumpcontractfile <CONTRACT_BINARY_FILE> <CONTRACT_ID> <OUTPUT_CSV_FILE>\n");
    printf("\t\tDump contract file into csv. Current supported CONTRACT_IDs: 1-QX \n");
    printf("\t-makeipobid <CONTRACT_INDEX> <NUMBER_OF_SHARE> <PRICE_PER_SHARE>\n");
    printf("\t\tParticipating IPO (dutch auction). valid private key and node ip/port, CONTRACT_INDEX are required.\n");
    printf("\t-getipostatus <CONTRACT_INDEX>\n");
    printf("\t\tView IPO status. valid node ip/port, CONTRACT_INDEX are required.\n");
    printf("\t-getsysteminfo\n");
    printf("\t\tView Current System Status. Includes initial tick, random mining seed, epoch info.\n");

    printf("\n[NODE COMMANDS]\n");
    printf("\t-getcurrenttick\n");
    printf("\t\tShow current tick information of a node\n");
    printf("\t-sendspecialcommand <COMMAND_IN_NUMBER> \n");
    printf("\t\tPerform a special command to node, valid private key and node ip/port are required.\t\n");
    printf("\t-togglemainaux <MODE_0> <Mode_1> \n");
    printf("\t\tRemotely toggle Main/Aux mode on node,valid private key and node ip/port are required.\t\n");
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
    printf("\t-synctime\n");
    printf("\t\tSync node time with local time, valid private key and node ip/port are required. Make sure that your local time is synced (with NTP)!\t\n");
    printf("\t-getminingscoreranking\n");
    printf("\t\tGet current mining score ranking. Valid private key and node ip/port are required.\t\n");
    printf("\t-getvotecountertx <COMPUTOR_LIST_FILE> <TICK>\n");
    printf("\t\tGet vote counter transaction of a tick: showing how many votes per ID that this tick leader saw from (<TICK>-675-3) to (<TICK>-3) \t\n");
    printf("\t-setloggingmode <MODE>\n");
    printf("\t\tSet console logging mode: 0 disabled, 1 low computational cost, 2 full logging. Valid private key and node ip/port are required.\t\n");
    printf("\t-compmessage \"<MESSAGE>\"\n");
    printf("\t\tBroadcast a message on Qubic network, the message will be relayed to discord via bot. Node ip/port are required. Seed for a valid comp is required\t\n");
    printf("\n[QX COMMANDS]\n");
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
    printf("\t-qxtransferrights <ASSET_NAME> <ISSUER_ID> <NEW_MANAGING_CONTRACT> <NUMBER_OF_SHARES>\n");
    printf("\t\tTransfer asset management rights of shares from QX to another contract. <NEW_MANAGING_CONTRACT> can be given as name or index. You need to own/possess the shares to do this (seed required).\n");

    printf("\n[QTRY COMMANDS]\n");
    printf("\t-qtrygetbasicinfo\n");
    printf("\t\tShow qtry basic info from a node.\n");
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

    printf("\n[GENERAL QUORUM PROPOSAL COMMANDS]\n");
    printf("\t-gqmpropsetproposal <PROPOSAL_STRING>\n");
    printf("\t\tSet proposal in general quorum proposals contract. May overwrite existing proposal, because each computor can have only one proposal at a time. For success, computor status is needed.\n");
    printf("\t\t<PROPOSAL_STRING> is explained if there is a parsing error.\n");
    printf("\t-gqmpropclearproposal\n");
    printf("\t\tClear own proposal in general quorum proposals contract. For success, computor status is needed.\n");
    printf("\t-gqmpropgetproposals <PROPOSAL_INDEX_OR_GROUP>\n");
    printf("\t\tGet proposal info from general quorum proposals contract.\n");
    printf("\t\tEither pass \"active\" to get proposals that are open for voting in the current epoch, or \"finished\" to get proposals of previous epochs not overwritten or cleared yet, or a proposal index.\n");
    printf("\t-gqmpropvote <PROPOSAL_INDEX> <VOTE_VALUE>\n");
    printf("\t\tVote for proposal in general quorum proposals contract.\n");
    printf("\t\t<VOTE_VALUE> is the option in range 0 ... N-1 or \"none\".\n");
    printf("\t-gqmpropgetvote <PROPOSAL_INDEX> [VOTER_IDENTITY]\n");
    printf("\t\tGet vote from general quorum proposals contract. If VOTER_IDENTITY is skipped, identity of seed is used.\n");
    printf("\t-gqmpropgetresults <PROPOSAL_INDEX>\n");
    printf("\t\tGet the current result of a proposal (general quorum proposals contract).\n");
    printf("\t-gqmpropgetrevdonation\n");
    printf("\t\tGet and print table of revenue donations applied after each epoch.\n");

    printf("\n[CCF COMMANDS]\n");
    printf("\t-ccfsetproposal <PROPOSAL_STRING>\n");
    printf("\t\tSet proposal in computor controlled fund (CCF) contract. May overwrite existing proposal, because each seed can have only one proposal at a time. Costs a fee.\n");
    printf("\t\t<PROPOSAL_STRING> is explained if there is a parsing error. Only \"Transfer|2\" (yes/no transfer proposals) are allowed in CCF.\n");
    printf("\t-ccfclearproposal\n");
    printf("\t\tClear own proposal in CCF contract. Costs a fee.\n");
    printf("\t-ccfgetproposals <PROPOSAL_INDEX_OR_GROUP>\n");
    printf("\t\tGet proposal info from CCF contract.\n");
    printf("\t\tEither pass \"active\" to get proposals that are open for voting in the current epoch, or \"finished\" to get proposals of previous epochs not overwritten or cleared yet, or a proposal index.\n");
    printf("\t-ccfvote <PROPOSAL_INDEX> <VOTE_VALUE>\n");
    printf("\t\tCast vote for a proposal in the CCF contract.\n");
    printf("\t\t<VOTE_VALUE> is the option in range 0 ... N-1 or \"none\".\n");
    printf("\t-ccfgetvote <PROPOSAL_INDEX> [VOTER_IDENTITY]\n");
    printf("\t\tGet vote from CCF contract. If VOTER_IDENTITY is skipped, identity of seed is used.\n");
    printf("\t-ccfgetresults <PROPOSAL_INDEX>\n");
    printf("\t\tGet the current result of a CCF proposal.\n");
    printf("\t-ccflatesttransfers\n");
    printf("\t\tGet and print latest transfers of CCF granted by quorum.\n");

    printf("\n[QEARN COMMANDS]\n");
    printf("\t-qearnlock <LOCK_AMOUNT>\n");
    printf("\t\tlock the qu to Qearn SC.\n");
    printf("\t-qearnunlock <UNLOCKING_AMOUNT> <LOCKED_EPOCH>\n");
    printf("\t\tunlock the qu from Qearn SC, unlock the amount of <UNLOCKING_AMOUNT> that locked in the epoch <LOCKED_EPOCH>.\n");
    printf("\t-qearngetlockinfoperepoch <EPOCH>\n");
    printf("\t\tGet the info(Total locked amount, Total bonus amount) locked in <EPOCH>.\n");
    printf("\t-qearngetuserlockedinfo <IDENTITY> <EPOCH>\n");
    printf("\t\tGet the locked amount that the user <IDENTITY> locked in the epoch <EPOCH>.\n");
    printf("\t-qearngetstateofround <EPOCH>\n");
    printf("\t\tGet the status(not started, running, ended) of the epoch <EPOCH>.\n");
    printf("\t-qearngetuserlockstatus <IDENTITY>\n");
    printf("\t\tGet the status(binary number) that the user locked for 52 weeks.\n");
    printf("\t-qearngetunlockingstatus <IDENTITY>\n");
    printf("\t\tGet the unlocking history of the user.\n");
    printf("\t-qearngetstatsperepoch <EPOCH>\n");
    printf("\t\tGet the Stats(early unlocked amount, early unlocked percent) of the epoch <EPOCH> and Stats(total locked amount, average APY) of QEarn SC.\n");
    printf("\t-qearngetburnedandboostedstats\n");
    printf("\t\tGet the Stats(burned amount and average percent, boosted amount and average percent, rewarded amount and average percent in QEarn SC) of QEarn SC\n");
    printf("\t-qearngetburnedandboostedstatsperepoch <EPOCH>\n");
    printf("\t\tGet the Stats(burned amount and percent, boosted amount and percent, rewarded amount and percent in epoch <EPOCH>) of QEarn SC\n");

    printf("\n[QVAULT COMMANDS]\n");
    printf("\t-qvaultstake <AMOUNT>\n");
    printf("\t\tStake the Qcap with <AMOUNT> amount\n");
    printf("\t-qvaultunstake <AMOUNT>\n");
    printf("\t\tUnstake the Qcap with <AMOUNT> amount\n");
    printf("\t-qvaultsubmitgeneralproposal <URL>\n");
    printf("\t\tSubmit the general proposal. <URL> can be a website or github url. the max number of letters is 255\n");
    printf("\t-qvaultsubmitquorumchangeproposal <URL> <NEW_QUORUM_PERMIllE>\n");
    printf("\t\tSubmit the quorum change proposal with new permille for quorum. it should be permille(0 ~ 1000) for sure. not percent\n");
    printf("\t-qvaultsubmitipoproposal <URL> <IPO_CONTRACT_INDEX>\n");
    printf("\t\tSubmit the ipo proposal with <IPO_CONTRACT_INDEX>\n");
    printf("\t-qvaultsubmitqearnproposal <URL> <AMOUNT_OF_QUBIC> <NUMBER_OF_EPOCH>\n");
    printf("\t\tSubmit the qearn proposal with <AMOUNT_OF_QUBIC> <NUMBER_OF_EPOCHES>\n");
    printf("\t\t<AMOUNT_OF_QUBIC> - the amount per epoch, <NUMBER_OF_EPOCHES> - the number of epoches for locking\n");
    printf("\t-qvaultsubmitfundproposal <URL> <PRICE_OF_QCAP> <AMOUNT_OF_QCAP>\n");
    printf("\t\tSubmit the fund proposal with <PRICE_OF_QCAP> <AMOUNT_OF_QCAP>\n");
    printf("\t\t<PRICE_OF_QCAP> - the amount of Qubic for one Qcap, <AMOUNT_OF_QCAP> - the amount of Qcap for sale\n");
    printf("\t-qvaultsubmitmarketplaceproposal <URL> <AMOUNT_OF_QUBIC> <SHARE_NAME> <AMOUNT_OF_QCAP> <SHARE_INDEX> <SHARE_AMOUNT>\n");
	printf("\t\tSubmit the marketplace proposal with <AMOUNT_OF_QUBIC> <SHARE_NAME> <AMOUNT_OF_QCAP> <SHARE_INDEX> <SHARE_AMOUNT>\n");
	printf("\t\t<AMOUNT_OF_QUBIC> - the amount of qubic received from the SC, <SHARE_NAME> - the share name as a number (uint64)\n");
	printf("\t\t<AMOUNT_OF_QCAP> - the amount of qcap received from the SC, <SHARE_INDEX> - the contract index that want to sell the share to the SC, <SHARE_AMOUNT> - the amount of share that want to sell to the SC\n");
	printf("\t-qvaultsubmitpercentallocationproposal <URL> <REINVESTED> <BURN> <DISTRIBUTE>\n");
	printf("\t\tSubmit the allocation proposal with <REINVESTED> <BURN> <DISTRIBUTE>\n");
	printf("\t\t<REINVESTED> - reinvesting permille, <BURN> - Qcap burn permille, <DISTRIBUTE> - distribute permille for Qcap holders. All percentages must sum to 970 (per mille)\n");
	printf("\t-qvaultvoteinproposal <PRICE_OF_IPO> <PROPOSAL_TYPE> <PROPOSAL_ID> <DECISION>\n");
	printf("\t\tVote in the proposal with <PRICE_OF_IPO> <PROPOSAL_TYPE> <PROPOSAL_ID> <DECISION>\n");
	printf("\t\t<PRICE_OF_IPO> - if you want to vote in the ipo proposal, you need to input the exact price for ipo, it should be more than 1B\n");
	printf("\t\t<PROPOSAL_TYPE> - the type of proposal, <PROPOSAL_ID> - the index of proposal, <DECISION> - yes = 1, no = 0\n");
	printf("\t-qvaultbuyqcap <PRICE_OF_QCAP> <AMOUNT_OF_QCAP>\n");
	printf("\t\tBuy the qcap. <AMOUNT_OF_QCAP> - the amount of Qcap that want to buy, <PRICE_OF_QCAP> - the price of Qcap for one Qcap\n");
	printf("\t-qvaulttransfersharemanagementrights <TOKEN_NAME> <TOKEN_ISSUER> <NEWMANAGING_CONTRACT_INDEX> <NUMBER_OF_TOKEN>\n");
	printf("\t\tTransfer the share management right to the <NEWMANAGING_CONTRACT_INDEX>\n");
	printf("\t-qvaultgetdata\n");
	printf("\t\tGetting the state variables from the SC\n");
    printf("\t-qvaultgetstakedamountandvotingpower <IDENTITY>\n");
	printf("\t\tGetting the staked amount and voting power of <IDENTITY> from the SC\n");
	printf("\t-qvaultgetgeneralproposal <PROPOSAL_ID>\n");
	printf("\t\tGetting the general proposal info of <PROPOSAL_ID> proposal\n");
	printf("\t-qvaultgetquorumchangeproposal <PROPOSAL_ID>\n");
	printf("\t\tGetting the quorum change proposal info of <PROPOSAL_ID> proposal\n");
	printf("\t-qvaultgetipoproposal <PROPOSAL_ID>\n");
	printf("\t\tGetting the ipo proposal info of <PROPOSAL_ID> proposal\n");
	printf("\t-qvaultgetqearnproposal <PROPOSAL_ID>\n");
	printf("\t\tGetting the qearn proposal info of <PROPOSAL_ID> proposal\n");
	printf("\t-qvaultgetfundproposal <PROPOSAL_ID>\n");
	printf("\t\tGetting the fund proposal info of <PROPOSAL_ID> proposal\n");
	printf("\t-qvaultgetmarketplaceproposal <PROPOSAL_ID>\n");
	printf("\t\tGetting the marketplace proposal info of <PROPOSAL_ID> proposal\n");
	printf("\t-qvaultgetallocationproposal <PROPOSAL_ID>\n");
	printf("\t\tGetting the allocation proposal info of <PROPOSAL_ID> proposal\n");
	printf("\t-qvaultgetidentitieshavingvotingpower <OFFSET> <COUNT>\n");
	printf("\t\tGetting the identities having the voting power\n");
	printf("\t\t<OFFSET> - the point to read, <COUNT> - the number of fetching\n");
	printf("\t-qvaultgetproposalcreationpower <IDENTITY>\n");
	printf("\t\tChecking if the <IDENTITY> has the proposal creation power\n");
	printf("\t-qvaultgetqcapburntamountinlastepoches <NUMBER_OF_EPOCH>\n");
	printf("\t\tGetting the burnt qcap amount in the last <NUMBER_OF_EPOCH> epoches\n");
	printf("\t-qvaultgetamounttobesoldperyear <YEAR>\n");
	printf("\t\tGetting the amount to be sold per year\n");
    printf("\t-qvaultgettotalrevenueinqcap\n");
	printf("\t\tGetting the total revenue in Qcap\n");
	printf("\t-qvaultgetrevenueinqcapperepoch <EPOCH>\n");
	printf("\t\tGetting the revenue for Qcap in <EPOCH> epoch\n");
	printf("\t-qvaultgetrevenuepershare <CONTRACT_INDEX>\n");
	printf("\t\tGetting the revenue in share <CONTRACT_INDEX>\n");
	printf("\t-qvaultgetamountofshareqvaulthold <ASSET_NAME> <ISSUER>\n");
	printf("\t\tGetting the amount of share the SC hold\n");
	printf("\t-qvaultgetnumberofholderandaverageamount\n");
	printf("\t\tGetting the number of Qcap holder and average amount\n");
	printf("\t-qvaultgetamountforqearninupcomingepoch <EPOCH>\n");
	printf("\t\tGetting the amount that should be locked in Qearn SC in the <EPOCH>\n");
    
    printf("\n[MSVAULT COMMANDS]\n");
    printf("\t-msvaultregistervault <REQUIRED_APPROVALS> <VAULT_NAME> <OWNER_ID_COMMA_SEPARATED>\n");
    printf("\t\tRegister a vault. Vault's number of votes for proposal approval <REQUIRED_APPROVALS>, vault name (max 32 chars), and a list of owners (separated by commas). Fee applies.\n");
    printf("\t-msvaultdeposit <VAULT_ID> <AMOUNT>\n");
    printf("\t\tDeposit qubic into vault given vault ID.\n");
    printf("\t-msvaultreleaseto <VAULT_ID> <AMOUNT> <DESTINATION_IDENTITY>\n");
    printf("\t\tRequest release qu to destination. Fee applies.\n");
    printf("\t-msvaultresetrelease <VAULT_ID>\n");
    printf("\t\tReset release requests. Fee applies.\n");
    printf("\t-msvaultgetvaults <IDENTITY>\n");
    printf("\t\tGet list of vaults owned by IDENTITY.\n");
    printf("\t-msvaultgetreleasestatus <VAULT_ID>\n");
    printf("\t\tGet release status of a vault.\n");
    printf("\t-msvaultgetbalanceof <VAULT_ID>\n");
    printf("\t\tGet balance of a vault.\n");
    printf("\t-msvaultgetvaultname <VAULT_ID>\n");
    printf("\t\tGet vault name.\n");
    printf("\t-msvaultgetrevenueinfo\n");
    printf("\t\tGet MsVault revenue info.\n");
    printf("\t-msvaultgetfees\n");
    printf("\t\tGet MsVault fees.\n");
    printf("\t-msvaultgetvaultowners <VAULT_ID>\n");
    printf("\t\tGet MsVault owners given vault ID.\n");

    printf("\n[TESTING COMMANDS]\n");
    printf("\t-testqpifunctionsoutput\n");
    printf("\t\tTest that output of qpi functions matches TickData and quorum tick votes for 15 ticks in the future (as specified by scheduletick offset). Requires the TESTEXA SC to be enabled.\n");
    printf("\t-testqpifunctionsoutputpast\n");
    printf("\t\tTest that output of qpi functions matches TickData and quorum tick votes for the last 15 ticks. Requires the TESTEXA SC to be enabled.\n");
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

static uint32_t getContractIndex(const char* str)
{
#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif
    uint32_t idx = 0;
    if (strcasecmp(str, "QX") == 0)
        idx = 1;
    else if (strcasecmp(str, "QUOTTERY") == 0 || strcasecmp(str, "QTRY") == 0)
        idx = 2;
    else if (strcasecmp(str, "RANDOM") == 0)
        idx = 3;
    else if (strcasecmp(str, "QUTIL") == 0)
        idx = 4;
    else if (strcasecmp(str, "MLM") == 0)
        idx = 5;
    else if (strcasecmp(str, "GQMPROP") == 0)
        idx = 6;
    else if (strcasecmp(str, "SWATCH") == 0)
        idx = 7;
    else if (strcasecmp(str, "CCF") == 0)
        idx = 8;
    else if (strcasecmp(str, "QEARN") == 0)
        idx = 9;
    else if (strcasecmp(str, "QVAULT") == 0)
        idx = 10;
    else
    {
        constexpr uint32_t contractCount = 11;
        if (sscanf(str, "%u", &idx) != 1 || idx == 0 || idx >= contractCount)
        {
            LOG("Contract \"%s\" is unknown!\n", str);
            exit(1);
        }
    }
    return idx;
#ifdef _MSC_VER
#undef strcasecmp
#endif
}

void readConfigFile(const char* path)
{
    FILE *file = fopen(path, "r");
    if (file == nullptr)
    {
        LOG("Error opening config file %s\n", path);
        return;
    }
    char line[1000] = {0};
    while (fgets(line, 1000, file))
    {
        std::vector<std::string> v;
        std::stringstream ss(line);
        while (ss.good())
        {
            std::string substr;
            getline(ss, substr, '=');
            v.push_back(substr);
        }
        memset(line, 0, 1000);
        if (v[0] == "node_ip")
        {
            if ( strcmp(g_nodeIp, DEFAULT_NODE_IP) == 0 )
            { // override when node ip is default value
                g_nodeIp = (char*) malloc(64);
                memset(g_nodeIp, 0, 64);
                memcpy(g_nodeIp, v[1].c_str(), v[1].size());
                if (g_nodeIp[v[1].size()-1] == '\n') g_nodeIp[v[1].size()-1] = 0;
            }
        }
        if (v[0] == "seed")
        {
            if (strcmp(g_seed, DEFAULT_SEED) == 0)
            {
                // override when seed is default value
                g_seed = (char*) malloc(55);
                memset(g_seed, 0, 55);
                memcpy(g_seed, v[1].c_str(), 55);
            }
        }
        if (v[0] == "node_port")
        {
            if (g_nodePort == DEFAULT_NODE_PORT)
            {
                // override when node port is default value
                g_nodePort = std::atoi(v[1].c_str());
            }
        }
        if (v[0] == "schedule_tick_offset")
        {
            if (g_offsetScheduledTick == DEFAULT_SCHEDULED_TICK_OFFSET)
            {
                // override when node port is default value
                g_offsetScheduledTick = std::atoi(v[1].c_str());
            }
        }
    }
    fclose(file);
}

void parseArgument(int argc, char** argv)
{
    //./qubic-cli [basic config] [Command] [command extra parameters]
    // basic config:
    // -conf , -seed, -nodeip, -nodeport, -scheduletick
    // command:
    // -showkeys, -getcurrenttick, -gettickdata, -checktxontick, -checktxontickfile, -readtickdata, -getbalance, -getasset, -sendtoaddress, -sendcustomtransaction, -sendspecialcommand, -sendrawpacket, -publishproposal
    int i = 1;
    g_cmd = TOTAL_COMMAND;
    while (i < argc)
    {
        /************************
         ***** BASIC CONFIG *****
         ************************/

        if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0) { print_help(); exit(0); }
        if (strcmp(argv[i], "-conf") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_configFile = argv[i+1];
            i+=2;
            continue;
        }
        if (strcmp(argv[i], "-seed") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_seed = argv[i+1];
            i+=2;
            continue;
        }
        if (strcmp(argv[i], "-nodeip") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_nodeIp = argv[i+1];
            i+=2;
            continue;
        }
        if (strcmp(argv[i], "-nodeport") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_nodePort = int(charToNumber(argv[i+1]));
            i+=2;
            continue;
        }
        if (strcmp(argv[i], "-scheduletick") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_offsetScheduledTick = int(charToNumber(argv[i+1]));
            i+=2;
            continue;
        }
        if (strcmp(argv[i], "-waituntilfinish") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_waitUntilFinish = int(charToNumber(argv[i+1]));
            i+=2;
            continue;
        }

        /***************************
         ***** WALLET COMMANDS *****
         ***************************/

        if (strcmp(argv[i], "-showkeys") == 0)
        {
            g_cmd = SHOW_KEYS;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getbalance") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = GET_BALANCE;
            g_requestedIdentity = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = GET_ASSET;
            g_requestedIdentity = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-queryassets") == 0)
        {
            g_cmd = QUERY_ASSETS;
            if (i + 1 < argc)
                g_paramString1 = argv[i + 1];
            if (i + 2 < argc)
                g_paramString2 = argv[i + 2];
            i += 3;
            CHECK_OVER_PARAMETERS
             break;
        }
        if (strcmp(argv[i], "-sendtoaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = SEND_COIN;
            g_targetIdentity = argv[i+1];
            g_TxAmount = charToNumber(argv[i+2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-sendtoaddressintick") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = SEND_COIN_IN_TICK;
            g_targetIdentity = argv[i+1];
            g_TxAmount = uint32_t(charToNumber(argv[i+2]));
            g_TxTick = uint32_t(charToNumber(argv[i+3]));
            i+=4;
            CHECK_OVER_PARAMETERS
            break;
        }

        /****************************************
         ***** BLOCKCHAIN/PROTOCOL COMMANDS *****
         ****************************************/

        if (strcmp(argv[i], "-gettickdata") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = GET_TICK_DATA;
            g_requestedTickNumber = uint32_t(charToNumber(argv[i+1]));
            g_requestedFileName = argv[i + 2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getquorumtick") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = GET_QUORUM_TICK;
            g_requestedFileName = argv[i + 1];
            g_requestedTickNumber = uint32_t(charToNumber(argv[i+2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getcomputorlist") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = GET_COMP_LIST;
            g_requestedFileName = argv[i + 1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getnodeiplist") == 0)
        {
            g_cmd = GET_NODE_IP_LIST;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-gettxinfo") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = GET_TX_INFO;
            g_requestedTxId = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-uploadfile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = UPLOAD_FILE;
            g_file_path = argv[i+1];
            i+=2;
            if (i < argc)
            {
                g_compress_tool = argv[i];
                i++;
            }
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-downloadfile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = DOWNLOAD_FILE;
            g_requestedTxId = argv[i+1];
            g_file_path = argv[i+2];
            i+=3;
            if (i < argc)
            {
                g_compress_tool = argv[i];
                i++;
            }
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-checktxontick") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = CHECK_TX_ON_TICK;
            g_requestedTickNumber = uint32_t(charToNumber(argv[i+1]));
            g_requestedTxId = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-checktxonfile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = CHECK_TX_ON_FILE;
            g_requestedTxId = argv[i+1];
            g_requestedFileName = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-readtickdata") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = READ_TICK_DATA;
            g_requestedFileName = argv[i+1];
            g_requestedFileName2 = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getvotecountertx") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = GET_VOTE_COUNTER_TX;
            g_requestedFileName = argv[i+1];
            g_requestedTickNumber = uint32_t(charToNumber(argv[i+2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-sendcustomtransaction") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(5)
            g_cmd = SEND_CUSTOM_TX;
            g_targetIdentity = argv[i+1];
            g_TxType = uint16_t(charToNumber(argv[i+2]));
            g_TxAmount = charToNumber(argv[i+3]);
            g_txExtraDataSize = int(charToNumber(argv[i+4]));
            hexToByte(argv[i+5], g_txExtraData, g_txExtraDataSize);
            i+=6;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-dumpspectrumfile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = DUMP_SPECTRUM_FILE;
            g_dump_binary_file_input = argv[i+1];
            g_dump_binary_file_output = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-dumpuniversefile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = DUMP_UNIVERSE_FILE;
            g_dump_binary_file_input = argv[i+1];
            g_dump_binary_file_output = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-dumpcontractfile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = DUMP_CONTRACT_FILE;
            g_dump_binary_file_input = argv[i+1];
            g_dump_binary_contract_id = uint32_t(charToNumber(argv[i+2]));
            g_dump_binary_file_output = argv[i+3];
            i+=4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-makeipobid") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = MAKE_IPO_BID;
            g_ipo_contract_index = uint32_t(charToNumber(argv[i + 1]));
            g_make_ipo_bid_number_of_share = uint16_t(charToNumber(argv[i+2]));
            g_make_ipo_bid_price_per_share = charToNumber(argv[i+3]);
            i+=4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getipostatus") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = GET_IPO_STATUS;
            g_ipo_contract_index = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }

        /*************************
         ***** NODE COMMANDS *****
         *************************/

        if (strcmp(argv[i], "-getsysteminfo") == 0)
        {
            g_cmd = GET_SYSTEM_INFO;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getcurrenttick") == 0)
        {
            g_cmd = GET_CURRENT_TICK;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-sendspecialcommand") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = SEND_SPECIAL_COMMAND;
            g_requestedSpecialCommand = int(charToNumber(argv[i+1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-togglemainaux") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = TOOGLE_MAIN_AUX;
            g_toggle_main_aux_0 = argv[i+1];
            g_toggle_main_aux_1 = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-setsolutionthreshold") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = SET_SOLUTION_THRESHOLD;
            g_set_solution_threshold_epoch = int(charToNumber(argv[i+1]));
            g_set_solution_threshold_value = int(charToNumber(argv[i+2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-refreshpeerlist") == 0)
        {
            g_cmd = REFRESH_PEER_LIST;
            g_requestedSpecialCommand = SPECIAL_COMMAND_REFRESH_PEER_LIST;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-forcenexttick") == 0)
        {
            g_cmd = FORCE_NEXT_TICK;
            g_requestedSpecialCommand = SPECIAL_COMMAND_FORCE_NEXT_TICK;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-reissuevote") == 0)
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
        if (strcmp(argv[i], "-sendrawpacket") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = SEND_RAW_PACKET;
            g_rawPacketSize = int(charToNumber(argv[i+1]));
            hexToByte(argv[i+2], g_rawPacket, g_rawPacketSize);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getminingscoreranking") == 0)
        {
            g_cmd = GET_MINING_SCORE_RANKING;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-setloggingmode") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = SET_LOGGING_MODE;
            g_loggingMode = char(charToNumber(argv[i+1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-compmessage") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = COMP_CHAT;
            g_compChatString = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }

        /***********************
         ***** QX COMMANDS *****
         ***********************/

        if (strcmp(argv[i], "-qxissueasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QX_ISSUE_ASSET;
            g_qx_issue_asset_name = argv[i+1];
            g_qx_issue_asset_number_of_unit = charToNumber(argv[i+2]);
            g_qx_issue_unit_of_measurement = argv[i+3];
            g_qx_issue_asset_num_decimal = char(charToNumber(argv[i+4]));
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qxtransferasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QX_TRANSFER_ASSET;
            g_qx_asset_transfer_asset_name = argv[i+1];
            g_qx_asset_transfer_issuer_in_hex = argv[i+2];
            g_qx_asset_transfer_new_owner_identity = argv[i+3];
            g_qx_asset_transfer_amount = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qxgetfee") == 0)
        {
            g_cmd = PRINT_QX_FEE;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qxorder") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(6)
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
        if (strcmp(argv[i], "-qxgetorder") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(5)
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
        if (strcmp(argv[i], "-qxtransferrights") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QX_TRANSFER_MANAGEMENT_RIGHTS;
            g_qx_asset_name = argv[i + 1];
            g_qx_issuer = argv[i + 2];
            g_contract_index = getContractIndex(argv[i + 3]);
            g_qx_number_of_share = charToNumber(argv[i + 4]);
            i += 5;
            CHECK_OVER_PARAMETERS
            break;
        }


        /*************************
         ***** QTRY COMMANDS *****
         *************************/

        if (strcmp(argv[i], "-qtryissuebet") == 0)
        {
            g_cmd = QUOTTERY_ISSUE_BET;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtryjoinbet") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QUOTTERY_JOIN_BET;
            g_quottery_bet_id = uint32_t(charToNumber(argv[i + 1]));
            g_quottery_number_bet_slot = charToNumber(argv[i+2]);
            g_quottery_amount_per_bet_slot = charToNumber(argv[i+3]);
            g_quottery_picked_option = uint32_t(charToNumber(argv[i+4]));
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrygetbetinfo") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUOTTERY_GET_BET_INFO;
            g_quottery_bet_id = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrygetbetdetail") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QUOTTERY_GET_BET_DETAIL;
            g_quottery_bet_id = uint32_t(charToNumber(argv[i + 1]));
            g_quottery_option_id = uint32_t(charToNumber(argv[i + 2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrygetactivebet") == 0)
        {
            g_cmd = QUOTTERY_GET_ACTIVE_BET;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrygetactivebetbycreator") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUOTTERY_GET_ACTIVE_BET_BY_CREATOR;
            g_quottery_creator_id = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrygetbasicinfo") == 0)
        {
            g_cmd = QUOTTERY_GET_BASIC_INFO;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrypublishresult") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QUOTTERY_PUBLISH_RESULT;
            g_quottery_bet_id = uint32_t(charToNumber(argv[i + 1]));
            g_quottery_option_id = uint32_t(charToNumber(argv[i + 2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrycancelbet") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUOTTERY_CANCEL_BET;
            g_quottery_bet_id = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilsendtomanyv1") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_SEND_TO_MANY_V1;
            g_qutil_sendtomanyv1_payout_list_file = argv[i + 1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilburnqubic") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_BURN_QUBIC;
            g_TxAmount = charToNumber(argv[i + 1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilsendtomanybenchmark") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QUTIL_SEND_TO_MANY_BENCHMARK;
            g_qutil_sendtomanybenchmark_destination_count = charToNumber(argv[i + 1]);
            g_qutil_sendtomanybenchmark_num_transfers_each = charToNumber(argv[i + 2]);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }

        /****************************
         ***** GQMPROP COMMANDS *****
         ****************************/

        if (strcmp(argv[i], "-gqmpropsetproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = GQMPROP_SET_PROPOSAL;
            g_proposalString = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-gqmpropclearproposal") == 0)
        {
            g_cmd = GQMPROP_CLEAR_PROPOSAL;
            i += 1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-gqmpropgetproposals") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = GQMPROP_GET_PROPOSALS;
            if (i + 1 >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX_OR_GROUP! E.g.: 0, \"active\", or \"finished\".");
                exit(1);
            }
            g_proposalString = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-gqmpropvote") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = GQMPROP_VOTE;
            if (i + 2 >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX and VOTE_VALUE!");
                exit(1);
            }
            g_proposalString = argv[i + 1];
            g_voteValueString = argv[i + 2];
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-gqmpropgetvote") == 0)
        {
            g_cmd = GQMPROP_GET_VOTE;
            ++i;
            if (i >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX!");
                exit(1);
            }
            g_proposalString = argv[i];
            ++i;
            if (i < argc)
            {
                g_requestedIdentity = argv[i];
                ++i;
            }
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-gqmpropgetresults") == 0)
        {
            g_cmd = GQMPROP_GET_VOTING_RESULTS;
            if (i + 1 >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX!");
                exit(1);
            }
            g_proposalString = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-gqmpropgetrevdonation") == 0)
        {
            g_cmd = GQMPROP_GET_REV_DONATION;
            i += 1;
            CHECK_OVER_PARAMETERS
            break;
        }

        /************************
         ***** CCF COMMANDS *****
         ************************/

        if (strcmp(argv[i], "-ccfsetproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = CCF_SET_PROPOSAL;
            g_proposalString = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-ccfclearproposal") == 0)
        {
            g_cmd = CCF_CLEAR_PROPOSAL;
            i += 1;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-ccfgetproposals") == 0)
        {
            g_cmd = CCF_GET_PROPOSALS;
            if (i + 1 >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX_OR_GROUP! E.g.: 0, \"active\", or \"finished\".");
                exit(1);
            }
            g_proposalString = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-ccfvote") == 0)
        {
            g_cmd = CCF_VOTE;
            if (i + 2 >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX and VOTE_VALUE!");
                exit(1);
            }
            g_proposalString = argv[i + 1];
            g_voteValueString = argv[i + 2];
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-ccfgetvote") == 0)
        {
            g_cmd = CCF_GET_VOTE;
            ++i;
            if (i >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX!");
                exit(1);
            }
            g_proposalString = argv[i];
            ++i;
            if (i < argc)
            {
                g_requestedIdentity = argv[i];
                ++i;
            }
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-ccfgetresults") == 0)
        {
            g_cmd = CCF_GET_VOTING_RESULTS;
            if (i + 1 >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX!");
                exit(1);
            }
            g_proposalString = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-ccflatesttransfers") == 0)
        {
            g_cmd = CCF_GET_LATEST_TRANSFERS;
            i += 1;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "-force") == 0)
        {
            g_force = true;
        }

        /**************************
         ***** QEARN COMMANDS *****
         **************************/

        if (strcmp(argv[i], "-qearnlock") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_LOCK;
            g_qearn_lock_amount = charToNumber(argv[i + 1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearnunlock") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QEARN_UNLOCK;
            g_qearn_unlock_amount = charToNumber(argv[i + 1]);
            g_qearn_locked_epoch = uint32_t(charToNumber(argv[i + 2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetlockinfoperepoch") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_GET_INFO_PER_EPOCH;
            g_qearn_getinfo_epoch = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetuserlockedinfo") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QEARN_GET_USER_LOCKED_INFO;
            g_requestedIdentity = argv[i+1];
            g_qearn_getinfo_epoch = uint32_t(charToNumber(argv[i + 2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetstateofround") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_GET_STATE_OF_ROUND;
            g_qearn_getinfo_epoch = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetuserlockstatus") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_GET_USER_LOCK_STATUS;
            g_requestedIdentity = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetunlockingstatus") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_GET_UNLOCKING_STATUS;
            g_requestedIdentity = argv[i+1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetstatsperepoch") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_GET_STATS_PER_EPOCH;
            g_qearn_getstats_epoch = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetburnedandboostedstats") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(0)
            g_cmd = QEARN_GET_BURNED_AND_BOOSTED_STATS;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetburnedandboostedstatsperepoch") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_GET_BURNED_AND_BOOSTED_STATS_PER_EPOCH;
            g_qearn_getstats_epoch = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }


        /**************************
         ***** QVAULT COMMANDS *****
         **************************/

        if (strcmp(argv[i], "-qvaultstake") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_STAKE;
            g_qvault_stake_amount = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultunstake") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_UNSTAKE;
            g_qvault_stake_amount = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitgeneralproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_SUBMIT_GP;
            g_qvaulturl = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitquorumchangeproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QVAULT_COMMAND_SUBMIT_QCP;
            g_qvaulturl = argv[i + 1];
            g_qvault_permille = uint32_t(charToNumber(argv[i + 2]));
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitipoproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QVAULT_COMMAND_SUBMIT_IPOP;
            g_qvaulturl = argv[i + 1];
            g_qvault_ipo_contract_index = uint32_t(charToNumber(argv[i + 2]));
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitqearnproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QVAULT_COMMAND_SUBMIT_QEARNP;
            g_qvaulturl = argv[i + 1];
            g_qvault_amount_of_qubic = uint64_t(charToNumber(argv[i + 2]));
            g_qvault_number_of_epoch = uint32_t(charToNumber(argv[i + 3]));
            i += 4;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitfundproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QVAULT_COMMAND_SUBMIT_FUNDP;
            g_qvaulturl = argv[i + 1];
            g_qvault_price_of_qcap = uint64_t(charToNumber(argv[i + 2]));
            g_qvault_amount_of_qcap = uint32_t(charToNumber(argv[i + 3]));
            i += 4;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitmarketplaceproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(6)
            g_cmd = QVAULT_COMMAND_SUBMIT_MKTP;
            g_qvaulturl = argv[i + 1];
            g_qvault_amount_of_qubic = uint64_t(charToNumber(argv[i + 2]));
            g_qvault_share_name = argv[i + 3];
            g_qvault_amount_of_qcap = uint32_t(charToNumber(argv[i + 4]));
            g_qvault_index_of_share = uint32_t(charToNumber(argv[i + 5]));
            g_qvault_amount_of_share = uint32_t(charToNumber(argv[i + 6]));
            i += 7;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitpercentallocationproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QVAULT_COMMAND_SUBMIT_ALLOP;
            g_qvaulturl = argv[i + 1];
            g_qvault_reinvested = uint32_t(charToNumber(argv[i + 2]));
            g_qvault_burn = uint32_t(charToNumber(argv[i + 3]));
            g_qvault_distribute = uint32_t(charToNumber(argv[i + 4]));
            i += 5;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultvoteinproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QVAULT_COMMAND_VOTE_IN_PROPOSAL;
            g_qvault_price_of_ipo = uint64_t(charToNumber(argv[i + 1]));
            g_qvault_proposal_type = uint32_t(charToNumber(argv[i + 2]));
            g_qvault_proposal_id = uint32_t(charToNumber(argv[i + 3]));
            g_qvault_yes = bool(charToNumber(argv[i + 4]));
            i += 5;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultbuyqcap") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QVAULT_COMMAND_BUY_QCAP;
            g_qvault_price_of_qcap = uint64_t(charToNumber(argv[i + 1]));
            g_qvault_amount_of_qcap = uint32_t(charToNumber(argv[i + 2]));
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaulttransfersharemanagementrights") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QVAULT_COMMAND_TRANSFER_SHARE_MANAGEMENT_RIGHTS;
            g_qvault_assetname = argv[i + 1];
            g_qvaultIdentity = argv[i + 2];
            g_qvault_newmanagement_contract_index = uint32_t(charToNumber(argv[i + 3]));
            g_qvault_number_of_share = int64_t(charToNumber(argv[i + 4]));
            i += 5;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetdata") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(0)
            g_cmd = QVAULT_COMMAND_GETDATA;
            i += 1;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetstakedamountandvotingpower") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_STAKED_AMOUNT_AND_VOTING_POWER;
            g_qvaultIdentity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetgeneralproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_GP;
            g_qvault_proposal_id = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetquorumchangeproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_QCP;
            g_qvault_proposal_id = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetipoproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_IPOP;
            g_qvault_proposal_id = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetqearnproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_QEARNP;
            g_qvault_proposal_id = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetfundproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_FUNDP;
            g_qvault_proposal_id = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetmarketplaceproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_MKTP;
            g_qvault_proposal_id = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetallocationproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_ALLOP;
            g_qvault_proposal_id = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetidentitieshavingvotingpower") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QVAULT_COMMAND_GET_IDENTITIES_HV_VT_PW;
            g_qvault_offset = uint32_t(charToNumber(argv[i + 1]));
            g_qvault_count = uint32_t(charToNumber(argv[i + 2]));
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetproposalcreationpower") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_PP_CREATION_POWER;
            g_qvaultIdentity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetqcapburntamountinlastepoches") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_QCAP_BURNT_AMOUNT_IN_LAST_EPOCHES;
            g_qvault_number_of_epoch = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetamounttobesoldperyear") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_AMOUNT_TO_BE_SOLD_PER_YEAR;
            g_qvault_year = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgettotalrevenueinqcap") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(0)
            g_cmd = QVAULT_COMMAND_GET_TOTAL_REVENUE_IN_QCAP;
            i += 1;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetrevenueinqcapperepoch") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_REVENUE_IN_QCAP_PER_EPOCH;
            g_qvault_epoch = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetrevenuepershare") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_REVENUE_PER_SHARE;
            g_qvault_contract_index = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetamountofshareqvaulthold") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QVAULT_COMMAND_GET_AMOUNT_OF_SHARE_QVAULT_HOLD;
            g_qvault_assetname = argv[i + 1];
            g_qvaultIdentity = argv[i + 2];
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetnumberofholderandaverageamount") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(0)
            g_cmd = QVAULT_COMMAND_GET_NUMBER_OF_HOLDER_AND_AVG_AM;
            i += 1;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetamountforqearninupcomingepoch") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_COMMAND_GET_AMOUNT_FOR_QEARN_IN_UPCOMING_EPOCH;
            g_qvault_epoch = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        
        /**************************
         **** MSVAULT COMMANDS ****
         **************************/

        if (strcmp(argv[i], "-msvaultregistervault") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = MSVAULT_REGISTER_VAULT_CMD;
            g_msVaultRequiredApprovals = (uint64_t)charToNumber(argv[i + 1]);

            {
                const char* inputVaultName = argv[i + 2];
                size_t len = strlen(inputVaultName);
                if (len > 32) {
                    LOG("Vault name must be at most 32 chars. Truncating...\n");
                    len = 32;
                }
                memset(g_msVaultVaultName, 0, 32);
                memcpy(g_msVaultVaultName, inputVaultName, len);
            }

            g_msVaultOwnersCommaSeparated = argv[i + 3];
            i += 4;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultdeposit") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = MSVAULT_DEPOSIT_CMD;
            g_msVaultID = charToNumber(argv[i+1]);
            g_TxAmount = charToNumber(argv[i+2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultreleaseto") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = MSVAULT_RELEASE_TO_CMD;
            g_msVaultID = charToNumber(argv[i+1]);
            g_TxAmount = charToNumber(argv[i+2]);
            g_msVaultDestination = argv[i + 3];
            i+=4;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultresetrelease") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_RESET_RELEASE_CMD;
            g_msVaultID = charToNumber(argv[i+1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultgetvaults") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_VAULTS_CMD;
            g_msVaultPublicId = argv[i + 1];
            i+=2;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultgetreleasestatus") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_RELEASE_STATUS_CMD;
            g_msVaultID = charToNumber(argv[i+1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultgetbalanceof") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_BALANCE_OF_CMD;
            g_msVaultID = charToNumber(argv[i+1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultgetvaultname") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_VAULT_NAME_CMD;
            g_msVaultID = charToNumber(argv[i+1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultgetrevenueinfo") == 0)
        {
            g_cmd = MSVAULT_GET_REVENUE_INFO_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultgetfees") == 0)
        {
            g_cmd = MSVAULT_GET_FEES_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-msvaultgetvaultowners") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_OWNERS_CMD;
            g_msVaultID = charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            return;
        }

        /**************************
         **** TESTING COMMANDS ****
         **************************/
        
        if (strcmp(argv[i], "-testqpifunctionsoutput") == 0)
        {
            g_cmd = TEST_QPI_FUNCTIONS_OUTPUT;
            i++;
            CHECK_OVER_PARAMETERS
            return;
        }
        if (strcmp(argv[i], "-testqpifunctionsoutputpast") == 0)
        {
            g_cmd = TEST_QPI_FUNCTIONS_OUTPUT_PAST;
            i++;
            CHECK_OVER_PARAMETERS
            return;
        }

        i++;
    }
    if (g_configFile != nullptr)
    {
        readConfigFile(g_configFile);
    }
}
