#pragma once
#include <cstdint>
#include <vector>
#include <memory>
// Not thread safe
class QubicConnection
{
public:
	QubicConnection(const char* nodeIp, int nodePort);
	~QubicConnection();	
	int receiveData(uint8_t* buffer, int sz);
	int sendData(uint8_t* buffer, int sz);
    void receiveDataAll(std::vector<uint8_t>& buffer);
    template <typename T> T receivePacketAs();
    template <typename T> std::vector<T> getLatestVectorPacketAs();
private:
	char mNodeIp[32];
	int mNodePort;
	int mSocket;
};
typedef std::shared_ptr<QubicConnection> QCPtr;
static QCPtr make_qc(const char* nodeIp, int nodePort)
{
    return std::make_shared<QubicConnection>(nodeIp, nodePort);
}