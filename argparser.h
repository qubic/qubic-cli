#pragma once

#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <sstream>

#include "global.h"
#include "logger.h"
#include "structs.h"

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
    printf("\nBasic config:\n");
    printf("\t-conf <file>\n");
    printf("\t\tSpecify configuration file. Relative paths will be prefixed by datadir location. See qubic.conf.example.\n");
    printf("\t\tNotice: variables in qubic.conf will be overrided by values on parameters.\n");
    printf("\t-seed <SEED>\n");
    printf("\t\t55-char seed for private key\n");
    printf("\t-nodeip <IPv4_ADDRESS>\n");
    printf("\t\tIP address of the target node for querying blockchain information (default: 127.0.0.1)\n");
    printf("\t-nodeport <PORT>\n");
    printf("\t\tPort of the target node for querying blockchain information (default: 21841)\n");
    printf("\t-scheduletick <TICK_OFFSET>\n");
    printf("\t\tOffset number of scheduled tick that will perform a transaction (default: 20)\n");
    printf("\t-force\n");
    printf("\t\tDo action although an error has been detected. Currently only implemented for proposals.\n");
    printf("\t-enabletestcontracts\n");
    printf("\t\tEnable test contract indices and names for commands using a contract index parameter. This flag has to be passed before the contract index/name. The node to connect to needs to have test contracts running.\n");
    printf("\t-print-only <base64 | hex>\n");
    printf("\t\tPrint the raw transaction data without sending it to the network. Useful for offline signing or broadcasting later.\n");

    printf("\nCommands:\n");
    printf("\n[WALLET COMMANDS]\n");
    printf("\t-showkeys\n");
    printf("\t\tGenerate identity, public key and private key from seed. Seed must be passed either from params or configuration file.\n");
    printf("\t-getbalance <IDENTITY>\n");
    printf("\t\tBalance of an identity (amount of qubic, number of in/out txs)\n");
    printf("\t-getasset <IDENTITY>\n");
    printf("\t\tPrint a list of assets of an identity\n");
    printf("\t-queryassets <QUERY_TYPE> <QUERY_STING>\n");
    printf("\t\tQuery and print assets information. Skip arguments to get detailed documentation.\n");
    printf("\t-gettotalnumberofassetshares <ISSUER_ID> <ASSET_NAME>\n");
    printf("\t\tGet total number of shares currently existing of a specific asset.\n");
    printf("\t-sendtoaddress <TARGET_IDENTITY> <AMOUNT>\n");
    printf("\t\tPerform a standard transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY>. A valid seed and node ip/port are required.\n");
    printf("\t-sendtoaddressintick <TARGET_IDENTITY> <AMOUNT> <TICK>\n");
    printf("\t\tPerform a standard transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY> in a specific <TICK>. A valid seed and node ip/port are required.\n");

    printf("\n[QUTIL COMMANDS]\n");
    printf("\t-qutilsendtomanyv1 <FILE>\n");
    printf("\t\tPerforms multiple transaction within in one tick. <FILE> must contain one ID and amount (space seperated) per line. Max 25 transaction. Fees apply! Valid seed and node ip/port are required.\n");
    printf("\t-qutilburnqubic <AMOUNT>\n");
    printf("\t\tPerforms burning qubic, valid seed and node ip/port are required.\n");
    printf("\t-qutilburnqubicforcontract <AMOUNT> <CONTRACT_INDEX>\n");
    printf("\t\tBurns qubic for the specified contract index, valid seed and node ip/port are required.\n");
    printf("\t-qutilqueryfeereserve <CONTRACT_INDEX>\n");
    printf("\t\tQueries the amount of qubic in the fee reserve of the specified contract, valid node ip/port are required.\n");
    printf("\t-qutildistributequbictoshareholders <ISSUER_ID> <ASSET_NAME> <AMOUNT>\n");
    printf("\t\tDistribute QU among shareholders, transferring the same amount of QU for each share. The fee is proportional to the number of shareholders. The remainder that cannot be distributed equally is reimbursed.\n");
    printf("\t-qutilsendtomanybenchmark <DESTINATION_COUNT> <NUM_TRANSFERS_EACH>\n");
    printf("\t\tSends <NUM_TRANSFERS_EACH> transfers of 1 qu to <DESTINATION_COUNT> addresses in the spectrum. Max 16.7M transfers total. Valid seed and node ip/port are required.\n");
    printf("\t-qutilcreatepoll <POLL_NAME> <POLL_TYPE> <MIN_AMOUNT> <GITHUB_LINK> <SEMICOLON_SEPARATED_ASSETS>\n");
    printf("\t\tCreate a new poll. <POLL_NAME> is the poll's name (32 bytes), <POLL_TYPE> is 1 for Qubic or 2 for Asset, <MIN_AMOUNT> is the minimum vote amount, <GITHUB_LINK> is a 256-byte GitHub link. For Asset polls (type 2), provide a semicolon-separated list of assets in the format 'asset_name,issuer;asset_name,issuer'. Valid seed and node ip/port are required.\n");    printf("\t-qutilvote <POLL_ID> <AMOUNT> <CHOSEN_OPTION>\n");
    printf("\t\tVote in a poll. <POLL_ID> is the poll's ID, <AMOUNT> is the vote amount, and <CHOSEN_OPTION> is the selected option (0-63). Valid seed and node ip/port are required.\n");
    printf("\t-qutilgetcurrentresult <POLL_ID>\n");
    printf("\t\tGet the current results of a poll. <POLL_ID> is the poll's ID. Valid node ip/port are required.\n");
    printf("\t-qutilgetpollsbycreator <CREATOR_ADDRESS>\n");
    printf("\t\tGet polls created by a specific user. <CREATOR_ADDRESS> is the creator's identity. Valid node ip/port are required.\n");
    printf("\t-qutilgetcurrentpollid\n");
    printf("\t\tGet the current poll ID and list of active polls.\n");
    printf("\t-qutilgetpollinfo <POLL_ID>\n");
    printf("\t\tGet information about a specific poll by its ID.\n");
    printf("\t-qutilcancelpoll <POLL_ID>\n");
    printf("\t\tCancel a poll by its ID. Only the poll creator can cancel it. Requires seed and node ip/port.\n");
    printf("\t-qutilgetfee\n");
    printf("\t\tShow current QUTIL fees.\n");

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
    printf("\t\tPerform a custom transaction (IPO, querying smart contract), valid seed and node ip/port are required.\n");
    printf("\t-dumpspectrumfile <SPECTRUM_BINARY_FILE> <OUTPUT_CSV_FILE>\n");
    printf("\t\tDump spectrum file into csv.\n");
    printf("\t-dumpuniversefile <UNIVERSE_BINARY_FILE> <OUTPUT_CSV_FILE>\n");
    printf("\t\tDump universe file into csv.\n");
    printf("\t-dumpcontractfile <CONTRACT_BINARY_FILE> <CONTRACT_ID> <OUTPUT_CSV_FILE>\n");
    printf("\t\tDump contract file into csv. Current supported CONTRACT_IDs: 1-QX \n");
    printf("\t-makeipobid <CONTRACT_INDEX> <NUMBER_OF_SHARE> <PRICE_PER_SHARE>\n");
    printf("\t\tParticipating IPO (dutch auction). Valid seed and node ip/port, CONTRACT_INDEX are required.\n");
    printf("\t-getipostatus <CONTRACT_INDEX>\n");
    printf("\t\tView IPO status. valid node ip/port, CONTRACT_INDEX are required.\n");
    printf("\t-getactiveipos\n");
    printf("\t\tView list of active IPOs in this epoch. valid node ip/port are required.\n");
    printf("\t-getsysteminfo\n");
    printf("\t\tView Current System Status. Includes initial tick, random mining seed, epoch info.\n");

    printf("\n[NODE COMMANDS]\n");
    printf("\t-getcurrenttick\n");
    printf("\t\tShow current tick information of a node\n");
    printf("\t-sendspecialcommand <COMMAND_IN_NUMBER> \n");
    printf("\t\tPerform a special command to node, valid seed and node ip/port are required.\t\n");
    printf("\t-togglemainaux <MODE_0> <Mode_1> \n");
    printf("\t\tRemotely toggle Main/Aux mode on node, valid seed and node ip/port are required.\t\n");
    printf("\t\t<MODE_0> and <MODE_1> value are: MAIN or AUX\t\n");
    printf("\t-setsolutionthreshold <EPOCH> <SOLUTION_THRESHOLD> \n");
    printf("\t\tRemotely set solution threshold for future epoch, valid seed and node ip/port are required.\t\n");
    printf("\t-refreshpeerlist\n");
    printf("\t\t(equivalent to F4) Remotely refresh the peer list of node, all current connections will be closed after this command is sent, valid seed and node ip/port are required.\t\n");
    printf("\t-forcenexttick\n");
    printf("\t\t(equivalent to F5) Remotely force next tick on node to be empty, valid seed and node ip/port are required.\t\n");
    printf("\t-reissuevote\n");
    printf("\t\t(equivalent to F9) Remotely re-issue (re-send) vote on node, valid seed and node ip/port are required.\t\n");
    printf("\t-sendrawpacket <DATA_IN_HEX> <SIZE>\n");
    printf("\t\tSend a raw packet to nodeip. Valid node ip/port are required.\n");
    printf("\t-synctime\n");
    printf("\t\tSync node time with local time, valid seed and node ip/port are required. Make sure that your local time is synced (with NTP)!\t\n");
    printf("\t-getminingscoreranking\n");
    printf("\t\tGet current mining score ranking. Valid seed and node ip/port are required.\t\n");
    printf("\t-getvotecountertx <COMPUTOR_LIST_FILE> <TICK>\n");
    printf("\t\tGet vote counter transaction of a tick: showing how many votes per ID that this tick leader saw from (<TICK>-675-3) to (<TICK>-3) \t\n");
    printf("\t-setloggingmode <MODE>\n");
    printf("\t\tSet console logging mode: 0 disabled, 1 low computational cost, 2 full logging. Valid seed and node ip/port are required.\t\n");
    printf("\t-compmessage \"<MESSAGE>\"\n");
    printf("\t\tBroadcast a message on Qubic network, the message will be relayed to discord via bot. Node ip/port are required. Seed for a valid comp is required\t\n");
    printf("\t-savesnapshot \n");
    printf("\t\tRemotely trigger saving snapshot, valid seed and node ip/port are required.\t\n");
    printf("\t-setexecutionfeemultiplier <NUMERATOR> <DENOMINATOR>\n");
    printf("\t\tSet the multiplier for the conversion of raw execution time to contract execution fees to ( NUMERATOR / DENOMINATOR ), valid seed and node ip/port are required.\t\n");
    printf("\t-getexecutionfeemultiplier\n");
    printf("\t\tGet the current multiplier for the conversion of raw execution time to contract execution fees, valid seed and node ip/port are required.\t\n");


    printf("\n[SMART CONTRACT COMMANDS]\n");
    printf("\t-callcontractfunction <CONTRACT_INDEX> <CONTRACT_FUNCTION> <INPUT_FORMAT_STRING> <OUTPUT_FORMAT_STRING>\n");
    printf("\t\tCall a contract function of contract index and print the output. Valid node ip/port are required.\t\n");
    printf("\t-invokecontractprocedure <CONTRACT_INDEX> <CONTRACT_PROCEDURE> <AMOUNT> <INPUT_FORMAT_STRING>\n");
    printf("\t\tInvoke a procedure of contract index. Valid seed and node ip/port are required.\t\n");

    printf("\t-setshareholderproposal <CONTRACT_INDEX> <PROPOSAL_STRING>\n");
    printf("\t\tSet shareholder proposal in a contract. May overwrite existing proposal, because each seed can have only one proposal at a time. Costs a fee. You need to be shareholder of the contract.\n");
    printf("\t\t<PROPOSAL_STRING> is explained if there is a parsing error. Most contracts only support \"Variable|2\" (yes/no proposals to change state variable).\n");
    printf("\t-clearshareholderproposal <CONTRACT_INDEX>\n");
    printf("\t\tClear own shareholder proposal in a contract. Costs a fee.\n");
    printf("\t-getshareholderproposals <CONTRACT_INDEX> <PROPOSAL_INDEX_OR_GROUP>\n");
    printf("\t\tGet shareholder proposal info from a contract.\n");
    printf("\t\tEither pass \"active\" to get proposals that are open for voting in the current epoch, or \"finished\" to get proposals of previous epochs not overwritten or cleared yet, or a proposal index.\n");
    printf("\t-shareholdervote <CONTRACT_INDEX> <PROPOSAL_INDEX> <VOTE_VALUE>\n");
    printf("\t\tCast vote(s) for a shareholder proposal in the contract. You need to be shareholder of the contract.\n");
    printf("\t\t<VOTE_VALUE> may be a single value to set all your votes (one per share) to the same value.\n");
    printf("\t\tIn this case, <VOTE_VALUE> is the option in range 0 ... N-1 or \"none\" (in usual case of option voting), or an arbitrary integer or \"none\" (if proposal is for scalar voting).\n");
    printf("\t\t<VOTE_VALUE> also may be a comma-separated list of pairs of count and value (for example: \"3,0,10,1\" meaning 3 votes for option 0 and 10 votes for option 1).\n");
    printf("\t\tIf the total count is less than the number of shares you own, the remaining votes will be set to \"none\".\n");
    printf("\t-getshareholdervotes <CONTRACT_INDEX> <PROPOSAL_INDEX> [VOTER_IDENTITY]\n");
    printf("\t\tGet shareholder proposal votes of the contract. If VOTER_IDENTITY is skipped, identity of seed is used.\n");
    printf("\t-getshareholderresults <CONTRACT_INDEX> <PROPOSAL_INDEX>\n");
    printf("\t\tGet the current result of a shareholder proposal.\n");

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
    printf("\t\tFor subscription proposals, append subscription parameters to PROPOSAL_STRING: |<WEEKS_PER_PERIOD>|<START_EPOCH>|<NUMBER_OF_PERIODS>. The AMOUNT in the transfer proposal is used as AMOUNT_PER_PERIOD.\n");
    printf("\t\tTo cancel the active subscription, <AMOUNT> or <WEEKS_PER_PERIOD> or <NUMBER_OF_PERIODS> should be zero.\n");
    printf("\t-ccfclearproposal\n");
    printf("\t\tClear own proposal in CCF contract. Costs a fee.\n");
    printf("\t-ccfgetproposals <PROPOSAL_INDEX_OR_GROUP>\n");
    printf("\t\tGet proposal info from CCF contract.\n");
    printf("\t\tEither pass \"active\" to get proposals that are open for voting in the current epoch, or \"finished\" to get proposals of previous epochs not overwritten or cleared yet, or a proposal index.\n");
    printf("\t-ccfgetsubscription <SUBSCRIPTION_DESTINATION>\n");
    printf("\t\tGet active subscription info for a specific destination from CCF contract.\n");
    printf("\t-ccfvote <PROPOSAL_INDEX> <VOTE_VALUE>\n");
    printf("\t\tCast vote for a proposal in the CCF contract.\n");
    printf("\t\t<VOTE_VALUE> is the option in range 0 ... N-1 or \"none\".\n");
    printf("\t-ccfgetvote <PROPOSAL_INDEX> [VOTER_IDENTITY]\n");
    printf("\t\tGet vote from CCF contract. If VOTER_IDENTITY is skipped, identity of seed is used.\n");
    printf("\t-ccfgetresults <PROPOSAL_INDEX>\n");
    printf("\t\tGet the current result of a CCF proposal.\n");
    printf("\t-ccflatesttransfers\n");
    printf("\t\tGet and print latest transfers of CCF granted by quorum.\n");
    printf("\t-ccfgetregularpayments\n");
    printf("\t\tGet and print regular subscription payments of CCF.\n");

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
    printf("\t-qvaultsubmitauthaddress <NEW_ADDRESS>\n");
    printf("\t\tSubmit the new authaddress using multisig address.\n");
    printf("\t-qvaultchangeauthaddress <NUMBER_OF_CHANGED_ADDRESS>\n");
    printf("\t\tChange the authaddress using multisig address. <NUMBER_OF_CHANGED_ADDRESS> is the one of (1, 2, 3).\n");
    printf("\t-qvaultsubmitfees <NEW_QCAPHOLDER_PERMILLE> <NEW_REINVESTING_PERMILLE> <NEW_DEV_PERMILLE>\n");
    printf("\t\tSubmit the new permilles for QcapHolders, Reinvesting, Development using multisig address. the sum of 3 permilles should be 970 because the permille of shareHolder is 30.\n");
    printf("\t-qvaultchangefees <NEW_QCAPHOLDER_PERMILLE> <NEW_REINVESTING_PERMILLE> <NEW_DEV_PERMILLE>\n");
    printf("\t\tChange the permilles for QcapHolders, Reinvesting, Development using multisig address. the sum of 3 permilles should be 970 because the permille of shareHolder is 30. Get the locked amount that the user <IDENTITY> locked in the epoch <EPOCH>.\n");
    printf("\t-qvaultsubmitreinvestingaddress <NEW_ADDRESS>\n");
    printf("\t\tSubmit the new reinvesting address using multisig address.\n");
    printf("\t-qvaultchangereinvestingaddress <NEW_ADDRESS>\n");
    printf("\t\tChange the address using multisig address. <NEW_ADDRESS> should be already submitted by -qvaultsubmitreinvestingaddress command.\n");
    printf("\t-qvaultsubmitadminaddress <NEW_ADDRESS>\n");
    printf("\t\tSubmit the admin address using multisig address.\n");
    printf("\t-qvaultchangeadminaddress <NEW_ADDRESS>\n");
    printf("\t\tChange the admin address using multisig address. <NEW_ADDRESS> should be already submitted by -qvaultsubmitadminaddress command.\n");
    printf("\t-qvaultgetdata\n");
    printf("\t\tGet the state data of smart contract. anyone can check the changes after using the any command.\n");
    printf("\t-qvaultsubmitbannedaddress <NEW_ADDRESS>\n");
    printf("\t\tSubmit the banned address using multisig address.\n");
    printf("\t-qvaultsavebannedaddress <NEW_ADDRESS>\n");
    printf("\t\tSave the banned address using multisig address. <NEW_ADDRESS> should be already submitted by -qvaultsubmitbannedaddress command.\n");
    printf("\t-qvaultsubmitunbannedaddress <NEW_ADDRESS>\n");
    printf("\t\tSubmit the unbanned address using multisig address.\n");
    printf("\t-qvaultsaveunbannedaddress <NEW_ADDRESS>\n");
    printf("\t\tUnban the <NEW_ADDRESS> using the multisig address. <NEW_ADDRESS> should be already submitted by -qvaultsaveunbannedaddress command.\n");

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
    printf("\t-msvaultdepositasset <VAULT_ID> <ASSET_NAME> <ISSUER_ID> <AMOUNT>\n");
    printf("\t\tDeposit a custom asset into a vault. Note: You must first grant management rights of the shares to the MsVault contract.\n");
    printf("\t-msvaultreleaseassetto <VAULT_ID> <ASSET_NAME> <ISSUER_ID> <AMOUNT> <DESTINATION_ID>\n");
    printf("\t\tRequest to release a custom asset from a vault to a destination. Fee applies.\n");
    printf("\t-msvaultresetassetrelease <VAULT_ID>\n");
    printf("\t\tReset your pending custom asset release request for a vault. Fee applies.\n");
    printf("\t-msvaultgetassetbalances <VAULT_ID>\n");
    printf("\t\tGet the custom asset balances for a vault.\n");
    printf("\t-msvaultgetassetreleasestatus <VAULT_ID>\n");
    printf("\t\tGet the pending custom asset release statuses for a vault.\n");
    printf("\t-msvaultgetmanagedassetbalance <ASSET_NAME> <ISSUER_ID> <OWNER_ID>\n");
    printf("\t\tGet the managed asset balance for a specific owner.\n");
    printf("\t-msvaultrevokeassetrights <ASSET_NAME> <ISSUER_ID> <NUMBER_OF_SHARES>\n");
    printf("\t\tRevoke asset management rights from MsVault, transferring them back to QX. Fee applies.\n");
    printf("\t-msvaultisshareholder <IDENTITY>\n");
    printf("\t\tCheck if the provided identity is a shareholder of the MsVault contract.\n");
    printf("\t-msvaultvotechange <REGISTER_FEE> <RELEASE_FEE> <RESET_RELEASE_FEE> <HOLD_FEE> <DEPOSIT_FEE>\n");
    printf("\t\tAs a shareholder, vote to change the contract fees. A deposit is required, which is refunded if the vote passes.\n");
    printf("\t-msvaultgetfeevotes\n");
    printf("\t\tGet the list of all active fee change votes.\n");
    printf("\t-msvaultgetfeevotesowner\n");
    printf("\t\tGet the list of shareholders who have cast a fee change vote.\n");
    printf("\t-msvaultgetfeevotesscore\n");
    printf("\t\tGet the corresponding scores (voting power) for each active fee change vote.\n");
    printf("\t-msvaultgetuniquefeevotes\n");
    printf("\t\tGet the list of unique fee proposals that have been voted on.\n");
    printf("\t-msvaultgetuniquefeevotesranking\n");
    printf("\t\tGet the aggregated scores (total voting power) for each unique fee proposal.\n");

    printf("\n[QSWAP COMMANDS]\n");
    printf("\t-qswapgetfee\n");
    printf("\t\tShow current Qswap fee.\n");
    printf("\t-qswapissueasset <ASSET_NAME> <NUMBER_OF_UNIT> <UNIT_OF_MEASUREMENT> <NUM_DECIMAL>\n");
    printf("\t\tCreate an asset via Qswap contract.\n");
    printf("\t-qswaptransferasset <ASSET_NAME> <ISSUER_IN_HEX> <NEW_OWNER_IDENTITY> <AMOUNT_OF_SHARE>\n");
    printf("\t\tTransfer an asset via Qswap contract.\n");
    printf("\t-qswaptransferassetrights <ASSET_NAME> <ISSUER_IN_HEX> <NEW_MANAGING_CONTRACT_INDEX> <AMOUNT_OF_SHARE>\n");
    printf("\t\tTransfer an asset rights.\n");
    printf("\t-qswapcreatepool <ASSET_NAME> <ISSUER_IN_HEX>\n");
    printf("\t\tCreate an AMM pool via Qswap contract.\n");
    printf("\t-qswapgetpoolbasicstate <ASSET_NAME> <ISSUER_IN_HEX>\n");
    printf("\t\tGet the basic information of a pool.\n");
    printf("\t-qswapaddliquidity <ASSET_NAME> <ISSUER_IN_HEX> <QU_AMOUNT_IN> <ASSET_AMOUNT_DESIRED> <QU_AMOUNT_MIN> <ASSET_AMOUNT_MIN>\n");
    printf("\t\tAdd liquidity with restriction to an AMM pool via Qswap contract.\n");
    printf("\t-qswapremoveliquidity <ASSET_NAME> <ISSUER_IN_HEX> <BURN_LIQUIDITY> <QU_AMOUNT_MIN> <ASSET_AMOUNT_MIN>\n");
    printf("\t\tRemove liquidity with restriction from an AMM pool via Qswap contract.\n");
    printf("\t-qswapgetliquidityof <ASSET_NAME> <ISSUER_IN_HEX> [LIQUIDITY_STAKER(in qublic format)]\n");
    printf("\t\tGet the staker's liquidity in a pool.\n");
    printf("\t-qswapswapexactquforasset <ASSET_NAME> <ISSUER_IN_HEX> <QU_AMOUNT_IN> <ASSET_AMOUNT_OUT_MIN>\n");
    printf("\t\tSwap qu for asset via Qswap contract, only execute if asset_amount_out >= ASSET_AMOUNT_OUT_MIN.\n");
    printf("\t-qswapswapquforexactasset <ASSET_NAME> <ISSUER_IN_HEX> <ASSET_AMOUNT_OUT> <QU_AMOUNT_IN_MAX>\n");
    printf("\t\tSwap qu for asset via Qswap contract, only execute if qu_amount_in <= QU_AMOUNT_IN_MAX.\n");
    printf("\t-qswapswapexactassetforqu <ASSET_NAME> <ISSUER_IN_HEX> <ASSET_AMOUNT_IN> <QU_AMOUNT_OUT_MIN>\n");
    printf("\t\tSwap asset for qu via Qswap contract, only execute if qu_amount_out >= QU_AMOUNT_OUT_MIN.\n");
    printf("\t-qswapswapassetforexactqu <ASSET_NAME> <ISSUER_IN_HEX> <QU_AMOUNT_OUT> <ASSET_AMOUNT_IN_MAX>\n");
    printf("\t\tSwap asset for qu via Qswap contract, only execute if asset_amount_in <= ASSET_AMOUNT_IN_MAX.\n");
    printf("\t-qswapquote exact_qu_input/exact_qu_output/exact_asset_input/exact_asset_output <ASSET_NAME> <ISSUER_IN_HEX> <AMOUNT>\n");
    printf("\t\tQuote amount_out/amount_in with the given amount_in/amount_out via Qswap contract.\n");

    printf("\n[NOSTROMO COMMANDS]\n");
    printf("\t-nostromoregisterintier <TIER_LEVEL>\n");
    printf("\t\tRegister in tier.\n");
    printf("\t-nostromologoutfromtier \n");
    printf("\t\tLogout from tier.\n");
    printf("\t-nostromocreateproject <TOKEN_NAME> <SUPPLY_OF_TOKEN> <START_YEAR> <START_MONTH> <START_DAY> <START_HOUR> <END_YEAR> <END_MONTH> <END_DAY> <END_HOUR>\n");
    printf("\t\tCreate a project with the specified token info and start and end date for voting.\n");
    printf("\t-nostromovoteinproject <PROJECT_INDEX> <DECISION>\n");
    printf("\t\tVote in the project with <DECISION> in the <PROJECT_INDEX> -> if you want to vote with yes, it should be 1. otherwise it is 0.\n");
    printf("\t-nostromocreatefundraising <TOKEN_PRICE> <SALE_AMOUNT> <REQUIRED_FUND> <PROJECT_INDEX> \n");
    printf("\t\t<FIRST_PHASE_START_YEAR> <FIRST_PHASE_START_MONTH> <FIRST_PHASE_START_DAY> <FIRST_PHASE_START_HOUR>\n");
    printf("\t\t<FIRST_PHASE_END_YEAR> <FIRST_PHASE_END_MONTH> <FIRST_PHASE_END_DAY> <FIRST_PHASE_END_HOUR>\n");
    printf("\t\t<SECOND_PHASE_START_YEAR> <SECOND_PHASE_START_MONTH> <SECOND_PHASE_START_DAY> <SECOND_PHASE_START_HOUR>\n");
    printf("\t\t<SECOND_PHASE_END_YEAR> <SECOND_PHASE_END_MONTH> <SECOND_PHASE_END_DAY> <SECOND_PHASE_END_HOUR>\n");
    printf("\t\t<THIRD_PHASE_START_YEAR> <THIRD_PHASE_START_MONTH> <THIRD_PHASE_START_DAY> <THIRD_PHASE_START_HOUR>\n");
    printf("\t\t<THIRD_PHASE_END_YEAR> <THIRD_PHASE_END_MONTH> <THIRD_PHASE_END_DAY> <THIRD_PHASE_END_HOUR>\n");
    printf("\t\t<LISTING_START_YEAR> <LISTING_START_MONTH> <LISTING_START_DAY> <LISTING_START_HOUR>\n");
    printf("\t\t<CLIFF_END_YEAR> <CLIFF_END_MONTH> <CLIFF_END_DAY> <CLIFF_END_HOUR>\n");
    printf("\t\t<VESTING_END_YEAR> <VESTING_END_MONTH> <VESTING_END_DAY> <VESTING_END_HOUR>\n");
    printf("\t\t<THRESHOLDS> <TGE> <NUMBER_OF_STEP_FOR_VESTING>\n");
    printf("\t\tCreate a fundraising with the specified token and project infos.\n");
    printf("\t-nostromoinvestinproject <FUNDRAISING_INDEX> <INVESTMENT_AMOUNT>\n");
    printf("\t\tInvest in the fundraising.\n");
    printf("\t-nostromoclaimtoken <CLAIM_AMOUNT> <FUNDRAISING_INDEX>\n");
    printf("\t\tClaim your token from SC.\n");
    printf("\t\tIf you invest in the fundraising and also it is the time for claiming, you can receive the token from SC.\n");
    printf("\t-nostromoupgradetierlevel <NEW_TIER_LEVEL>\n");
    printf("\t\tUpgrade your tierlevel to <NEW_TIER_LEVEL>\n");
    printf("\t-nostromotransfersharemanagementrights <TOKEN_NAME> <TOKEN_ISSUER> <NEW_MANAGEMENT_CONTRACT_INDEX> <AMOUNT_OF_TRANSFER>\n");
    printf("\t\tTransfer the share management right to <NEW_MANAGEMENT_CONTRACT_INDEX>\n");
    printf("\t-nostromogetstats\n");
    printf("\t\tGet the infos of SC(like total pool weight, epoch revenue, number of registers, number of projects, ...)\n");
    printf("\t-nostromogettierlevelbyuser <USER_ID>\n");
    printf("\t\tGet the tier_level for <USER_ID>.\n");
    printf("\t-nostromogetuservotestatus <USER_ID>\n");
    printf("\t\tGet the list of project index voted by <USER_ID>.\n");
    printf("\t-nostromochecktokencreatability <TOKEN_NAME>\n");
    printf("\t\tCheck if the <TOKEN_NAME> can be issued by SC.\n");
    printf("\t\tIf <TOKEN_NAME> is already created by SC, it can not be issued anymore.\n");
	printf("\t-nostromogetnumberofinvestedprojects <USER_ID>\n");
	printf("\t\tGet the number invested and project. you can check if the <USER_ID> can invest.\n");
	printf("\t\tThe max number that can invest by one user at once in SC is 128 currently.\n");
	printf("\t-nostromogetprojectbyindex <PROJECT_INDEX>\n");
	printf("\t\tGet the infos of project.\n");
	printf("\t-nostromogetfundraisingbyindex <FUNDRAISING_INDEX>\n");
	printf("\t\tGet the infos of fundraising.\n");
	printf("\t-nostromogetprojectindexlistbycreator <USER_ID>\n");
	printf("\t\tGet the list of project that <USER_ID> created.\n");
	printf("\t-nostromogetInfoUserInvested <INVESTOR_ADDRESS>\n");
	printf("\t\tGet the invseted infos(indexOfFundraising, InvestedAmount, ClaimedAmount).\n");
	printf("\t-nostromogetmaxclaimamount <INVESTOR_ADDRESS> <INDEX_OF_FUNDRAISING>\n");
	printf("\t\tGet the max claim amount at the moment.\n");

    printf("\n[QBOND COMMANDS]\n");
    printf("\t-qbondstake <MILLIONS_AMOUNT>\n");
    printf("\t\tStake QU and get MBNDxxx token for every million of QU.\n");
    printf("\t-qbondtransfer <IDENTITY> <EPOCH> <AMOUNT>\n");
    printf("\t\tTransfer <AMOUNT> of MBonds of specific <EPOCH> to new owner <IDENTITY>\n");
    printf("\t-qbondaddask <EPOCH> <PRICE> <AMOUNT>\n");
    printf("\t\tAdd ask order of <AMOUNT> MBonds of <EPOCH> at <PRICE>\n");
    printf("\t-qbondremoveask <EPOCH> <PRICE> <AMOUNT>\n");
    printf("\t\tRemove <AMOUNT> MBonds of <EPOCH> from ask order at <PRICE>\n");
    printf("\t-qbondaddbid <EPOCH> <PRICE> <AMOUNT>\n");
    printf("\t\tAdd bid order of <AMOUNT> MBonds of <EPOCH> at <PRICE>\n");
    printf("\t-qbondremovebid <EPOCH> <PRICE> <AMOUNT>\n");
    printf("\t\tRemove <AMOUNT> MBonds of <EPOCH> from bid order at <PRICE>\n");
    printf("\t-qbondburnqu <AMOUNT>\n");
    printf("\t\tBurn <AMOUNT> of qu by QBOND sc.\n");
    printf("\t-qbondupdatecfa <IDENTITY> <OPERATION>\n");
    printf("\t\tOnly for admin! Update commission free addresses. <OPERATION> must be 0 to remove <IDENTITY> or 1 to add.\n");
    printf("\t-qbondgetfees\n");
    printf("\t\tGet fees of QBond sc.\n");
    printf("\t-qbondgetearnedfees\n");
    printf("\t\tGet earned fees by QBond sc.\n");
    printf("\t-qbondgetinfoperepoch <EPOCH>\n");
    printf("\t\tGet overall information about <EPOCH> (stakers amount, total staked, APY)\n");
    printf("\t-qbondgetorders <EPOCH> <ASKS_OFFSET> <BIDS_OFFSET>\n");
    printf("\t\tGet orders of <EPOCH> MBonds.\n");
    printf("\t-qbondgetuserorders <OWNER> <ASKS_OFFSET> <BIDS_OFFSET>\n");
    printf("\t\tGet MBonds orders owner by <OWNER>.\n");
    printf("\t-qbondtable\n");
    printf("\t\tGet info about APY of each MBond.\n");
    printf("\t-qbondgetusermbonds <OWNER>\n");
    printf("\t\tGet MBonds owned by the <OWNER>.\n");
    printf("\t-qbondgetcfa\n");
    printf("\t\tGet list of commission free addresses.\n");

    printf("\n[TESTING COMMANDS]\n");
    printf("\t-testqpifunctionsoutput\n");
    printf("\t\tTest that output of qpi functions matches TickData and quorum tick votes for 15 ticks in the future (as specified by scheduletick offset). Requires the TESTEXA SC to be enabled.\n");
    printf("\t-testqpifunctionsoutputpast\n");
    printf("\t\tTest that output of qpi functions matches TickData and quorum tick votes for the last 15 ticks. Requires the TESTEXA SC to be enabled.\n");
    printf("\t-testgetincomingtransferamounts <B_OR_C>\n");
    printf("\t\tGet incoming transfer amounts from either TESTEXB (\"B\") or TESTEXC (\"C\"). Requires the TESTEXB and TESTEXC SCs to be enabled.\n");
    printf("\t-testbidinipothroughcontract <B_OR_C> <CONTRACT_INDEX> <NUMBER_OF_SHARE> <PRICE_PER_SHARE>\n");
    printf("\t\tBid in an IPO either as TESTEXB (\"B\") or as TESTEXC (\"C\"). Requires the TESTEXB and TESTEXC SCs to be enabled.\n");
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

static uint32_t getContractIndex(const char* str, bool enableTestContracts)
{
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
    else if (strcasecmp(str, "MSVAULT") == 0)
        idx = 11;
    else if (strcasecmp(str, "QBAY") == 0)
        idx = 12;
    else if (strcasecmp(str, "QSWAP") == 0)
        idx = 13;
    else if (strcasecmp(str, "NOST") == 0)
        idx = 14;
    else if (strcasecmp(str, "QDRAW") == 0)
        idx = 15;
    else if (strcasecmp(str, "RL") == 0)
        idx = 16;
    else if (strcasecmp(str, "QBOND") == 0)
        idx = 17;
    else if (strcasecmp(str, "QIP") == 0)
        idx = 18;
    else
    {
        unsigned int contractCount = CONTRACT_COUNT;
        if (enableTestContracts)
        {
            if (strcasecmp(str, "TESTEXA") == 0)
                idx = CONTRACT_COUNT + 1;
            else if (strcasecmp(str, "TESTEXB") == 0)
                idx = CONTRACT_COUNT + 2;
            else if (strcasecmp(str, "TESTEXC") == 0)
                idx = CONTRACT_COUNT + 3;
            else if (strcasecmp(str, "TESTEXD") == 0)
                idx = CONTRACT_COUNT + 4;

            contractCount += 4; // + 4 to make contracts TestExampleA-D accessible via contract index number
        }
        if (sscanf(str, "%u", &idx) != 1 || idx == 0 || (g_nodePort == DEFAULT_NODE_PORT && (idx > contractCount)))
        {
            LOG("Contract \"%s\" is unknown!\n", str);
            exit(1);
        }
    }
    return idx;
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
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "-force") == 0)
        {
            g_force = true;
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-enabletestcontracts") == 0)
        {
            g_enableTestContracts = true;
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-print-only") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1);
            g_printToScreen = argv[i+1];
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
        if (strcmp(argv[i], "-gettotalnumberofassetshares") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = GET_TOTAL_NUMBER_OF_ASSET_SHARES;
            g_paramString1 = argv[i + 1];
            g_paramString2 = argv[i + 2];
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutildistributequbictoshareholders") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QUTIL_DISTRIBUTE_QU_TO_SHAREHOLDERS;
            g_paramString1 = argv[i + 1];
            g_paramString2 = argv[i + 2];
            g_txAmount = charToNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-sendtoaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = SEND_COIN;
            g_targetIdentity = argv[i+1];
            g_txAmount = charToNumber(argv[i+2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-sendtoaddressintick") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = SEND_COIN_IN_TICK;
            g_targetIdentity = argv[i+1];
            g_txAmount = uint32_t(charToNumber(argv[i+2]));
            g_txTick = uint32_t(charToNumber(argv[i+3]));
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
            g_filePath = argv[i+1];
            i+=2;
            if (i < argc)
            {
                g_compressTool = argv[i];
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
            g_filePath = argv[i+2];
            i+=3;
            if (i < argc)
            {
                g_compressTool = argv[i];
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
            g_txType = uint16_t(charToNumber(argv[i+2]));
            g_txAmount = charToNumber(argv[i+3]);
            g_txExtraDataSize = int(charToNumber(argv[i+4]));
            hexToByte(argv[i+5], g_txExtraData, g_txExtraDataSize);
            i+=6;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-invokecontractprocedure") == 0) {
            CHECK_NUMBER_OF_PARAMETERS(4);
            g_cmd = INVOKE_CONTRACT_PROCEDURE;
            g_contractIndex = getContractIndex(argv[i+1], g_enableTestContracts);
            g_txType = uint16_t(charToNumber(argv[i+2]));
            g_txAmount = charToNumber(argv[i+3]);
            g_invokeContractProcedureInputFormat = argv[i+4];
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-callcontractfunction") == 0) {
            CHECK_NUMBER_OF_PARAMETERS(4);
            g_cmd = CALL_CONTRACT_FUNCTION;
            g_contractIndex = getContractIndex(argv[i+1], g_enableTestContracts);
            g_contractFunctionNumber = uint16_t(charToNumber(argv[i+2]));
            g_callContractFunctionInputFormat = argv[i+3];
            g_callContractFunctionOutputFormat = argv[i+4];
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-dumpspectrumfile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = DUMP_SPECTRUM_FILE;
            g_dumpBinaryFileInput = argv[i+1];
            g_dumpBinaryFileOutput = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-dumpuniversefile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = DUMP_UNIVERSE_FILE;
            g_dumpBinaryFileInput = argv[i+1];
            g_dumpBinaryFileOutput = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-dumpcontractfile") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = DUMP_CONTRACT_FILE;
            g_dumpBinaryFileInput = argv[i+1];
            g_dumpBinaryContractId = uint32_t(charToNumber(argv[i+2]));
            g_dumpBinaryFileOutput = argv[i+3];
            i+=4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-makeipobid") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = MAKE_IPO_BID;
            g_IPOContractIndex = uint32_t(charToNumber(argv[i + 1]));
            g_makeIPOBidNumberOfShare = uint16_t(charToNumber(argv[i+2]));
            g_makeIPOBidPricePerShare = charToNumber(argv[i+3]);
            i+=4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getipostatus") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = GET_IPO_STATUS;
            g_IPOContractIndex = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getactiveipos") == 0)
        {
            g_cmd = GET_ACTIVE_IPOS;
            i++;
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
            g_toggleMainAux0 = argv[i+1];
            g_toggleMainAux1 = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-setsolutionthreshold") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = SET_SOLUTION_THRESHOLD;
            g_setSolutionThresholdEpoch = int(charToNumber(argv[i+1]));
            g_setSolutionThresholdValue = int(charToNumber(argv[i+2]));
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
        if (strcmp(argv[i], "-savesnapshot") == 0)
        {
            g_cmd = SAVE_SNAPSHOT;
            g_requestedSpecialCommand = SPECIAL_COMMAND_SAVE_SNAPSHOT;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-sendrawpacket") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = SEND_RAW_PACKET;
            g_rawPacketSize = int(charToNumber(argv[i+2]));
            hexToByte(argv[i+1], g_rawPacket, g_rawPacketSize);
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
        if (strcmp(argv[i], "-setexecutionfeemultiplier") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = SET_EXECUTION_FEE_MULTIPLIER;
            g_executionFeeMultiplierNumerator = charToNumber(argv[i + 1]);
            g_executionFeeMultiplierDenominator = charToNumber(argv[i + 2]);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-getexecutionfeemultiplier") == 0)
        {
            g_cmd = GET_EXECUTION_FEE_MULTIPLIER;
            i++;
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
            g_qx_issueAssetName = argv[i+1];
            g_qx_issueAssetNumberOfUnit = charToNumber(argv[i+2]);
            g_qx_issueUnitOfMeasurement = argv[i+3];
            g_qx_issueAssetNumDecimal = char(charToNumber(argv[i+4]));
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qxtransferasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QX_TRANSFER_ASSET;
            g_qx_assetTransferAssetName = argv[i+1];
            g_qx_assetTransferIssuerInHex = argv[i+2];
            g_qx_assetTransferNewOwnerIdentity = argv[i+3];
            g_qx_assetTransferAmount = charToNumber(argv[i+4]);
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
            g_qx_command1 = argv[i+1];
            g_qx_command2 = argv[i+2];
            g_qx_issuer = argv[i+3];
            g_qx_assetName = argv[i+4];
            g_qx_price = charToNumber(argv[i+5]);
            g_qx_numberOfShare = charToNumber(argv[i+6]);
            i+=7;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qxgetorder") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(5)
            g_cmd = QX_GET_ORDER;
            g_qx_command1 = argv[i+1];
            g_qx_command2 = argv[i+2];
            g_qx_issuer = argv[i+3];
            g_qx_assetName = argv[i+4];
            g_qx_offset = charToNumber(argv[i+5]);
            i+=6;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qxtransferrights") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QX_TRANSFER_MANAGEMENT_RIGHTS;
            g_qx_assetName = argv[i + 1];
            g_qx_issuer = argv[i + 2];
            g_contractIndex = getContractIndex(argv[i + 3], g_enableTestContracts);
            g_qx_numberOfShare = charToNumber(argv[i + 4]);
            i += 5;
            CHECK_OVER_PARAMETERS
            break;
        }

        /***********************
         ***** QSWAP COMMANDS *****
         ***********************/

        if (strcmp(argv[i], "-qswapissueasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QSWAP_ISSUE_ASSET;
            g_qswap_issueAssetName = argv[i+1];
            g_qswap_issueAssetNumberOfUnit = charToNumber(argv[i+2]);
            g_qswap_issueUnitOfMeasurement = argv[i+3];
            g_qswap_issueAssetNumDecimal = static_cast<char>(charToNumber(argv[i+4]));
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswaptransferasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QSWAP_TRANSFER_ASSET;
            g_qswap_assetTransferAssetName = argv[i+1];
            g_qswap_assetTransferIssuer = argv[i+2];
            g_qswap_assetTransferNewOwnerIdentity = argv[i+3];
            g_qswap_assetTransferAmount = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswaptransferassetrights") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QSWAP_TRANSFER_ASSET_RIGHTS;
            g_qswap_assetTransferAssetName = argv[i+1];
            g_qswap_assetTransferIssuer = argv[i+2];
            g_qswap_newContractIndex = getContractIndex(argv[i + 3], g_enableTestContracts);
            g_qswap_assetTransferAmount = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapgetfee") == 0)
        {
            g_cmd = PRINT_QSWAP_FEE;
            i+=1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapcreatepool") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QSWAP_CREATE_POOL;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapgetpoolbasicstate") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QSWAP_GET_POOL_BASIC;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapaddliquidity") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(6)
            g_cmd = QSWAP_ADD_LIQUIDITY;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            g_qswap_addLiquidityQuAmount = charToNumber(argv[i+3]);
            g_qswap_addLiquidityAssetAmountDesired = charToNumber(argv[i+4]);
            g_qswap_liquidityQuAmountMin = charToNumber(argv[i+5]);
            g_qswap_liquidityAssetAmountMin = charToNumber(argv[i+6]);
            i+=7;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapremoveliquidity") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(5)
            g_cmd = QSWAP_REMOVE_LIQUIDITY;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            g_qswap_removeLiquidityBurnLiquidity = charToNumber(argv[i+3]);
            g_qswap_liquidityQuAmountMin = charToNumber(argv[i+4]);
            g_qswap_liquidityAssetAmountMin = charToNumber(argv[i+5]);
            i+=6;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapgetliquidityof") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QSWAP_GET_LIQUIDITY_OF;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            g_qswap_getLiquidityOfStakerIssuer = argv[i+3];
            i+=4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapswapexactquforasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QSWAP_SWAP_EXACT_QU_FOR_ASSET;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            g_qswap_swapAmountIn = charToNumber(argv[i+3]);
            g_qswap_swapAmountOutMin = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapswapquforexactasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QSWAP_SWAP_QU_FOR_EXACT_ASSET;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            g_qswap_swapAmountOut = charToNumber(argv[i+3]);
            g_qswap_swapAmountInMax = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapswapexactassetforqu") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QSWAP_SWAP_EXACT_ASSET_FOR_QU;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            g_qswap_swapAmountIn = charToNumber(argv[i+3]);
            g_qswap_swapAmountOutMin = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapswapassetforexactqu") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QSWAP_SWAP_ASSET_FOR_EXACT_QU;
            g_qswap_assetName = argv[i+1];
            g_qswap_issuer = argv[i+2];
            g_qswap_swapAmountOut = charToNumber(argv[i+3]);
            g_qswap_swapAmountInMax = charToNumber(argv[i+4]);
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qswapquote") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = QSWAP_QUOTE;
            g_qswap_command1 = argv[i+1];
            g_qswap_assetName = argv[i+2];
            g_qswap_issuer = argv[i+3];
            g_qswap_quoteAmount = charToNumber(argv[i+4]);
            i+=5;
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
            g_quottery_betId = uint32_t(charToNumber(argv[i + 1]));
            g_quottery_numberBetSlot = charToNumber(argv[i+2]);
            g_quottery_amountPerBetSlot = charToNumber(argv[i+3]);
            g_quottery_pickedOption = uint32_t(charToNumber(argv[i+4]));
            i+=5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrygetbetinfo") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUOTTERY_GET_BET_INFO;
            g_quottery_betId = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrygetbetdetail") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QUOTTERY_GET_BET_DETAIL;
            g_quottery_betId = uint32_t(charToNumber(argv[i + 1]));
            g_quottery_optionId = uint32_t(charToNumber(argv[i + 2]));
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
            g_quottery_creatorId = argv[i+1];
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
            g_quottery_betId = uint32_t(charToNumber(argv[i + 1]));
            g_quottery_optionId = uint32_t(charToNumber(argv[i + 2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qtrycancelbet") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUOTTERY_CANCEL_BET;
            g_quottery_betId = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }

        /****************************
         ****** QUTIL COMMANDS ******
         ****************************/
        if (strcmp(argv[i], "-qutilsendtomanyv1") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_SEND_TO_MANY_V1;
            g_qutil_sendToManyV1PayoutListFile = argv[i + 1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilburnqubic") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_BURN_QUBIC;
            g_txAmount = charToNumber(argv[i + 1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilburnqubicforcontract") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QUTIL_BURN_QUBIC_FOR_CONTRACT;
            g_txAmount = charToNumber(argv[i + 1]);
            g_contractIndex = getContractIndex(argv[i + 2], g_enableTestContracts);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilqueryfeereserve") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_QUERY_FEE_RESERVE;
            g_contractIndex = getContractIndex(argv[i + 1], g_enableTestContracts);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilsendtomanybenchmark") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QUTIL_SEND_TO_MANY_BENCHMARK;
            g_qutil_sendToManyBenchmarkDestinationCount = charToNumber(argv[i + 1]);
            g_qutil_sendToManyBenchmarkNumTransfersEach = charToNumber(argv[i + 2]);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilcreatepoll") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
                g_cmd = QUTIL_CREATE_POLL;
            int base_params = 4;
            if (i + base_params >= argc)
            {
                LOG("Not enough parameters provided for command, expected at least 4.\nRun qubic-cli -h to display help.\n");
                exit(1);
            }
            g_qutil_pollNameStr = argv[i + 1];
            g_qutil_pollType = charToUnsignedNumber(argv[i + 2]);
            g_qutil_minAmount = charToUnsignedNumber(argv[i + 3]);
            g_qutil_githubLinkStr = argv[i + 4];

            int params_consumed = base_params;
            if (g_qutil_pollType == 2) // Asset poll
            {
                params_consumed = 5;
                if (i + params_consumed >= argc)
                {
                    LOG("Not enough parameters for Asset poll, expected 5 parameters.\n");
                    exit(1);
                }
                g_qutil_semicolonSeparatedAssets = argv[i + 5];
            }
            else if (g_qutil_pollType == 1) // Qubic poll
            {
                g_qutil_semicolonSeparatedAssets = nullptr;
            }
            else
            {
                LOG("Invalid POLL_TYPE. Must be 1 (Qubic) or 2 (Asset).\n");
                exit(1);
            }
            i += 1 + params_consumed;
            CHECK_OVER_PARAMETERS
                break;
        }
        if (strcmp(argv[i], "-qutilvote") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QUTIL_VOTE;
            g_qutil_votePollId = charToUnsignedNumber(argv[i + 1]);
            g_qutil_voteAmount = charToUnsignedNumber(argv[i + 2]);
            g_qutil_voteChosenOption = charToUnsignedNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilgetcurrentresult") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_GET_CURRENT_RESULT;
            g_qutil_getResultPollId = charToUnsignedNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilgetpollsbycreator") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_GET_POLLS_BY_CREATOR;
            g_qutil_getPollsCreatorAddress = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilgetcurrentpollid") == 0) {
            g_cmd = QUTIL_GET_CURRENT_POLL_ID;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilgetpollinfo") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_GET_POLL_INFO;
            g_qutil_getPollInfoPollId = charToUnsignedNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilcancelpoll") == 0) {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QUTIL_CANCEL_POLL;
            g_qutil_cancelPollId = charToUnsignedNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qutilgetfee") == 0)
        {
            g_cmd = QUTIL_PRINT_FEE;
            i += 1;
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
        if (strcmp(argv[i], "-ccfgetsubscription") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = CCF_GET_SUBSCRIPTION;
            g_ccf_subscriptionDestination = argv[i + 1];
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
        if (strcmp(argv[i], "-ccfgetregularpayments") == 0)
        {
            g_cmd = CCF_GET_REGULAR_PAYMENTS;
            i += 1;
            CHECK_OVER_PARAMETERS;
            break;
        }

        /**************************
         ***** QEARN COMMANDS *****
         **************************/

        if (strcmp(argv[i], "-qearnlock") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_LOCK;
            g_qearn_lockAmount = charToNumber(argv[i + 1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearnunlock") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QEARN_UNLOCK;
            g_qearn_unlockAmount = charToNumber(argv[i + 1]);
            g_qearn_lockedEpoch = uint32_t(charToNumber(argv[i + 2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetlockinfoperepoch") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_GET_INFO_PER_EPOCH;
            g_qearn_getInfoEpoch = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetuserlockedinfo") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QEARN_GET_USER_LOCKED_INFO;
            g_requestedIdentity = argv[i+1];
            g_qearn_getInfoEpoch = uint32_t(charToNumber(argv[i + 2]));
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qearngetstateofround") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QEARN_GET_STATE_OF_ROUND;
            g_qearn_getInfoEpoch = uint32_t(charToNumber(argv[i + 1]));
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
            g_qearn_getStatsEpoch = uint32_t(charToNumber(argv[i + 1]));
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
            g_qearn_getStatsEpoch = uint32_t(charToNumber(argv[i + 1]));
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitauthaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_SUBMIT_AUTH_ADDRESS;
            g_qvault_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultchangeauthaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_CHANGE_AUTH_ADDRESS;
            g_qvault_numberOfChangedAddress = uint32_t(charToNumber(argv[i + 1]));
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitfees") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QVAULT_SUBMIT_FEES;
            g_qvault_newQCAPHolderFee = uint32_t(charToNumber(argv[i + 1]));
            g_qvault_newReinvestingFee = uint32_t(charToNumber(argv[i + 2]));
            g_qvault_newDevFee = uint32_t(charToNumber(argv[i + 3]));
            i += 4;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultchangefees") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QVAULT_CHANGE_FEES;
            g_qvault_newQCAPHolderFee = uint32_t(charToNumber(argv[i + 1]));
            g_qvault_newReinvestingFee = uint32_t(charToNumber(argv[i + 2]));
            g_qvault_newDevFee = uint32_t(charToNumber(argv[i + 3]));
            i += 4;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitreinvestingaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_SUBMIT_REINVESTING_ADDRESS;
            g_qvault_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultchangereinvestingaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_CHANGE_REINVESTING_ADDRESS;
            g_qvault_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitadminaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_SUBMIT_ADMIN_ADDRESS;
            g_qvault_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultchangeadminaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_CHANGE_ADMIN_ADDRESS;
            g_qvault_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultgetdata") == 0)
        {
            g_cmd = QVAULT_GET_DATA;
            i += 1;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitbannedaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_SUBMIT_BANNED_ADDRESS;
            g_qvault_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsavebannedaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_SAVE_BANNED_ADDRESS;
            g_qvault_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsubmitunbannedaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_SUBMIT_UNBANNED_ADDRESS;
            g_qvault_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-qvaultsaveunbannedaddress") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QVAULT_SAVE_UNBANNED_ADDRESS;
            g_qvault_identity = argv[i + 1];
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
            g_msvault_requiredApprovals = (uint64_t)charToNumber(argv[i + 1]);

            {
                const char* inputVaultName = argv[i + 2];
                size_t len = strlen(inputVaultName);
                if (len > 32) {
                    LOG("Vault name must be at most 32 chars. Truncating...\n");
                    len = 32;
                }
                memset(g_msvault_vaultName, 0, 32);
                memcpy(g_msvault_vaultName, inputVaultName, len);
            }

            g_msvault_ownersCommaSeparated = argv[i + 3];
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultdeposit") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = MSVAULT_DEPOSIT_CMD;
            g_msvault_id = charToNumber(argv[i+1]);
            g_txAmount = charToNumber(argv[i+2]);
            i+=3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultreleaseto") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = MSVAULT_RELEASE_TO_CMD;
            g_msvault_id = charToNumber(argv[i+1]);
            g_txAmount = charToNumber(argv[i+2]);
            g_msvault_destination = argv[i + 3];
            i+=4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultresetrelease") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_RESET_RELEASE_CMD;
            g_msvault_id = charToNumber(argv[i+1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetvaults") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_VAULTS_CMD;
            g_msvault_publicId = argv[i + 1];
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetreleasestatus") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_RELEASE_STATUS_CMD;
            g_msvault_id = charToNumber(argv[i+1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetbalanceof") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_BALANCE_OF_CMD;
            g_msvault_id = charToNumber(argv[i+1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetvaultname") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_VAULT_NAME_CMD;
            g_msvault_id = charToNumber(argv[i+1]);
            i+=2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetrevenueinfo") == 0)
        {
            g_cmd = MSVAULT_GET_REVENUE_INFO_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetfees") == 0)
        {
            g_cmd = MSVAULT_GET_FEES_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetvaultowners") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_OWNERS_CMD;
            g_msvault_id = charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultdepositasset") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = MSVAULT_DEPOSIT_ASSET_CMD;
            g_msvault_id = charToNumber(argv[i + 1]);
            g_msVaultAssetName = argv[i + 2];
            g_msVaultIssuer = argv[i + 3];
            g_txAmount = charToNumber(argv[i + 4]);
            i += 5;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultreleaseassetto") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(5)
            g_cmd = MSVAULT_RELEASE_ASSET_TO_CMD;
            g_msvault_id = charToNumber(argv[i + 1]);
            g_msVaultAssetName = argv[i + 2];
            g_msVaultIssuer = argv[i + 3];
            g_txAmount = charToNumber(argv[i + 4]);
            g_msvault_destination = argv[i + 5];
            i += 6;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultresetassetrelease") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_RESET_ASSET_RELEASE_CMD;
            g_msvault_id = charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetassetbalances") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_ASSET_BALANCES_CMD;
            g_msvault_id = charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetassetreleasestatus") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_GET_ASSET_RELEASE_STATUS_CMD;
            g_msvault_id = charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetmanagedassetbalance") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = MSVAULT_GET_MANAGED_ASSET_BALANCE_CMD;
            g_msVaultAssetName = argv[i + 1];
            g_msVaultIssuer = argv[i + 2];
            g_msVaultOwner = argv[i + 3];
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultrevokeassetrights") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = MSVAULT_REVOKE_ASSET_RIGHTS_CMD;
            g_msVaultAssetName = argv[i + 1];
            g_msVaultIssuer = argv[i + 2];
            g_txAmount = charToNumber(argv[i + 3]); // Reusing g_TxAmount for numberOfShares
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultisshareholder") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = MSVAULT_IS_SHAREHOLDER_CMD;
            g_msVaultCandidateIdentity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultvotechange") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(5) // Excluding burn fee for now as it's not currently used
            g_cmd = MSVAULT_VOTE_FEE_CHANGE_CMD;
            g_msVaultNewRegisteringFee = charToUnsignedNumber(argv[i + 1]);
            g_msVaultNewReleaseFee = charToUnsignedNumber(argv[i + 2]);
            g_msVaultNewReleaseResetFee = charToUnsignedNumber(argv[i + 3]);
            g_msVaultNewHoldingFee = charToUnsignedNumber(argv[i + 4]);
            g_msVaultNewDepositFee = charToUnsignedNumber(argv[i + 5]);
            i += 6;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetfeevotes") == 0)
        {
            g_cmd = MSVAULT_GET_FEE_VOTES_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetfeevotesowner") == 0)
        {
            g_cmd = MSVAULT_GET_FEE_VOTES_OWNER_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetfeevotesscore") == 0)
        {
            g_cmd = MSVAULT_GET_FEE_VOTES_SCORE_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetuniquefeevotes") == 0)
        {
            g_cmd = MSVAULT_GET_UNIQUE_FEE_VOTES_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-msvaultgetuniquefeevotesranking") == 0)
        {
            g_cmd = MSVAULT_GET_UNIQUE_FEE_VOTES_RANKING_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }

        /**************************
         **** NOSTROMO COMMANDS ****
         **************************/

        if (strcmp(argv[i], "-nostromoregisterintier") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_REGISTER_IN_TIER;
            g_nost_tierLevel = (uint32_t)charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromologoutfromtier") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(0)
            g_cmd = NOSTROMO_LOGOUT_FROM_TIER;
            i += 1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromocreateproject") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(10)
            g_cmd = NOSTROMO_CREATE_PROJECT;
            g_nost_tokenName = argv[i + 1];
            g_nost_supply = charToNumber(argv[i + 2]);
            g_nost_startYear = (uint32_t)charToNumber(argv[i + 3]);
            g_nost_startMonth = (uint32_t)charToNumber(argv[i + 4]);
            g_nost_startDay = (uint32_t)charToNumber(argv[i + 5]);
            g_nost_startHour = (uint32_t)charToNumber(argv[i + 6]);
            g_nost_endYear = (uint32_t)charToNumber(argv[i + 7]);
            g_nost_endMonth = (uint32_t)charToNumber(argv[i + 8]);
            g_nost_endDay = (uint32_t)charToNumber(argv[i + 9]);
            g_nost_endHour = (uint32_t)charToNumber(argv[i + 10]);
            i += 11;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromovoteinproject") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = NOSTROMO_VOTE_IN_PROJECT;
            g_nost_indexOfProject = (uint32_t)charToNumber(argv[i + 1]);
            g_nost_decision = (bool)charToNumber(argv[i + 2]);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromocreatefundraising") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(43)
            g_cmd = NOSTROMO_CREATE_FUNDRAISING;
            g_nost_tokenPrice = charToNumber(argv[i + 1]);
            g_nost_soldAmount = charToNumber(argv[i + 2]);
            g_nost_requiredFunds = charToNumber(argv[i + 3]);
            g_nost_indexOfProject = (uint32_t)charToNumber(argv[i + 4]);

            g_nost_firstPhaseStartYear = (uint32_t)charToNumber(argv[i + 5]);
            g_nost_firstPhaseStartMonth = (uint32_t)charToNumber(argv[i + 6]);
            g_nost_firstPhaseStartDay = (uint32_t)charToNumber(argv[i + 7]);
            g_nost_firstPhaseStartHour = (uint32_t)charToNumber(argv[i + 8]);
            g_nost_firstPhaseEndYear = (uint32_t)charToNumber(argv[i + 9]);
            g_nost_firstPhaseEndMonth = (uint32_t)charToNumber(argv[i + 10]);
            g_nost_firstPhaseEndDay = (uint32_t)charToNumber(argv[i + 11]);
            g_nost_firstPhaseEndHour = (uint32_t)charToNumber(argv[i + 12]);

            g_nost_secondPhaseStartYear = (uint32_t)charToNumber(argv[i + 13]);
            g_nost_secondPhaseStartMonth = (uint32_t)charToNumber(argv[i + 14]);
            g_nost_secondPhaseStartDay = (uint32_t)charToNumber(argv[i + 15]);
            g_nost_secondPhaseStartHour = (uint32_t)charToNumber(argv[i + 16]);
            g_nost_secondPhaseEndYear = (uint32_t)charToNumber(argv[i + 17]);
            g_nost_secondPhaseEndMonth = (uint32_t)charToNumber(argv[i + 18]);
            g_nost_secondPhaseEndDay = (uint32_t)charToNumber(argv[i + 19]);
            g_nost_secondPhaseEndHour = (uint32_t)charToNumber(argv[i + 20]);

            g_nost_thirdPhaseStartYear = (uint32_t)charToNumber(argv[i + 21]);
            g_nost_thirdPhaseStartMonth = (uint32_t)charToNumber(argv[i + 22]);
            g_nost_thirdPhaseStartDay = (uint32_t)charToNumber(argv[i + 23]);
            g_nost_thirdPhaseStartHour = (uint32_t)charToNumber(argv[i + 24]);
            g_nost_thirdPhaseEndYear = (uint32_t)charToNumber(argv[i + 25]);
            g_nost_thirdPhaseEndMonth = (uint32_t)charToNumber(argv[i + 26]);
            g_nost_thirdPhaseEndDay = (uint32_t)charToNumber(argv[i + 27]);
            g_nost_thirdPhaseEndHour = (uint32_t)charToNumber(argv[i + 28]);

            g_nost_listingStartYear = (uint32_t)charToNumber(argv[i + 29]);
            g_nost_listingStartMonth = (uint32_t)charToNumber(argv[i + 30]);
            g_nost_listingStartDay = (uint32_t)charToNumber(argv[i + 31]);
            g_nost_listingStartHour = (uint32_t)charToNumber(argv[i + 32]);

            g_nost_cliffEndYear = (uint32_t)charToNumber(argv[i + 33]);
            g_nost_cliffEndMonth = (uint32_t)charToNumber(argv[i + 34]);
            g_nost_cliffEndDay = (uint32_t)charToNumber(argv[i + 35]);
            g_nost_cliffEndHour = (uint32_t)charToNumber(argv[i + 36]);

            g_nost_vestingEndYear = (uint32_t)charToNumber(argv[i + 37]);
            g_nost_vestingEndMonth = (uint32_t)charToNumber(argv[i + 38]);
            g_nost_vestingEndDay = (uint32_t)charToNumber(argv[i + 39]);
            g_nost_vestingEndHour = (uint32_t)charToNumber(argv[i + 40]);

            g_nost_threshold = (uint8_t)charToNumber(argv[i + 41]);
            g_nost_TGE = (uint8_t)charToNumber(argv[i + 42]);
            g_nost_stepOfVesting = (uint8_t)charToNumber(argv[i + 43]);
            i += 44;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromoinvestinproject") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = NOSTROMO_INVEST_IN_FUNDRAISING;
            g_nost_indexOfFundraising = (uint32_t)charToNumber(argv[i + 1]);
            g_nost_amount = charToNumber(argv[i + 2]);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromoclaimtoken") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = NOSTROMO_CLAIM_TOKEN;
            g_nost_amount = charToNumber(argv[i + 1]);
            g_nost_indexOfFundraising = (uint32_t)charToNumber(argv[i + 2]);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromoupgradetierlevel") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_UPGRADE_TIER_LEVEL;
            g_nost_tierLevel = (uint32_t)charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromotransfersharemanagementrights") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(4)
            g_cmd = NOSTROMO_TRANSFER_SHARE_MANAGEMENT_RIGHTS;
            g_nost_tokenName = argv[i + 1];
            g_nost_identity = argv[i + 2];
            g_nost_newManagementContractIndex = uint32_t(charToNumber(argv[i + 3]));
            g_nost_numberOfShare = int64_t(charToNumber(argv[i + 4]));
            i += 5;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-nostromogetstats") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(0)
            g_cmd = NOSTROMO_GET_STATS;
            i += 1;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromogettierlevelbyuser") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_GET_TIER_LEVEL_BY_USER;
            g_nost_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromogetuservotestatus") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_GET_USER_VOTE_STATUS;
            g_nost_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromochecktokencreatability") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_CHECK_TOKEN_CREATABILITY;
            g_nost_tokenName = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromogetnumberofinvestedprojects") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_GET_NUMBER_OF_INVESTED_PROJECTS;
            g_nost_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromogetprojectbyindex") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_GET_PROJECT_BY_INDEX;
            g_nost_indexOfProject = (uint32_t)charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromogetfundraisingbyindex") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_GET_FUNDRAISING_BY_INDEX;
            g_nost_indexOfFundraising = (uint32_t)charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromogetprojectindexlistbycreator") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_GET_PROJECT_INDEX_LIST_BY_CREATOR;
            g_nost_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromogetinfouserinvested") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = NOSTROMO_GET_INFO_USER_INVESTED;
            g_nost_identity = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-nostromogetmaxclaimamount") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = NOSTROMO_GET_MAX_CLAIM_AMOUNT;
            g_nost_identity = argv[i + 1];
            g_nost_indexOfFundraising = (uint32_t)charToNumber(argv[i + 2]);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }

        /************************
         **** QBOND COMMANDS ****
         ************************/

        if (strcmp(argv[i], "-qbondstake") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QBOND_STAKE_CMD;
            g_qbond_millionsOfQu = charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondtransfer") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QBOND_TRANSFER_CMD;
            g_qbond_targetIdentity = argv[i + 1];
            g_qbond_epoch = charToNumber(argv[i + 2]);
            g_qbond_mbondsAmount = charToNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondaddask") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QBOND_ADD_ASK_ORDER_CMD;
            g_qbond_epoch = charToNumber(argv[i + 1]);
            g_qbond_mbondPrice = charToNumber(argv[i + 2]);
            g_qbond_mbondsAmount = charToNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondremoveask") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QBOND_REMOVE_ASK_ORDER_CMD;
            g_qbond_epoch = charToNumber(argv[i + 1]);
            g_qbond_mbondPrice = charToNumber(argv[i + 2]);
            g_qbond_mbondsAmount = charToNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondaddbid") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QBOND_ADD_BID_ORDER_CMD;
            g_qbond_epoch = charToNumber(argv[i + 1]);
            g_qbond_mbondPrice = charToNumber(argv[i + 2]);
            g_qbond_mbondsAmount = charToNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondremovebid") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QBOND_REMOVE_BID_ORDER_CMD;
            g_qbond_epoch = charToNumber(argv[i + 1]);
            g_qbond_mbondPrice = charToNumber(argv[i + 2]);
            g_qbond_mbondsAmount = charToNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondburnqu") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QBOND_BURN_QU_CMD;
            g_qbond_burnAmount = charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondupdatecfa") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = QBOND_UPDATE_CFA_CMD;
            g_qbond_targetIdentity = argv[i + 1];
            g_qbond_updateCFAOperation = (bool)charToNumber(argv[i + 2]);
            i += 3;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondgetfees") == 0)
        {
            g_cmd = QBOND_GET_FEES_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondgetearnedfees") == 0)
        {
            g_cmd = QBOND_GET_EARNED_FEES_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondgetinfoperepoch") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QBOND_GET_EPOCH_INFO_CMD;
            g_qbond_epoch = charToNumber(argv[i + 1]);
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondgetorders") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QBOND_GET_ORDERS_CMD;
            g_qbond_epoch = charToNumber(argv[i + 1]);
            g_qbond_asksOffset = charToNumber(argv[i + 2]);
            g_qbond_bidsOffset = charToNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondgetuserorders") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(3)
            g_cmd = QBOND_GET_USER_ORDERS_CMD;
            g_qbond_owner = argv[i + 1];
            g_qbond_asksOffset = charToNumber(argv[i + 2]);
            g_qbond_bidsOffset = charToNumber(argv[i + 3]);
            i += 4;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondtable") == 0)
        {
            g_cmd = QBOND_GET_TABLE_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondgetusermbonds") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = QBOND_GET_USER_MBONDS_CMD;
            g_qbond_owner = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-qbondgetcfa") == 0)
        {
            g_cmd = QBOND_GET_CFA_CMD;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }


        /*****************************************
         ***** SHAREHOLDER PROPOSAL COMMANDS *****
         *****************************************/

        if (strcmp(argv[i], "-setshareholderproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(2)
            g_cmd = SHAREHOLDER_SET_PROPOSAL;
            g_contractIndex = getContractIndex(argv[i + 1], g_enableTestContracts);
            g_proposalString = argv[i + 2];
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-clearshareholderproposal") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = SHAREHOLDER_CLEAR_PROPOSAL;
            g_contractIndex = getContractIndex(argv[i + 1], g_enableTestContracts);
            i += 2;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-getshareholderproposals") == 0)
        {
            CHECK_NUMBER_OF_PARAMETERS(1)
            g_cmd = SHAREHOLDER_GET_PROPOSALS;
            g_contractIndex = getContractIndex(argv[i + 1], g_enableTestContracts);
            if (i + 2 >= argc)
            {
                LOG("ERROR: You need to pass PROPOSAL_INDEX_OR_GROUP! E.g.: 0, \"active\", or \"finished\".");
                exit(1);
            }
            g_proposalString = argv[i + 2];
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-shareholdervote") == 0)
        {
            g_cmd = SHAREHOLDER_VOTE;
            if (i + 3 >= argc)
            {
                LOG("ERROR: You need to pass CONTRACT_INDEX, PROPOSAL_INDEX, and VOTE_VALUE!");
                exit(1);
            }
            g_contractIndex = getContractIndex(argv[i + 1], g_enableTestContracts);
            g_proposalString = argv[i + 2];
            g_voteValueString = argv[i + 3];
            i += 4;
            CHECK_OVER_PARAMETERS;
            break;
        }
        if (strcmp(argv[i], "-getshareholdervotes") == 0)
        {
            g_cmd = SHAREHOLDER_GET_VOTE;
            ++i;
            if (i >= argc)
            {
                LOG("ERROR: You need to pass CONTRACT_INDEX and PROPOSAL_INDEX!");
                exit(1);
            }
            g_contractIndex = getContractIndex(argv[i], g_enableTestContracts);
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
        if (strcmp(argv[i], "-getshareholderresults") == 0)
        {
            g_cmd = SHAREHOLDER_GET_VOTING_RESULTS;
            if (i + 2 >= argc)
            {
                LOG("ERROR: You need to pass CONTRACT_INDEX and PROPOSAL_INDEX!");
                exit(1);
            }
            g_contractIndex = getContractIndex(argv[i + 1], g_enableTestContracts);
            g_proposalString = argv[i + 2];
            i += 3;
            CHECK_OVER_PARAMETERS;
            break;
        }

        /**************************
         **** TESTING COMMANDS ****
         **************************/
        
        if (strcmp(argv[i], "-testqpifunctionsoutput") == 0)
        {
            g_cmd = TEST_QPI_FUNCTIONS_OUTPUT;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-testqpifunctionsoutputpast") == 0)
        {
            g_cmd = TEST_QPI_FUNCTIONS_OUTPUT_PAST;
            i++;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-testgetincomingtransferamounts") == 0)
        {
            g_cmd = TEST_GET_INCOMING_TRANSFER_AMOUNTS;
            g_paramString1 = argv[i + 1];
            i += 2;
            CHECK_OVER_PARAMETERS
            break;
        }
        if (strcmp(argv[i], "-testbidinipothroughcontract") == 0)
        {
            g_cmd = TEST_BID_IN_IPO_THROUGH_CONTRACT;
            g_paramString1 = argv[i + 1];
            g_IPOContractIndex = (uint32_t)charToNumber(argv[i + 2]);
            g_makeIPOBidNumberOfShare = (uint32_t)charToNumber(argv[i + 3]);
            g_makeIPOBidPricePerShare = charToNumber(argv[i + 4]);
            i += 5;
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
