#pragma once

#include "defines.h"

COMMAND g_cmd;
char* g_seed = (char*)DEFAULT_SEED;
char* g_nodeIp = (char*)DEFAULT_NODE_IP;
char* g_targetIdentity = nullptr;
char* g_configFile = nullptr;
char* g_requestedFileName = nullptr;
char* g_requestedFileName2 = nullptr;
char* g_requestedTxId  = nullptr;
char* g_requestedIdentity  = nullptr;
char* g_qx_share_transfer_possessed_identity = nullptr;
char* g_qx_share_transfer_new_owner_identity = nullptr;
int64_t g_qx_share_transfer_amount = 0;
const char* g_proposalString = "";
const char* g_voteValueString = "";
const char* g_paramString1 = "";
const char* g_paramString2 = "";
bool g_force = false;

int64_t g_TxAmount = 0;
uint16_t g_TxType = 0;
uint32_t g_TxTick = 0;
int g_nodePort = DEFAULT_NODE_PORT;
int g_txExtraDataSize = 0;
int g_rawPacketSize = 0;
int g_requestedSpecialCommand = -1;
char* g_toggle_main_aux_0 = nullptr;
char* g_toggle_main_aux_1 = nullptr;
int g_set_solution_threshold_epoch = -1;
int g_set_solution_threshold_value = -1;
char* g_file_path = nullptr;
char* g_compress_tool = nullptr;
uint32_t g_contract_index = 0;
char g_loggingMode = 0;
char* g_compChatString = nullptr;


uint32_t g_requestedTickNumber = 0;
uint32_t g_offsetScheduledTick = DEFAULT_SCHEDULED_TICK_OFFSET;
int g_waitUntilFinish = 0;
uint8_t g_txExtraData[1024] = {0};
uint8_t g_rawPacket[1024] = {0};

char* g_qx_issue_asset_name = nullptr;
char* g_qx_issue_unit_of_measurement = nullptr;
int64_t g_qx_issue_asset_number_of_unit = -1;
char g_qx_issue_asset_num_decimal = 0;

char* g_qx_command_1 = nullptr;
char* g_qx_command_2 = nullptr;
char* g_qx_issuer = nullptr;
char* g_qx_asset_name = nullptr;
long long g_qx_offset = -1;
long long g_qx_price = -1;
long long g_qx_number_of_share = -1;

char* g_qx_asset_transfer_possessed_identity = nullptr;
char* g_qx_asset_transfer_new_owner_identity = nullptr;
int64_t g_qx_asset_transfer_amount = -1;
char* g_qx_asset_transfer_asset_name = nullptr;
char* g_qx_asset_transfer_issuer_in_hex = nullptr;

char* g_dump_binary_file_input = nullptr;
char* g_dump_binary_file_output = nullptr;
uint32_t g_dump_binary_contract_id = 0;

// IPO bid
uint32_t g_ipo_contract_index = 0;
uint16_t g_make_ipo_bid_number_of_share = 0;
uint64_t g_make_ipo_bid_price_per_share = 0;
// quottery
uint32_t g_quottery_bet_id = 0;
uint32_t g_quottery_option_id = 0;
char* g_quottery_creator_id = nullptr;
uint64_t g_quottery_number_bet_slot = 0;
uint64_t g_quottery_amount_per_bet_slot = 0;
uint32_t g_quottery_picked_option = 0;

// qutil
char* g_qutil_sendtomanyv1_payout_list_file = nullptr;
int64_t g_qutil_sendtomanybenchmark_destination_count = 0;
int64_t g_qutil_sendtomanybenchmark_num_transfers_each = 0;

// qearn
uint64_t g_qearn_lock_amount = 0;
uint64_t g_qearn_unlock_amount = 0;
uint32_t g_qearn_locked_epoch = 0;
uint32_t g_qearn_getinfo_epoch = 0;
uint32_t g_qearn_getstats_epoch = 0;

// qvault
char* g_qvaultIdentity = nullptr;
uint32_t g_qvault_numberOfChangedAddress = 0;
uint32_t g_qvault_newQCAPHolder_fee = 0;
uint32_t g_qvault_newreinvesting_fee = 0;
uint32_t g_qvault_newdev_fee = 0;

// msvault
uint64_t g_msVaultID = 0;
uint64_t g_msVaultRequiredApprovals = 0;
uint8_t g_msVaultVaultName[32] = { 0 };
char* g_msVaultDestination = nullptr;
char* g_msVaultPublicId = nullptr;
char* g_msVaultOwnersCommaSeparated = nullptr;

// nostromo

char* g_nostTokenName = nullptr;
uint64_t g_nostSupply = 0;
uint32_t g_nost_tierlevel = 0;
uint32_t g_noststartyear = 0;
uint32_t g_noststartmonth = 0;
uint32_t g_noststartday = 0;
uint32_t g_noststarthour = 0;
uint32_t g_nostendyear = 0;
uint32_t g_nostendmonth = 0;
uint32_t g_nostendday = 0;
uint32_t g_nostendhour = 0;
uint32_t g_nostindexofproject = 0;
bool g_nostdecsion = 0;
uint64_t g_nost_tokenPrice;
uint64_t g_nost_soldAmount;
uint64_t g_nost_requiredFunds;

uint32_t g_nost_indexOfProject = 0;
uint32_t g_nost_firstPhaseStartYear;
uint32_t g_nost_firstPhaseStartMonth;
uint32_t g_nost_firstPhaseStartDay;
uint32_t g_nost_firstPhaseStartHour;
uint32_t g_nost_firstPhaseEndYear;
uint32_t g_nost_firstPhaseEndMonth;
uint32_t g_nost_firstPhaseEndDay;
uint32_t g_nost_firstPhaseEndHour;

uint32_t g_nost_secondPhaseStartYear;
uint32_t g_nost_secondPhaseStartMonth;
uint32_t g_nost_secondPhaseStartDay;
uint32_t g_nost_secondPhaseStartHour;
uint32_t g_nost_secondPhaseEndYear;
uint32_t g_nost_secondPhaseEndMonth;
uint32_t g_nost_secondPhaseEndDay;
uint32_t g_nost_secondPhaseEndHour;

uint32_t g_nost_thirdPhaseStartYear;
uint32_t g_nost_thirdPhaseStartMonth;
uint32_t g_nost_thirdPhaseStartDay;
uint32_t g_nost_thirdPhaseStartHour;
uint32_t g_nost_thirdPhaseEndYear;
uint32_t g_nost_thirdPhaseEndMonth;
uint32_t g_nost_thirdPhaseEndDay;
uint32_t g_nost_thirdPhaseEndHour;

uint32_t g_nost_listingStartYear;
uint32_t g_nost_listingStartMonth;
uint32_t g_nost_listingStartDay;
uint32_t g_nost_listingStartHour;

uint32_t g_nost_cliffEndYear;
uint32_t g_nost_cliffEndMonth;
uint32_t g_nost_cliffEndDay;
uint32_t g_nost_cliffEndHour;

uint32_t g_nost_vestingEndYear;
uint32_t g_nost_vestingEndMonth;
uint32_t g_nost_vestingEndDay;
uint32_t g_nost_vestingEndHour;

uint8_t g_nost_threshold;
uint8_t g_nost_TGE;
uint8_t g_nost_stepOfVesting;

uint64_t g_nost_amount = 0;
uint32_t g_nost_indexOfFundaraising;

char* g_nost_identity = nullptr;