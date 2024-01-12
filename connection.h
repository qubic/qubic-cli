#pragma once
#include <cstdint>
#include <vector>

// Not thread safe
class QubicConnection
{
public:
	QubicConnection(const char* nodeIp, int nodePort);
	~QubicConnection();	
	int receiveData(uint8_t* buffer, int sz);
	int sendData(uint8_t* buffer, int sz);
    void receiveDataAll(std::vector<uint8_t>& buffer);
	int error;
private:
	char mNodeIp[32];
	int mNodePort;
	int mSocket;
};
