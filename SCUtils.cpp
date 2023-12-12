#include <cstring>
#include "SCUtils.h"
#include "structs.h"
#include "connection.h"
#include "logger.h"

void getQxFees(const char* nodeIp, const int nodePort, Fees_output& result){
    auto qc = new QubicConnection(nodeIp, nodePort);
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
    } packet;
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = 0;
    packet.rcf.inputType = 1;
    packet.rcf.contractIndex = 1;
    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            auto fees = (Fees_output*)(data + ptr + sizeof(RequestResponseHeader));
            result = *fees;
        }
        ptr+= header->size();
    }
    delete qc;
}

void printQxFee(const char* nodeIp, const int nodePort){
    Fees_output result;
    getQxFees(nodeIp, nodePort, result);
    LOG("Asset issuance fee: %u\n", result.assetIssuanceFee);
    LOG("Transfer fee: %u\n", result.transferFee);
    LOG("Trade fee: %u\n", result.tradeFee);
}