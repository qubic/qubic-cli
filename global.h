#include "defines.h"
COMMAND g_cmd;
char* g_seed = DEFAULT_SEED;
char* g_nodeIp = DEFAULT_NODE_IP;
char* g_targetIdentity = nullptr;
char* g_configFile = nullptr;
char* g_requestedFileName = nullptr;
char* g_requestedTxId  = nullptr;
char* g_requestedIdentity  = nullptr;

int64_t g_TxAmount = 0;
ushort g_TxType = 0;
int g_nodePort = DEFAULT_NODE_PORT;
int g_txExtraDataSize = 0;
int g_rawPacketSize = 0;
int g_requestedSpecialCommand = -1;
uint32_t g_requestedTickNumber = 0;
uint32_t g_offsetScheduledTick = DEFAULT_SCHEDULED_TICK_OFFSET;
uint8_t g_txExtraData[1024] = {0};
uint8_t g_rawPacket[1024] = {0};

