#include "defines.h"
COMMAND g_cmd;
char* g_seed = DEFAULT_SEED;
char* g_nodeIp = DEFAULT_NODE_IP;
char* g_targetIdentity = nullptr;
char* g_configFile = nullptr;
char* g_requestedFileName = nullptr;
char* g_requestedFileName2 = nullptr;
char* g_requestedTxId  = nullptr;
char* g_requestedIdentity  = nullptr;
char* g_qx_share_transfer_possessed_identity = nullptr;
char* g_qx_share_transfer_new_owner_identity = nullptr;
int64_t g_qx_share_transfer_amount = 0;

int64_t g_TxAmount = 0;
uint16_t g_TxType = 0;
int g_nodePort = DEFAULT_NODE_PORT;
int g_txExtraDataSize = 0;
int g_rawPacketSize = 0;
int g_requestedSpecialCommand = -1;

uint32_t g_requestedTickNumber = 0;
uint32_t g_offsetScheduledTick = DEFAULT_SCHEDULED_TICK_OFFSET;
uint8_t g_txExtraData[1024] = {0};
uint8_t g_rawPacket[1024] = {0};

char* g_qx_issue_asset_name = nullptr;
char* g_qx_issue_unit_of_measurement = nullptr;
int64_t g_qx_issue_asset_number_of_unit = -1;
char g_qx_issue_asset_num_decimal = 0;

char* g_qx_asset_transfer_possessed_identity = nullptr;
char* g_qx_asset_transfer_new_owner_identity = nullptr;
int64_t g_qx_asset_transfer_amount = -1;
char* g_qx_asset_transfer_asset_name;
char* g_qx_asset_transfer_issuer_in_hex;

char* g_dump_binary_file_input;
char* g_dump_binary_file_output;

//IPO bid
uint32_t g_ipo_contract_index = 0;
uint16_t g_make_ipo_bid_number_of_share = 0;
uint64_t g_make_ipo_bid_price_per_share = 0;


uint64_t g_get_log_passcode[4] = {0};