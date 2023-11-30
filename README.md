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
Command:
	-showkeys
		Generating identity, pubkey key from private key. Private key must be passed either from params or configuration file.
	-getcurrenttick
		Show current tick information of a node
	-gettickdata <TICK_NUMBER> <OUTPUT_FILE_NAME>
		Get tick data and write it to a file. Use -readtickdata to examine the file. valid node ip/port are required.
	-getcomputorlist <OUTPUT_FILE_NAME>
		Get of the current epoch. Feed this data to -readtickdata to verify tick data. valid node ip/port are required.
	-getnodeiplist
		Print a list of node ip from a seed node ip. Valid node ip/port are required.
	-checktxontick <TICK_NUMBER> <TX_ID>
		Check if a transaction is included in a tick. valid node ip/port are required.
	-checktxonfile <TX_ID> <TICK_DATA_FILE>
		Check if a transaction is included in a tick (tick data from a file). valid node ip/port are required.
	-readtickdata <FILE_NAME> <COMPUTOR_LIST>
		Read tick data from a file, print the output on screen, COMPUTOR_LIST is required if you need to verify block data
	-getbalance <IDENTITY>
		Balance of an identity (amount of qubic, number of in/out txs)
	-getownedasset <IDENTITY>
		Print OWNED asset of an identity
	-sendtoaddress <TARGET_IDENTITY> <AMOUNT>
		Perform a normal transaction to sendData <AMOUNT> qubic to <TARGET_IDENTITY>. valid private key and node ip/port are required.
	-sendcustomtransaction <TARGET_IDENTITY> <TX_TYPE> <AMOUNT> <EXTRA_BYTE_SIZE> <EXTRA_BYTE_IN_HEX>
		Perform a custom transaction (IPO, querying smart contract), valid private key and node ip/port are required.
	-sendspecialcommand <COMMAND_IN_NUMBER> 
		Perform a special command to node, valid private key and node ip/port are required.	
	-sendrawpacket <DATA_IN_HEX> <SIZE>
		Send a raw packet to nodeip. Valid node ip/port are required.
	-publishproposal 
		(on development)
	-qxtransfershare <POSSESSED_IDENTITY> <NEW_OWNER_IDENTITY> <AMOUNT_OF_SHARE>
		Transfer Qx's shares to new owner. valid private key and node ip/port, POSSESSED_IDENTITY are required.
		(if you set -scheduletick larger than 50000, the tool will be forced to send the tx at that tick)
	-qxissueasset <ASSET_NAME> <NUMBER_OF_UNIT> <UNIT_OF_MEASUREMENT> <NUM_DECIMAL>
		Create an asset via Qx contract.
	-qxtransferasset <ASSET_NAME> <ISSUER_IN_HEX> <POSSESSED_IDENTITY> <NEW_OWNER_IDENTITY> <AMOUNT_OF_SHARE>
		Transfer an asset via Qx contract.
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
