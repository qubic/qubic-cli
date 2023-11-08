#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "connection.h"
#include "logger.h"
static int connect(const char* nodeIp, int nodePort)
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
    sockaddr_in addr;
    explicit_bzero((char *)&addr, sizeof(addr));
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
	return recv(mSocket, buffer, sz, 0);
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

int QubicConnection::sendData(uint8_t* buffer, int sz)
{
    int size = sz;
    int numberOfBytes;
    while (size) {
        if ((numberOfBytes = send(mSocket, buffer, size, 0)) <= 0) {
            return 0;
        }
        buffer += numberOfBytes;
        size -= numberOfBytes;
    }
	return sz - size;
}