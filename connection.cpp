#include "structs.h"
#include <stdexcept>
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
    tv.tv_sec = 2;
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
    if (mSocket < 0)
        throw std::logic_error("No connection.");
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
    if (receivedData.size() == 0)
    {
        throw std::logic_error("Error: Did not receive any response from node.");
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
    memset(&result, 0, sizeof(T));
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == T::type()){
            auto dataT = (T*)(data + ptr + sizeof(RequestResponseHeader));
            result = *dataT;
        }
        ptr+= header->size();
    }
    return result;
}

template <typename T>
std::vector<T> QubicConnection::getLatestVectorPacketAs()
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
    std::vector<T> results;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == T::type()){
            auto dataT = (T*)(data + ptr + sizeof(RequestResponseHeader));
            results.push_back(*dataT);
        }
        ptr+= header->size();
    }
    return results;
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
template SpecialCommandSendTime QubicConnection::receivePacketAs<SpecialCommandSendTime>();
template GetSendToManyV1Fee_output QubicConnection::receivePacketAs<GetSendToManyV1Fee_output>();

template std::vector<Tick> QubicConnection::getLatestVectorPacketAs<Tick>();