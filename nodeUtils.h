#pragma once
void printTickInfoFromNode(const char* nodeIp, int nodePort);
uint32_t getTickNumberFromNode(const char* nodeIp, int nodePort);
bool checkTxOnTick(const char* nodeIp, const int nodePort, const char* txHash, uint32_t requestedTick);
void getTickDataToFile(const char* nodeIp, const int nodePort, uint32_t requestedTick, const char* fileName);
void printTickDataFromFile(const char* fileName);
bool checkTxOnFile(const char* txHash, const char* fileName);
void sendRawPacket(const char* nodeIp, const int nodePort, int rawPacketSize, uint8_t* rawPacket);
void sendSpecialCommand(const char* nodeIp, const int nodePort, const char* seed, int command);
void getComputorListToFile(const char* nodeIp, const int nodePort, const char* fileName);