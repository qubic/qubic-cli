#include "structs.h"
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <Ws2tcpip.h>
#define close(x) closesocket(x)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <cstring>

#include "connection.h"
#include "logger.h"
#ifdef _MSC_VER
static int connect(const char* nodeIp, int nodePort)
{
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 0), &wsa_data);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    size_t tv = 1000;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
    sockaddr_in addr;
    memset((char*)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(nodePort);

    if (inet_pton(AF_INET, nodeIp, &addr.sin_addr) <= 0) {
        LOG("Error translating command line ip address to usable one.");
        return -1;
    }
    int res = connect(serverSocket, (const sockaddr*)&addr, sizeof(addr));
    if (res < 0) {
        LOG("Failed to connect %s | error %d\n", nodeIp, res);
        return -1;
    }
    return serverSocket;
}
#else
static int connect(const char* nodeIp, int nodePort)
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
    setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&tv, sizeof tv);
    sockaddr_in addr;
    memset((char*)&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(nodePort);

    if (inet_pton(AF_INET, nodeIp, &addr.sin_addr) <= 0) {
        LOG("Error translating command line ip address to usable one.");
        return -1;
    }

    if (connect(serverSocket, (const sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG("Failed to connect %s\n", nodeIp);
        return -1;
    }
    return serverSocket;
}
#endif
QubicConnection::QubicConnection(const char* nodeIp, int nodePort)
{
	memset(mNodeIp, 0, 32);
	memcpy(mNodeIp, nodeIp, strlen(nodeIp));
	mNodePort = nodePort;
	mSocket = connect(nodeIp, nodePort);
}
QubicConnection::~QubicConnection()
{
	close(mSocket);
}

int QubicConnection::receiveData(uint8_t* buffer, int sz)
{
	return recv(mSocket, (char*)buffer, sz, 0);
}
void QubicConnection::receiveDataAll(std::vector<uint8_t>& receivedData)
{
    receivedData.resize(0);
    uint8_t tmp[1024];
    int recvByte = receiveData(tmp, 1024);
    while (recvByte > 0)
    {
        receivedData.resize(recvByte + receivedData.size());
        memcpy(receivedData.data() + receivedData.size() - recvByte, tmp, recvByte);
        recvByte = receiveData(tmp, 1024);
    }
}

template <typename T>
T QubicConnection::receivePacketAs()
{
    std::vector<uint8_t> receivedData;
    receivedData.resize(0);
    uint8_t tmp[1024];
    int recvByte = receiveData(tmp, 1024);
    while (recvByte > 0)
    {
        receivedData.resize(recvByte + receivedData.size());
        memcpy(receivedData.data() + receivedData.size() - recvByte, tmp, recvByte);
        recvByte = receiveData(tmp, 1024);
    }

    recvByte = receivedData.size();
    uint8_t* data = receivedData.data();
    int ptr = 0;
    T result;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == T::type()){
            auto curTickData = (T*)(data + ptr + sizeof(RequestResponseHeader));
            result = *curTickData;
        }
        ptr+= header->size();
    }
    return result;
}

int QubicConnection::sendData(uint8_t* buffer, int sz)
{
    int size = sz;
    int numberOfBytes;
    while (size) {
        if ((numberOfBytes = send(mSocket, (char*)buffer, size, 0)) <= 0) {
            return 0;
        }
        buffer += numberOfBytes;
        size -= numberOfBytes;
    }
	return sz - size;
}

template SpecialCommand QubicConnection::receivePacketAs<SpecialCommand>();
template SpecialCommandToggleMainModeResquestAndResponse QubicConnection::receivePacketAs<SpecialCommandToggleMainModeResquestAndResponse>();
template SpecialCommandSetSolutionThresholdResquestAndResponse QubicConnection::receivePacketAs<SpecialCommandSetSolutionThresholdResquestAndResponse>();