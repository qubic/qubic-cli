#include <cstring>
#include <iostream>
#include <string>

#include "SCUtils.h"
#include "keyUtils.h"
#include "structs.h"
#include "qxStruct.h"
#include "connection.h"
#include "logger.h"

void dumpQxContractToCSV(const char* input, const char* output)
{
    std::cout << "Dumping QX contract file " << input << std::endl;

    // Check the file size
    FILE* f;
    {
        f = fopen(input, "rb"); // Open the file in binary mode
        if (f == nullptr)
        {
            std::cout << "Can not open file! Exit. " << output << std::endl;
            return;
        }

        fseek(f, 0, SEEK_END);
        unsigned long fileSize = (unsigned long)ftell(f);
        fclose(f);

        if (fileSize != sizeof(QX))
        {
            std::cout << "File size is different from QX state size! " << fileSize << " . Expected " << sizeof(QX) << std::endl;
            return;
        }
    }

    std::shared_ptr<QX> qxState = std::make_shared<QX>();

    f = fopen(input, "rb");
    fread(qxState.get(), 1, sizeof(QX), f);
    fclose(f);

    f = fopen(output, "w");
    {
       std::string header ="EarnedAmount,DistributedAmount,BurnedAmount,AssetIssuanceFee,TransferFee,TradeFee,Entity,Issuer,AssetName,NumberOfShares,Price\n";
       fwrite(header.c_str(), 1, header.size(), f);
    }
    std::string stringLine = std::to_string(qxState->_earnedAmount)
                             + "," + std::to_string(qxState->_distributedAmount)
                             + "," + std::to_string(qxState->_burnedAmount)
                             + "," + std::to_string(qxState->_assetIssuanceFee)
                             + "," + std::to_string(qxState->_transferFee)
                             + "," + std::to_string(qxState->_tradeFee);

    int64_t elementIdx = 0;
    uint8_t povID[32];
    char buffer[128];
    char assetNameBuffer[8];
    for (int64_t elementIdx = 0; elementIdx < qxState->_entityOrders.population(); elementIdx++)
    {
        if (elementIdx > 0)
        {
            stringLine = ",,,,,";
        }
        // Write data
        // Entity
        qxState->_entityOrders.pov(elementIdx, povID);

        memset(buffer, 0, 128);
        getIdentityFromPublicKey(povID, buffer, false);
        stringLine = stringLine + "," + buffer;

        // Extract data
        auto entityOrder = qxState->_entityOrders.element(elementIdx);

        // Issuer
        memset(buffer, 0, 128);
        getIdentityFromPublicKey(entityOrder.issuer, buffer, false);
        stringLine = stringLine + "," + buffer;

        // Asset name
        memcpy(assetNameBuffer, (char*)&entityOrder.assetName, 8);
        std::string assetName = assetNameBuffer;
        stringLine = stringLine + "," + assetName;

        // Number of share
        stringLine = stringLine + "," + std::to_string(entityOrder.numberOfShares);

        // Price
        int64_t price = qxState->_entityOrders.priority(elementIdx);
        stringLine = stringLine + "," + std::to_string(price);

        stringLine += "\n";

        fwrite(stringLine.c_str(), 1, stringLine.size(), f);
    }

    fclose(f);

    std::cout << "File is written into " << output << std::endl;
}

void dumpQtryContractToCSV(const char* input, const char* output)
{
    std::cout << "Dumping Qtry contract file " << input << std::endl;
}

void dumpContractToCSV(const char* input, uint32_t contractId, const char* output)
{
    // Checking the contract type
    std::string fileName = input;
    auto extensionLocation = fileName.rfind('.');
    switch (contractId)
    {
    case SCType::SC_TYPE_QX :
        dumpQxContractToCSV(input, output);
        break;
    default:
        std::cout << "Unsupported contract id: " << contractId << std::endl;
        break;
    }
}