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
#include <string>

#include "connection.h"
#include "logger.h"

// includes for template instantiations
#include "quottery.h"
#include "qxStruct.h"

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

    if (inet_pton(AF_INET, nodeIp, &addr.sin_addr) <= 0) 
    {
        LOG("Error translating command line ip address to usable one.");
        return -1;
    }
    int res = connect(serverSocket, (const sockaddr*)&addr, sizeof(addr));
    if (res < 0) 
    {
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

    if (inet_pton(AF_INET, nodeIp, &addr.sin_addr) <= 0) 
    {
        LOG("Error translating command line ip address to usable one.");
        return -1;
    }

    if (connect(serverSocket, (const sockaddr *)&addr, sizeof(addr)) < 0) 
    {
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
        throw std::logic_error("Unable to establish connection.");

    // receive handshake - exchange peer packets
    mHandshakeData.resize(sizeof(ExchangePublicPeers));
    uint8_t* data = mHandshakeData.data();
    *((ExchangePublicPeers*)data) = receivePacketWithHeaderAs<ExchangePublicPeers>();   
}
void QubicConnection::getHandshakeData(std::vector<uint8_t>& buffer)
{
    buffer = mHandshakeData;
}
QubicConnection::~QubicConnection()
{
	close(mSocket);
}

int QubicConnection::receiveData(uint8_t* buffer, int sz)
{
    if (sz > 4096)
    {
        LOG("Warning: trying to receive too big chunk, consider using receiveDataBig instead\n");
    }
	return recv(mSocket, (char*)buffer, sz, 0);
}

int QubicConnection::receiveDataBig(uint8_t* buffer, int sz)
{
    int count = 0;
    while (sz)
    {
        int chunk = (std::min)(sz, 1024);
        int recvByte = receiveData(buffer + count, chunk);
        count += recvByte;
        sz -= recvByte;
    }
    return count;
}

void QubicConnection::resolveConnection()
{
    mSocket = connect(mNodeIp, mNodePort);
    if (mSocket < 0)
        throw std::logic_error("Unable to establish connection.");
}

// Receive the next qubic packet with a RequestResponseHeader
template <typename T>
T QubicConnection::receivePacketWithHeaderAs(bool skipOtherHeaders)
{
    // first receive the header
    RequestResponseHeader header;
    int recvByte = receiveData((uint8_t*)&header, sizeof(RequestResponseHeader));
    int remainingSize;
    while (true)
    {
        if (recvByte != sizeof(RequestResponseHeader))
        {
            throw std::logic_error("No connection.");
        }
        if (header.type() != T::type())
        {
            if (skipOtherHeaders)
            {
                remainingSize = header.size() - sizeof(RequestResponseHeader);
                if (remainingSize)
                {
                    if (remainingSize > 4096)
                    {
                        receiveDataBig(mBuffer, remainingSize);
                    }
                    else
                    {
                        receiveData(mBuffer, remainingSize);
                    }
                }
                // receive next header
                recvByte = receiveData((uint8_t*)&header, sizeof(RequestResponseHeader));
            }
            else
            {
                throw std::logic_error("Unexpected header type: " + std::to_string(header.type()) + " (expected: " + std::to_string(T::type()) + ").");
            }
        }
        else
        {
            break;
        }
    }
    int packetSize = header.size();
    remainingSize = packetSize - sizeof(RequestResponseHeader);
    T result;
    memset(&result, 0, sizeof(T));
    if (remainingSize)
    {
        memset(mBuffer, 0, remainingSize);
        // receive the rest
        if (remainingSize > 4096)
        {
            recvByte = receiveDataBig(mBuffer, remainingSize);
        }
        else
        {
            recvByte = receiveData(mBuffer, remainingSize);
        }
        if (recvByte != remainingSize)
        {
            throw std::logic_error("Unexpected data size.");
        }
        result = *((T*)mBuffer);
    }
    return result;
}

// same as receivePacketWithHeaderAs but without the header
template <typename T>
T QubicConnection::receivePacketAs()
{
    int packetSize = sizeof(T);
    T result;
    memset(&result, 0, sizeof(T));
    int recvByte = receiveData(mBuffer, packetSize);
    if (recvByte != packetSize)
    {
        throw std::logic_error("Unexpected data size.");
    }
    result = *((T*)mBuffer);
    return result;
}

template <typename T>
std::vector<T> QubicConnection::getLatestVectorPacketAs(bool skipOtherHeaders)
{
    std::vector<T> results;
    while (true)
    {
        try
        {
            results.push_back(receivePacketWithHeaderAs<T>(skipOtherHeaders));
        }
        catch (std::logic_error& e)
        {
            LOG(("getLatestVectorPacketAs ended after receiving " + std::to_string(results.size()) + " elements. ").c_str());
            LOG(e.what());
            break;
        }
    }
    return results;
}

int QubicConnection::sendData(uint8_t* buffer, int sz)
{
    int size = sz;
    int numberOfBytes;
    while (size) 
    {
        if ((numberOfBytes = send(mSocket, (char*)buffer, size, 0)) <= 0) 
        {
            return 0;
        }
        buffer += numberOfBytes;
        size -= numberOfBytes;
    }
	return sz - size;
}

template SpecialCommand QubicConnection::receivePacketWithHeaderAs<SpecialCommand>(bool skipOtherHeaders);
template SpecialCommandToggleMainModeResquestAndResponse QubicConnection::receivePacketWithHeaderAs<SpecialCommandToggleMainModeResquestAndResponse>(bool skipOtherHeaders);
template SpecialCommandSetSolutionThresholdResquestAndResponse QubicConnection::receivePacketWithHeaderAs<SpecialCommandSetSolutionThresholdResquestAndResponse>(bool skipOtherHeaders);
template SpecialCommandSendTime QubicConnection::receivePacketWithHeaderAs<SpecialCommandSendTime>(bool skipOtherHeaders);
template GetSendToManyV1Fee_output QubicConnection::receivePacketWithHeaderAs<GetSendToManyV1Fee_output>(bool skipOtherHeaders);
template CurrentTickInfo QubicConnection::receivePacketWithHeaderAs<CurrentTickInfo>(bool skipOtherHeaders);
template CurrentSystemInfo QubicConnection::receivePacketWithHeaderAs<CurrentSystemInfo>(bool skipOtherHeaders);
template TickData QubicConnection::receivePacketWithHeaderAs<TickData>(bool skipOtherHeaders);
template RespondTxStatus QubicConnection::receivePacketWithHeaderAs<RespondTxStatus>(bool skipOtherHeaders);
template BroadcastComputors QubicConnection::receivePacketWithHeaderAs<BroadcastComputors>(bool skipOtherHeaders);
template RespondContractIPO QubicConnection::receivePacketWithHeaderAs<RespondContractIPO>(bool skipOtherHeaders);
template qtryBasicInfo_output QubicConnection::receivePacketWithHeaderAs<qtryBasicInfo_output>(bool skipOtherHeaders);
template getBetInfo_output QubicConnection::receivePacketWithHeaderAs<getBetInfo_output>(bool skipOtherHeaders);
template getBetOptionDetail_output QubicConnection::receivePacketWithHeaderAs<getBetOptionDetail_output>(bool skipOtherHeaders);
template getActiveBet_output QubicConnection::receivePacketWithHeaderAs<getActiveBet_output>(bool skipOtherHeaders);
template getActiveBetByCreator_output QubicConnection::receivePacketWithHeaderAs<getActiveBetByCreator_output>(bool skipOtherHeaders);
template QxFees_output QubicConnection::receivePacketWithHeaderAs<QxFees_output>(bool skipOtherHeaders);
template qxGetAssetOrder_output QubicConnection::receivePacketWithHeaderAs<qxGetAssetOrder_output>(bool skipOtherHeaders);
template qxGetEntityOrder_output QubicConnection::receivePacketWithHeaderAs<qxGetEntityOrder_output>(bool skipOtherHeaders);

template ExchangePublicPeers QubicConnection::receivePacketAs<ExchangePublicPeers>();

template std::vector<Tick> QubicConnection::getLatestVectorPacketAs<Tick>(bool skipOtherHeaders);
template std::vector<RespondOwnedAssets> QubicConnection::getLatestVectorPacketAs<RespondOwnedAssets>(bool skipOtherHeaders);
template std::vector<RespondPossessedAssets> QubicConnection::getLatestVectorPacketAs<RespondPossessedAssets>(bool skipOtherHeaders);