#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>

static void sanityCheckTxType(ushort type)
{
    if (type > 1)
    {
        LOG("unknown input type %u\n", type);
        exit(1);
    }
}

static void sanityCheckSeed(char* privKey)
{
	if (privKey == NULL)
	{
		LOG("privKey is null\n");
		exit(1);
	}
    if (strlen(privKey) != 55){
        LOG("Seed must be 55-char length\n");
        exit(1);
    }
}
static bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}
static void sanityCheckNode(char* ip, int port)
{
	if (port < 1024)
	{
		LOG("invalid port number\n");
		exit(1);
	}
	if (ip == NULL)
	{
		LOG("invalid ipv4 address\n");
		exit(1);
	}
	if (!isValidIpAddress(ip))
	{
		LOG("invalid ipv4 address: %s\n", ip);
		exit(1);		
	}
}
static void sanityCheckIdentity(char* identity)
{
	if (identity == NULL){
		LOG("identity is null\n");
		exit(1);
	}
	for (int i = 0; i < 60; i++){
		if (identity[i] < 'A' || identity[i] > 'Z'){
			LOG("invalid identity at position %d (%c)", i, identity[i]);
			exit(1);
		}
	}
}
static void sanityCheckTxHash(char* txHash)
{
    if (txHash == NULL){
        LOG("identity is null\n");
        exit(1);
    }
    for (int i = 0; i < 60; i++){
        if (txHash[i] < 'a' || txHash[i] > 'z'){
            LOG("invalid identity at position %d (%c)", i, txHash[i]);
            exit(1);
        }
    }
}
static void sanityCheckTxAmount(int64_t amount)
{
	if (amount < 0){
		LOG("invalid amount %lld\n", amount);
		exit(1);
	}
}
static void sanityCheckExtraDataSize(int data_size)
{
	if (data_size > 1024){
		LOG("Invalid data size %d, data size must be smaller than 1024\n", data_size);
		exit(1);
	}
}
static void sanityCheckRawPacketSize(int data_size)
{
	if (data_size > 1024){
		LOG("Invalid data size %d, data size must be smaller than 1024\n", data_size);
		exit(1);
	}
}

static void sanityFileExist (const std::string name) {
    std::ifstream f(name.c_str());
    if (!f.good())
    {
        LOG("File %s does not exist\n", name.c_str());
        exit(1);
    }
}

static void sanityCheckSpecialCommand(int cmd)
{
    if (cmd == -1)
    {
        LOG("Invalid special command");
        exit(1);
    }
}