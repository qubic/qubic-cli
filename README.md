# qubic-cli

Qubic Core client

An intermediate tool to communicate to qubic core node.
```
./qubic-cli [basic config] [Command] [command extra parameters]
-help print this message
Basic config:
	-conf <file>
		Specify configuration file. Relative paths will be prefixed by datadir location. See qubic.conf.example.
		Notice: variables in qubic.conf will be overrided by values on parameters.
	-seed <SEED>
		55-char seed private key
	-nodeip <IPv4_ADDRESS>
		IP address of the target node for querying blockchain information (default: 127.0.0.1)
	-nodeport <PORT>
		Port of the target node for querying blockchain information (default: 21841)
	-scheduletick <TICK_OFFSET>
		Offset number of scheduled tick that will perform a transaction (default: 20)
	-force
		Do action although an error has been detected. Currently only implemented for proposals.
Command:
[WALLET COMMAND]
	-showkeys
		Generating identity, pubkey key from private key. Private key must be passed either from params or configuration file.
	-getbalance <IDENTITY>
		Balance of an identity (amount of qubic, number of in/out txs)
	-getasset <IDENTITY>
		Print a list of assets of an identity
	-sendtoaddress <TARGET_IDENTITY> <AMOUNT>
		Perform a standard transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY>. A valid private key and node ip/port are required.
	-sendtoaddressintick <TARGET_IDENTITY> <AMOUNT> <TICK>
		Perform a standard transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY> in a specific <TICK>. A valid private key and node ip/port are required.
	-qutilsendtomanyv1 <FILE>
		Performs multiple transaction within in one tick. <FILE> must contain one ID and amount (space seperated) per line. Max 25 transaction. Fees apply! valid private key and node ip/port are required.
	-qutilburnqubic <AMOUNT>
		Performs burning qubic, valid private key and node ip/port are required.

[BLOCKCHAIN/PROTOCOL COMMAND]
	-gettickdata <TICK_NUMBER> <OUTPUT_FILE_NAME>
		Get tick data and write it to a file. Use -readtickdata to examine the file. valid node ip/port are required.
	-getquorumtick <COMP_LIST_FILE> <TICK_NUMBER>
		Get quorum tick data, the summary of quorum tick will be printed, <COMP_LIST_FILE> is fetched by command -getcomputorlist. valid node ip/port are required.
	-getcomputorlist <OUTPUT_FILE_NAME>
		Get of the current epoch. Feed this data to -readtickdata to verify tick data. valid node ip/port are required.
	-getnodeiplist
		Print a list of node ip from a seed node ip. Valid node ip/port are required.
	-gettxinfo <TX_ID>
		Get tx infomation, will print empty if there is no tx or invalid tx. valid node ip/port are required.
	-checktxontick <TICK_NUMBER> <TX_ID>
		Check if a transaction is included in a tick. valid node ip/port are required.
	-checktxonfile <TX_ID> <TICK_DATA_FILE>
		Check if a transaction is included in a tick (tick data from a file). valid node ip/port are required.
	-readtickdata <FILE_NAME> <COMPUTOR_LIST>
		Read tick data from a file, print the output on screen, COMPUTOR_LIST is required if you need to verify block data
	-sendcustomtransaction <TARGET_IDENTITY> <TX_TYPE> <AMOUNT> <EXTRA_BYTE_SIZE> <EXTRA_BYTE_IN_HEX>
		Perform a custom transaction (IPO, querying smart contract), valid private key and node ip/port are required.
	-dumpspectrumfile <SPECTRUM_BINARY_FILE> <OUTPUT_CSV_FILE>
		Dump spectrum file into csv.
	-dumpuniversefile <UNIVERSE_BINARY_FILE> <OUTPUT_CSV_FILE>
		Dump universe file into csv.
	-dumpcontractfile <CONTRACT_BINARY_FILE> <CONTRACT_ID> <OUTPUT_CSV_FILE>
		Dump contract file into csv. Current supported CONTRACT_ID: 1-QX
	-makeipobid <CONTRACT_INDEX> <NUMBER_OF_SHARE> <PRICE_PER_SHARE>
		Participating IPO (dutch auction). valid private key and node ip/port, CONTRACT_INDEX are required.
	-getipostatus <CONTRACT_INDEX>
		View IPO status. valid node ip/port, CONTRACT_INDEX are required.
	-getsysteminfo
		View Current System Status. Includes initial tick, random mining seed, epoch info.

[NODE COMMAND]
	-getcurrenttick
		Show current tick information of a node
	-sendspecialcommand <COMMAND_IN_NUMBER> 
		Perform a special command to node, valid private key and node ip/port are required.	
	-tooglemainaux <MODE_0> <Mode_1> 
		Remotely toogle Main/Aux mode on node,valid private key and node ip/port are required.	
		<MODE_0> and <MODE_1> value are: MAIN or AUX	
	-setsolutionthreshold <EPOCH> <SOLUTION_THRESHOLD> 
		Remotely set solution threshold for future epoch,valid private key and node ip/port are required.	
	-refreshpeerlist
		(equivalent to F4) Remotely refresh the peer list of node, all current connections will be closed after this command is sent, valid private key and node ip/port are required.	
	-forcenexttick
		(equivalent to F5) Remotely force next tick on node to be empty, valid private key and node ip/port are required.	
	-reissuevote
		(equivalent to F9) Remotely re-issue (re-send) vote on node, valid private key and node ip/port are required.	
	-sendrawpacket <DATA_IN_HEX> <SIZE>
		Send a raw packet to nodeip. Valid node ip/port are required.
	-getlogfromnode <PASSCODE_0> <PASSCODE_1> <PASSCODE_2> <PASSCODE_3>
		Fetch a single log line from the node. Valid node ip/port, passcodes are required.
	-synctime
		Sync node time with local time, valid private key and node ip/port are required. Make sure that your local time is synced (with NTP)!	
	-getminingscoreranking
		Get current mining score ranking. Valid private key and node ip/port are required.	
	-getvotecountertx <COMPUTOR_LIST_FILE> <TICK>
		Get vote counter transaction of a tick: showing how many votes per ID that this tick leader saw from (<TICK>-675-3) to (<TICK>-3) 	

[QX COMMAND]
	-qxgetfee
		Show current Qx fee.
	-qxissueasset <ASSET_NAME> <NUMBER_OF_UNIT> <UNIT_OF_MEASUREMENT> <NUM_DECIMAL>
		Create an asset via Qx contract.
	-qxtransferasset <ASSET_NAME> <ISSUER_IN_HEX> <NEW_OWNER_IDENTITY> <AMOUNT_OF_SHARE>
		Transfer an asset via Qx contract.
	-qxorder add/remove bid/ask [ISSUER (in qubic format)] [ASSET_NAME] [PRICE] [NUMBER_OF_SHARE]
		Set order on Qx.
	-qxgetorder entity/asset bid/ask [ISSUER/ENTITY (in qubic format)] [ASSET_NAME (NULL for requesting entity)] [OFFSET]
		Get orders on Qx

[QTRY COMMAND]
	-qtrygetbasicinfo
		Show qtry basic info from a node.
	-qtryissuebet
		Issue a bet (prompt mode)
	-qtrygetactivebet
		Show all active bet id.
	-qtrygetactivebetbycreator <BET_CREATOR_ID>
		Show all active bet id of an ID.
	-qtrygetbetinfo <BET_ID>
		Get meta information of a bet
	-qtrygetbetdetail <BET_ID> <OPTION_ID>
		Get a list of IDs that bet on <OPTION_ID> of the bet <BET_ID>
	-qtryjoinbet <BET_ID> <NUMBER_OF_BET_SLOT> <AMOUNT_PER_SLOT> <PICKED_OPTION>
		Join a bet
	-qtrypublishresult <BET_ID> <WIN_OPTION>
		(Oracle providers only) publish a result for a bet
	-qtrycancelbet <BET_ID>
		(Game operator only) cancel a bet

[GENERAL QUORUM PROPOSAL COMMANDS]
	-gqmpropsetproposal <PROPOSAL_STRING>
		Set proposal in general quorum proposals contract. May overwrite existing proposal, because each computor can have only one proposal at a time. For success, computor status is needed.
		<PROPOSAL_STRING> is explained if there is a parsing error.
	-gqmpropclearproposal
		Clear own proposal in general quorum proposals contract. For success, computor status is needed.
	-gqmpropgetproposals <PROPOSAL_INDEX_OR_GROUP>
		Get proposal info from general quorum proposals contract.
		Either pass "active" to get proposals that are open for voting in the current epoch, or "finished" to get proposals of previous epochs not overwritten or cleared yet, or a proposal index.
	-gqmpropvote <PROPOSAL_INDEX> <VOTE_VALUE>
		Vote for proposal in general quorum proposals contract.
		<VOTE_VALUE> is the option in range 0 ... N-1 or "none".
	-gqmpropgetvote <PROPOSAL_INDEX> [VOTER_IDENTITY]
		Get vote from general quorum proposals contract. If VOTER_IDENTITY is skipped, identity of seed is used.
	-gqmpropgetresults <PROPOSAL_INDEX>
		Get the current result of a proposal (general quorum proposals contract).
	-gqmpropgetrevdonation
		Get and print table of revenue donations applied after each epoch.

[CCF COMMANDS]
	-ccfsetproposal <PROPOSAL_STRING>
		Set proposal in computor controlled fund (CCF) contract. May overwrite existing proposal, because each seed can have only one proposal at a time. Costs a fee.
		<PROPOSAL_STRING> is explained if there is a parsing error. Only "Transfer|2" (yes/no transfer proposals) are allowed in CCF.
	-ccfclearproposal
		Clear own proposal in CCF contract. Costs a fee.
	-ccfgetproposals <PROPOSAL_INDEX_OR_GROUP>
		Get proposal info from CCF contract.
		Either pass "active" to get proposals that are open for voting in the current epoch, or "finished" to get proposals of previous epochs not overwritten or cleared yet, or a proposal index.
	-ccfvote <PROPOSAL_INDEX> <VOTE_VALUE>
		Cast vote for a proposal in the CCF contract.
		<VOTE_VALUE> is the option in range 0 ... N-1 or "none".
	-ccfgetvote <PROPOSAL_INDEX> [VOTER_IDENTITY]
		Get vote from CCF contract. If VOTER_IDENTITY is skipped, identity of seed is used.
	-ccfgetresults <PROPOSAL_INDEX>
		Get the current result of a CCF proposal.
	-ccflatesttransfers
		Get and print latest transfers of CCF granted by quorum.

[QEARN COMMAND]

	-qearnlock <LOCK_AMOUNT>
		lock the qu to Qearn SC.
	-qearnunlock <UNLOCKING_AMOUNT> <LOCKED_EPOCH>
		unlock the qu from Qearn SC, unlock the amount of <UNLOCKING_AMOUNT> that locked in the epoch <LOCKED_EPOCH>.
	-qearngetlockinfoperepoch <EPOCH>
		Get the info(Total locked amount, Total bonus amount) locked in <EPOCH>.
	-qearngetuserlockedinfo <IDENTITY> <EPOCH>
		Get the locked amount that the user <IDENTITY> locked in the epoch <EPOCH>.
	-qearngetstateofround <EPOCH>
		Get the status(not started, running, ended) of the epoch <EPOCH>.
	-qearngetuserlockstatus <IDENTITY>
		Get the status(binary number) that the user locked for 52 weeks.
	-qearngetunlockingstatus <IDENTITY>
		Get the unlocking history of the user.
```

### BUILD
```
mkdir build;
cd build;
cmake ../;
make;
```


### USAGE
To get current tick of a node:

`./qubic-cli -nodeip 127.0.0.1 -getcurrenttick`

example return:
```
Tick: 10660587
Epoch: 82
Number Of Aligned Votes: 0
Number Of Misaligned Votes: 0
```

Dump publickey, privatekey and identity:
`./qubic-cli -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -showkeys`

example return:
```
Seed: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
Private key: 62506d370a4e9f42720269c0c973a544de0b6559bda46d1d8dd2fcda9fe4fada
Public key: 1f590d03e613bdded38b4c0820ac44615f91af12435980b3ede3c08c315a2544
Identity: BZBQFLLBNCXEMGLOBHUVFTLUPLVCPQUASSILFABOFFBCADQSSUPNWLZBQEXK
```

Send coin:

`./qubic-cli -nodeip 127.0.0.1 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -sendtoaddress AFZPUAIYVPNUYGJRQVLUKOPPVLHAZQTGLYAAUUNBXFTVTAMSBKQBLEIEPCVJ 1`

Send special command to node:

`./qubic-cli -nodeip 127.0.0.1 -seed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa -sendspecialcommand 123`

Dump tick data to a file:

`./qubic-cli -nodeip 127.0.0.1 -gettickdata 10600000 10600000.bin`

Read tick data file:

`./qubic-cli -readtickdata 10600000.bin`

Check tx on tick data file:

`./qubic-cli -checktxonfile TX_HASH 10600000.bin`

Check tx on online:

`./qubic-cli -nodeip 127.0.0.1 -checktxontick 10600000 TX_HASH`

More information, please read the help. `./qubic-cli -help`

#### NOTE: PROPER ACTIONS are needed if you use this tool as a replacement for qubic wallet. Please use it with caution.
