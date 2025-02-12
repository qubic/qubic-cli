#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <chrono>
#include <memory>
#include <thread>
#include <stdexcept>

#include "structs.h"
#include "connection.h"
#include "nodeUtils.h"
#include "logger.h"
#include "K12AndKeyUtil.h"
#include "keyUtils.h"
#include "walletUtils.h"
#include <fstream>
#include <fcntl.h>

constexpr int fullFragmentSize = 1024 - FileFragmentTransactionPrefix::minInputSize();
struct fullFragment{
    RequestResponseHeader header;
    FileFragmentTransactionPrefix fragmentHeader;
    uint8_t content[fullFragmentSize];
};

static std::string getExtension(std::string fn)
{
    return fn.substr(fn.find_last_of(".") + 1);
}

bool uploadHeader(QCPtr& qc, const char* seed, size_t fileSize, int numberOfFragments, std::string extension, uint8_t* txHash, const uint32_t scheduledTickOffset)
{
    struct {
        RequestResponseHeader header;
        FileHeaderTransaction fh;
    } payload;
    FileHeaderTransaction& file_header = payload.fh;

    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t txTick = currentTick + scheduledTickOffset;

    getPublicKeyFromSeed(seed, file_header.sourcePublicKey);
    memset(file_header.destinationPublicKey, 0, 32);
    file_header.amount = FileHeaderTransaction::minAmount();
    file_header.tick = txTick;
    file_header.fileSize = fileSize;
    file_header.numberOfFragments = numberOfFragments;
    memset(file_header.fileFormat, 0, 8);
    memcpy(file_header.fileFormat, extension.data(), extension.size());
    file_header.inputType = FileHeaderTransaction::transactionType();
    file_header.inputSize = FileHeaderTransaction::minInputSize();

    payload.header.setSize(sizeof(payload));
    payload.header.zeroDejavu();
    payload.header.setType(BROADCAST_TRANSACTION);

    signData(seed, (uint8_t*)&payload.fh, sizeof(payload.fh) - SIGNATURE_SIZE, payload.fh.signature);
    qc->sendData((uint8_t *) &payload, payload.header.size());

    KangarooTwelve((uint8_t*)&payload.fh, sizeof(payload.fh), txHash, 32);
    LOG("Waiting for tx to be included at tick %d\n", txTick);
    currentTick = getTickNumberFromNode(qc);
    while (currentTick < txTick + 1)
    {
        LOG("Current tick %u\n", currentTick);
        Q_SLEEP(3000);
        currentTick = getTickNumberFromNode(qc);
    }
    LOG("Verifying transaction\n");
    char txHashQubic[64] = {0};
    getIdentityFromPublicKey(txHash, txHashQubic, true);
    int ret = _GetTxInfo(qc, txHashQubic);
    while (ret == 2)
    {
        ret = _GetTxInfo(qc, txHashQubic);
        Q_SLEEP(1000);
    }
    if (ret == 0)
    {
        LOG("Transaction %s is included\n", txHashQubic);
        return true;
    }
    else
    {
        LOG("Transaction %s is NOT included.\n", txHashQubic);
        memset(txHash, 0, 32);
        return false;
    }
}

bool uploadFragment(QCPtr& qc, const char* seed, const uint64_t fragmentId,
                    const uint8_t* fragmentData, const size_t fragmentSize,
                    const uint8_t* prevFragmentTxHash,
                    uint8_t* outTxHash,
                    const uint32_t scheduledTickOffset)
{
#pragma pack(push, 1) // this is important
    struct {
        RequestResponseHeader header;
        FileFragmentTransactionPrefix fftp;
        uint8_t fragmentPostfix[fullFragmentSize + SIGNATURE_SIZE];
    } payload; // be aware: not always submit full payload
#pragma pack(pop)
    auto& fftp = payload.fftp;
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t txTick = currentTick + scheduledTickOffset;

    getPublicKeyFromSeed(seed, fftp.sourcePublicKey);
    memset(fftp.destinationPublicKey, 0, 32);
    fftp.amount = FileFragmentTransactionPrefix::minAmount();
    fftp.tick = txTick;
    fftp.fragmentIndex = fragmentId;
    memcpy(fftp.prevFileFragmentTransactionDigest, prevFragmentTxHash, 32);
    memcpy(payload.fragmentPostfix, fragmentData, fragmentSize);
    fftp.inputType = FileFragmentTransactionPrefix::transactionType();
    fftp.inputSize = FileFragmentTransactionPrefix::minInputSize() + fragmentSize;
    uint8_t* ptr_signature =  payload.fragmentPostfix + fragmentSize;
    size_t payloadSize = sizeof(RequestResponseHeader) + sizeof(FileFragmentTransactionPrefix) + fragmentSize + SIGNATURE_SIZE; // not send full in trailer
    payload.header.setSize(payloadSize);
    payload.header.zeroDejavu();
    payload.header.setType(BROADCAST_TRANSACTION);
    signData(seed, (uint8_t*)&payload.fftp, sizeof(FileFragmentTransactionPrefix) + fragmentSize, ptr_signature);

    qc->sendData((uint8_t *) &payload, payload.header.size());
    KangarooTwelve((uint8_t*)&payload.fftp, sizeof(FileFragmentTransactionPrefix) + fragmentSize + SIGNATURE_SIZE, outTxHash, 32);
    LOG("Waiting for tx to be included at tick %d\n", txTick);
    currentTick = getTickNumberFromNode(qc);
    while (currentTick < txTick + 1)
    {
        LOG("Current tick %u\n", currentTick);
        Q_SLEEP(3000);
        currentTick = getTickNumberFromNode(qc);
    }
    LOG("Verifying transaction\n");
    char txHashQubic[64] = {0};
    getIdentityFromPublicKey(outTxHash, txHashQubic, true);
    int ret = _GetTxInfo(qc, txHashQubic);
    while (ret == 2)
    {
        ret = _GetTxInfo(qc, txHashQubic);
        Q_SLEEP(1000);
    }
    if (ret == 0)
    {
        LOG("Transaction %s is included\n", txHashQubic);
        return true;
    }
    else
    {
        LOG("Transaction %s is NOT included.\n", txHashQubic);
        memset(outTxHash, 0, 32);
        return false;
    }
}

std::string compressFileWithTool(const char* inputFile, const char* tool)
{
    // Cut off the extension if there is any
    std::string filePathStr = inputFile;
    std::string compressFileName;
    size_t lastDot = filePathStr.find_last_of('.');
    if (lastDot != std::string::npos)
    {
        compressFileName = filePathStr.substr(0, lastDot);
    }

    // Add extension depend on the commmand
    std::string commandWithArgument;
    std::string commandStr = tool;
    if (commandStr.find_last_of("zip") != std::string::npos)
    {
        compressFileName = compressFileName + ".zip";
        commandWithArgument = "zip -9 " + compressFileName + " " + filePathStr;
    }
    else if (commandStr.find_last_of("tar") != std::string::npos)
    {
        compressFileName = compressFileName + ".tar.gz";
        commandWithArgument = "tar -czvf " + compressFileName + " " + filePathStr;
    }

    LOG("Execute command: %s.\n", commandWithArgument.c_str());
    int sts = system(commandWithArgument.c_str());
    if (sts != 0)
    {
        LOG("Execute command FAILED: %.\n");
        return "";
    }
    return compressFileName;
}

int decompressFileWithTool(const char* inputFile, const char* tool)
{
    // Cut off the extension if there is any
    std::string decompressFileName = inputFile;

    // Add extension based on the command
    std::string commandWithArgument;
    std::string commandStr = tool;
    if (commandStr.find("zip") != std::string::npos)
    {
        commandWithArgument = "unzip " + decompressFileName;
    }
    else if (commandStr.find("tar") != std::string::npos)
    {
        commandWithArgument = "tar -xzvf " + decompressFileName;
    }

    LOG("Execute command: %s.\n", commandWithArgument.c_str());
    int sts = system(commandWithArgument.c_str());
    if (sts != 0)
    {
        LOG("Execute command FAILED: %d.\n", sts);
        return 1;
    }
    return 0;
}

void uploadFile(const char* nodeIp, const int nodePort, const char* filePath, const char* seed, const uint32_t scheduledTickOffset, const char* compressTool)
{
    std::string filePathStr = filePath;
    // Run the compression if there is any provided
    if (compressTool)
    {
        filePathStr = compressFileWithTool(filePath, compressTool);
    }

    // Start to upload the file
    size_t fileSize = 0;
    std::string extension;
    std::ifstream in(filePathStr, std::ifstream::ate | std::ifstream::binary);
    fileSize = in.tellg();
    extension = getExtension(filePathStr);
    if (fileSize == 0)
    {
        LOG("File size is 0. Exit.\n");
        return;
    }
    if (extension.empty())
    {
        LOG("Invalid extension. Exit\n");
        return;
    }

    std::vector<fullFragment> file_fragments;
    int numberOfFragments = (fileSize + fullFragmentSize - 1) / fullFragmentSize;
    auto qc = make_qc(nodeIp, nodePort);
    std::vector<uint8_t> fragmentData;
    fragmentData.resize(fileSize);
    FILE* f = fopen(filePathStr.c_str(), "rb");
    fread(fragmentData.data(), 1, fileSize, f);
    fclose(f);

    TxHash32Struct fileHeaderTxDigest;
    LOG("Uploading file header...\n");
    while (!uploadHeader(qc, seed, fileSize, numberOfFragments, extension, fileHeaderTxDigest.ptr, scheduledTickOffset))
    {
        LOG("Failed to upload file header, retry in 3 seconds\n");
        Q_SLEEP(3000);
    }
    std::vector<TxHash32Struct> fragmentTxDigests;
    fragmentTxDigests.resize(numberOfFragments);
    std::vector<int> segmentStart, segmentSize;
    segmentStart.resize(numberOfFragments);
    segmentSize.resize(numberOfFragments);
    size_t tmp = fileSize;

    segmentStart[0] = 0;
    segmentSize[0] = std::min(int(fullFragmentSize), int(tmp));
    tmp -= segmentSize[0];
    for (int i = 1; i < numberOfFragments; i++)
    {
        segmentStart[i] = segmentStart[i-1] + segmentSize[i-1];
        segmentSize[i] = std::min(int(fullFragmentSize), int(tmp));
        tmp -= segmentSize[i];
    }
    LOG("Uploading fragments...\n");
    {
        int i = 0;
        while (!uploadFragment(qc, seed, i, fragmentData.data() + segmentStart[i], segmentSize[i], fileHeaderTxDigest.ptr, fragmentTxDigests[i].ptr, scheduledTickOffset))
        {
            LOG("Failed to upload fragment %d, retry in 3 seconds\n", i);
            Q_SLEEP(3000);
        }
    }
    for (int i = 1; i < numberOfFragments; i++)
    {
        LOG("Uploading fragment #%d\n", i);
        while (!uploadFragment(qc, seed, i, fragmentData.data() + segmentStart[i], segmentSize[i], fragmentTxDigests[i-1].ptr, fragmentTxDigests[i].ptr, scheduledTickOffset))
        {
            LOG("Failed to upload fragment %d, retry in 3 seconds\n", i);
            Q_SLEEP(3000);
        }
    }
    LOG("Successfully uploaded file. List of all hashes:\n");
    char qubicHash[64] = {0};
    getIdentityFromPublicKey(fileHeaderTxDigest.ptr, qubicHash, true);
    LOG("File header: %s\n", qubicHash);
    for (int i = 0; i < numberOfFragments; i++)
    {
        getIdentityFromPublicKey(fragmentTxDigests[i].ptr, qubicHash, true);
        if (i != numberOfFragments - 1)
        {
            LOG("Fragment #%d: %s\n", i, qubicHash);
        }
        else
        {
            LOG("Trailer: %s\n", qubicHash);
        }
    }
}

void downloadFile(const char* nodeIp, const int nodePort, const char* trailer, const char* outFilePath, const char* decompressTool)
{
    std::vector<uint8_t> fileData;
    uint8_t buffer[1024];
    auto qc = make_qc(nodeIp, nodePort);
    int dataSize = 0;
    int ret = _GetInputDataFromTxHash(qc, trailer, buffer, dataSize);
    while (ret == 2)
    {
        ret = _GetInputDataFromTxHash(qc, trailer, buffer, dataSize);
        Q_SLEEP(1000);
    }
    if (ret == 1)
    {
        LOG("Cannot find trailer %s is included\n", trailer);
        return;
    }
    int contentSize = dataSize - 40;
    fileData.resize(contentSize);
    memcpy(fileData.data(), buffer + 40, contentSize);

    uint64_t nFragment = ((uint64_t*)buffer)[0] + 1;
    LOG("Number of fragment: %d\n", nFragment);
    LOG("Downloaded fragment #%lld\n", nFragment - 1);
    char nextTxHash[64] = {0};
    getIdentityFromPublicKey(buffer + 8, nextTxHash, true);

    for (int i = 0; i < nFragment - 1; i++)
    {
        ret = _GetInputDataFromTxHash(qc, nextTxHash, buffer, dataSize);
        while (ret == 2)
        {
            ret = _GetInputDataFromTxHash(qc, nextTxHash, buffer, dataSize);
            Q_SLEEP(1000);
        }
        if (dataSize)
        {
            if (dataSize < 40)
            {
                LOG("Malformed data size, please check this tx hash %s\n", nextTxHash);
            }
            uint64_t fragmentId = ((uint64_t*)(buffer))[0];
            LOG("Downloaded fragment #%lld\n", fragmentId);
            getIdentityFromPublicKey(buffer + 8, nextTxHash, true);
            std::vector<uint8_t> tmp;
            contentSize = dataSize - 40;
            tmp.resize(fileData.size() + contentSize);
            memcpy(tmp.data(), buffer + 40, contentSize);
            memcpy(tmp.data() + contentSize, fileData.data(), fileData.size());
            std::swap(fileData, tmp);
        }
    }
    ret = _GetInputDataFromTxHash(qc, nextTxHash, buffer, dataSize);
    while (ret == 2)
    {
        ret = _GetInputDataFromTxHash(qc, nextTxHash, buffer, dataSize);
        Q_SLEEP(1000);
    }
    LOG("Downloaded header\n");
    uint64_t fileSize = ((uint64_t*)buffer)[0];
    uint64_t numberOfFragments = ((uint64_t*)(buffer+8))[0];
    char fileFormat[8] = {0};
    memcpy(fileFormat, buffer + 16, 8);
    if (fileSize != fileData.size())
    {
        LOG("mismatched file size. Header tell %d | have %d\n", fileSize, fileData.size());
        return;
    }
    if (numberOfFragments != nFragment)
    {
        LOG("mismatched number of fragment. Header tell %d | have %d\n", numberOfFragments, nFragment);
        return;
    }
    LOG("File extension: %s\n", fileFormat);
    std::string outPath = std::string(outFilePath) + "." + std::string(fileFormat);
    FILE* f = fopen(outPath.c_str(), "wb");
    fwrite(fileData.data(), 1, fileData.size(), f);
    fclose(f);
    LOG("Data have been written to %s\n", outPath.c_str());

    // Run the decompression if there is any provided
    if (decompressTool)
    {
        decompressFileWithTool(outPath.c_str(), decompressTool);
    }

}
