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
#include <stdexcept>

#include "connection.h"
#include "logger.h"
#include "structs.h"

// includes for template instantiations
#include "quottery.h"
#include "qxStruct.h"
#include "qvault.h"
#include "qearn.h"
#include "msvault.h"

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
    // If node has no ComputorList or a self-generated ComputorList it will requestComputor upon tcp initialization
    // Ignore this message if it is here
    try{
        *((RequestComputors*)data) = receivePacketWithHeaderAs<RequestComputors>();
    }
    catch(std::logic_error& e){
    }
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
T QubicConnection::receivePacketWithHeaderAs()
{
    // first receive the header
    RequestResponseHeader header;
    int recvByte = receiveData((uint8_t*)&header, sizeof(RequestResponseHeader));
    if (recvByte != sizeof(RequestResponseHeader))
    {
        throw std::logic_error("No connection.");
    }
    if (header.type() != T::type())
    {
        throw std::logic_error("Unexpected header type: " + std::to_string(header.type()) + " (expected: " + std::to_string(T::type()) + ").");
    }
    
    int packetSize = header.size();
    int remainingSize = packetSize - sizeof(RequestResponseHeader);
    T result;
    memset(&result, 0, sizeof(T));
    if (remainingSize)
    {
        memset(mBuffer, 0, sizeof(T));
        // receive the rest, allow 5 tries because sometimes not all requested bytes are received
        int recvByteTotal = 0;
        for (int i = 0; i < 5; ++i)
        {
            if (remainingSize > 4096)
                recvByte = receiveDataBig(mBuffer + recvByteTotal, remainingSize);
            else
                recvByte = receiveData(mBuffer + recvByteTotal, remainingSize);
            recvByteTotal += recvByte;
            remainingSize -= recvByte;
            if (!remainingSize)
                break;
        }
        if (remainingSize)
        {
            throw std::logic_error("Unexpected data size: missing " + std::to_string(remainingSize) + " bytes, expected a total of " + std::to_string(packetSize) + " bytes (incl. header).");
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
std::vector<T> QubicConnection::getLatestVectorPacketAs()
{
    std::vector<T> results;
    while (true)
    {
        try
        {
            results.push_back(receivePacketWithHeaderAs<T>());
        }
        catch (std::logic_error& e)
        {
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

template SpecialCommand QubicConnection::receivePacketWithHeaderAs<SpecialCommand>();
template SpecialCommandToggleMainModeResquestAndResponse QubicConnection::receivePacketWithHeaderAs<SpecialCommandToggleMainModeResquestAndResponse>();
template SpecialCommandSetSolutionThresholdResquestAndResponse QubicConnection::receivePacketWithHeaderAs<SpecialCommandSetSolutionThresholdResquestAndResponse>();
template SpecialCommandSendTime QubicConnection::receivePacketWithHeaderAs<SpecialCommandSendTime>();
template GetSendToManyV1Fee_output QubicConnection::receivePacketWithHeaderAs<GetSendToManyV1Fee_output>();
template CurrentTickInfo QubicConnection::receivePacketWithHeaderAs<CurrentTickInfo>();
template CurrentSystemInfo QubicConnection::receivePacketWithHeaderAs<CurrentSystemInfo>();
template TickData QubicConnection::receivePacketWithHeaderAs<TickData>();
template RespondTxStatus QubicConnection::receivePacketWithHeaderAs<RespondTxStatus>();
template BroadcastComputors QubicConnection::receivePacketWithHeaderAs<BroadcastComputors>();
template RespondContractIPO QubicConnection::receivePacketWithHeaderAs<RespondContractIPO>();
// QUOTTERY
template qtryBasicInfo_output QubicConnection::receivePacketWithHeaderAs<qtryBasicInfo_output>();
template getBetInfo_output QubicConnection::receivePacketWithHeaderAs<getBetInfo_output>();
template getBetOptionDetail_output QubicConnection::receivePacketWithHeaderAs<getBetOptionDetail_output>();
template getActiveBet_output QubicConnection::receivePacketWithHeaderAs<getActiveBet_output>();
template getActiveBetByCreator_output QubicConnection::receivePacketWithHeaderAs<getActiveBetByCreator_output>();
// QX
template QxFees_output QubicConnection::receivePacketWithHeaderAs<QxFees_output>();
template qxGetAssetOrder_output QubicConnection::receivePacketWithHeaderAs<qxGetAssetOrder_output>();
template qxGetEntityOrder_output QubicConnection::receivePacketWithHeaderAs<qxGetEntityOrder_output>();
// QVAULT
template QVaultGetData_output QubicConnection::receivePacketWithHeaderAs<QVaultGetData_output>();
// QEARN
template QEarnGetLockInfoPerEpoch_output QubicConnection::receivePacketWithHeaderAs<QEarnGetLockInfoPerEpoch_output>();
template QEarnGetUserLockedInfo_output QubicConnection::receivePacketWithHeaderAs<QEarnGetUserLockedInfo_output>();
template QEarnGetStateOfRound_output QubicConnection::receivePacketWithHeaderAs<QEarnGetStateOfRound_output>();
template QEarnGetUserLockStatus_output QubicConnection::receivePacketWithHeaderAs<QEarnGetUserLockStatus_output>();
template QEarnGetStatsPerEpoch_output QubicConnection::receivePacketWithHeaderAs<QEarnGetStatsPerEpoch_output>();
template QEarnGetEndedStatus_output QubicConnection::receivePacketWithHeaderAs<QEarnGetEndedStatus_output>();
template QEarnGetBurnedAndBoostedStats_output QubicConnection::receivePacketWithHeaderAs<QEarnGetBurnedAndBoostedStats_output>();
template QEarnGetBurnedAndBoostedStatsPerEpoch_output QubicConnection::receivePacketWithHeaderAs<QEarnGetBurnedAndBoostedStatsPerEpoch_output>();

template ExchangePublicPeers QubicConnection::receivePacketAs<ExchangePublicPeers>();

template std::vector<Tick> QubicConnection::getLatestVectorPacketAs<Tick>();
template std::vector<RespondOwnedAssets> QubicConnection::getLatestVectorPacketAs<RespondOwnedAssets>();
template std::vector<RespondPossessedAssets> QubicConnection::getLatestVectorPacketAs<RespondPossessedAssets>();

// MSVAULT
template MsVaultGetBalanceOf_output QubicConnection::receivePacketWithHeaderAs<MsVaultGetBalanceOf_output>();
template MsVaultGetReleaseStatus_output QubicConnection::receivePacketWithHeaderAs<MsVaultGetReleaseStatus_output>();
template MsVaultGetVaults_output QubicConnection::receivePacketWithHeaderAs<MsVaultGetVaults_output>();
template MsVaultGetVaultName_output QubicConnection::receivePacketWithHeaderAs<MsVaultGetVaultName_output>();
template MsVaultGetRevenueInfo_output QubicConnection::receivePacketWithHeaderAs<MsVaultGetRevenueInfo_output>();
template MsVaultGetFees_output QubicConnection::receivePacketWithHeaderAs<MsVaultGetFees_output>();
template MsVaultGetVaultOwners_output QubicConnection::receivePacketWithHeaderAs<MsVaultGetVaultOwners_output>();
