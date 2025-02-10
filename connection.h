#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>

// Not thread safe
class QubicConnection
{
public:
	QubicConnection(const char* nodeIp, int nodePort);
	~QubicConnection();

    // Establish connection to mNodePort on node mNodeIp. 
    // May throw std::logic_error.
    void resolveConnection();

    // Receive at most sz bytes and write them to buffer. Return the actual number of received bytes.
    // Should only return less than sz bytes on timeout, closed connection, or error.
	int receiveData(uint8_t* buffer, int sz);

    // Receive sz bytes and write them to buffer. Throws std::logic_error if sz bytes cannot be read. 
    int receiveAllDataOrThrowException(uint8_t* buffer, int sz);

    // Send sz bytes contained in buffer.
	int sendData(uint8_t* buffer, int sz);

    //void receiveDataAll(std::vector<uint8_t>& buffer);
    void getHandshakeData(std::vector<uint8_t>& buffer);

    // Receive data of type T that is preceeded by a header. 
    // May throw std::logic_error.
    template <typename T> T receivePacketWithHeaderAs();

    // Receive data of type T without a header. 
    // May throw std::logic_error.
    template <typename T> T receivePacketAs();

    // Receive vector data of Ts where each T is preceeded by a header.
    template <typename T> std::vector<T> getLatestVectorPacketAs();
private:
	char mNodeIp[32];
	int mNodePort;
	int mSocket;
    uint8_t mBuffer[0xFFFFFF];
    std::vector<uint8_t> mHandshakeData; // storing handshake data after open a connection
};

typedef std::shared_ptr<QubicConnection> QCPtr;

static QCPtr make_qc(const char* nodeIp, int nodePort)
{
    return std::make_shared<QubicConnection>(nodeIp, nodePort);
}

class EndResponseReceived : public std::runtime_error
{
public:
    explicit EndResponseReceived(const char* message = "Received end response message") : std::runtime_error(message) {}
};
