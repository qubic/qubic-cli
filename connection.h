#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>
#include <array>
#include <span>

#define DEFAULT_TIMEOUT_MSEC 1000

namespace
{
    // Custom concept to detect if something behaves like a pointer (raw or smart)
    template <typename T>
    concept IsPointerLike = std::is_pointer_v<T> || requires(T t)
    {
        t.operator->();
    };
}

// Not thread safe
class QubicConnection
{
public:
	QubicConnection(const char* nodeIp, int nodePort, unsigned long timeoutMillisec = DEFAULT_TIMEOUT_MSEC);
	~QubicConnection();

    // Establish connection to mNodePort on node mNodeIp. 
    // May throw std::logic_error.
    void resolveConnection();

    // Receive at most sz bytes and write them to buffer. Return the actual number of received bytes.
    // Should only return less than sz bytes on timeout, closed connection, or error.
    // Throws std::logic_error if sz > buffer.size().
	int receiveData(std::span<uint8_t> buffer, unsigned int sz);

    // Receive an object of type T. Return the actual number of received bytes.
    // Should only return less than sz bytes on timeout, closed connection, or error.
    template <typename T>
    int receiveData(T& obj)
    {
		return receiveData(std::span<uint8_t>(reinterpret_cast<uint8_t*>(&obj), sizeof(T)), sizeof(T));
    }

    // Receive sz bytes and write them to the buffer. Throws std::logic_error if sz bytes cannot be read.
    int receiveAllDataOrThrowException(std::span<uint8_t> buffer, unsigned int sz);

    // Send sz bytes contained in buffer. Throws std::logic_error if sz > buffer.size().
	int sendData(std::span<const uint8_t> buffer, unsigned int sz);

    // Send an object of type T. This template only accepts trivially copyable types that are no ranges or pointers.
    template <typename T>
    requires std::is_trivially_copyable_v<T>
    && (!std::ranges::range<T>)
    && (!IsPointerLike<T>)
    int sendData(const T& obj)
    {
        return sendData(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&obj), sizeof(T)), sizeof(T));
    }

    //void receiveDataAll(std::vector<uint8_t>& buffer);
    void getHandshakeData(std::vector<uint8_t>& buffer);

    // Receive data of type T that is preceeded by a header. Skips data that does not match T.
    // May throw std::logic_error or EndResponseReceived.
    template <typename T> T receivePacketWithHeaderAs();

    // Same as receivePacketWithHeaderAs() but with pre-allocated T. Use this for large T to prevent stack overflow.
    template <typename T> void receivePacketWithHeaderAs(T& result);

    // Receive data of type T without a header. 
    // May throw std::logic_error.
    template <typename T> T receivePacketAs();

    // Receive vector data of Ts where each T is preceeded by a header.
    template <typename T> std::vector<T> getLatestVectorPacketAs();
private:
	char mNodeIp[32];
	int mNodePort;
	int mSocket;
    std::array<uint8_t, 0xFFFFFF> mBuffer;
    std::vector<uint8_t> mHandshakeData; // storing handshake data after open a connection
};

typedef std::shared_ptr<QubicConnection> QCPtr;

static QCPtr make_qc(const char* nodeIp, int nodePort, unsigned long timeoutMsec = DEFAULT_TIMEOUT_MSEC)
{
    return std::make_shared<QubicConnection>(nodeIp, nodePort, timeoutMsec);
}

class EndResponseReceived : public std::runtime_error
{
public:
    explicit EndResponseReceived(const char* message = "Received end response message") : std::runtime_error(message) {}
};

class ConnectionTimeout : public std::runtime_error
{
public:
    explicit ConnectionTimeout(const char* message = "Connection timeout") : std::runtime_error(message) {}
};
