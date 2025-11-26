#include <algorithm>
#include <cstdint>
#include <cstring>

#include "wallet_utils.h"
#include "key_utils.h"
#include "logger.h"
#include "qvault.h"

#define QVAULT_CONTRACT_INDEX 10
#define QVAULT_PROPOSAL_CREATION_FEE 10000000
#define QVAULT_SHARE_MANAGEMENT_TRANSFER_FEE 100

constexpr uint32_t QVAULT_MAX_URLS_COUNT = 256;

// QVAULT FUNCTIONS
#define QVAULT_GETDATA 1
#define QVAULT_GET_STAKED_AMOUNT_AND_VOTING_POWER 2
#define QVAULT_GET_GP 3
#define QVAULT_GET_QCP 4
#define QVAULT_GET_IPOP 5
#define QVAULT_GET_QEARNP 6
#define QVAULT_GET_FUNDP 7
#define QVAULT_GET_MKTP 8
#define QVAULT_GET_ALLOP 9
#define QVAULT_GET_IDENTITIES_HV_VT_PW 11
#define QVAULT_GET_PP_CREATION_POWER 12
#define QVAULT_GET_QCAP_BURNT_AMOUNT_IN_LAST_EPOCHES 13
#define QVAULT_GET_AMOUNT_TO_BE_SOLD_PER_YEAR 14
#define QVAULT_GET_TOTAL_REVENUE_IN_QCAP 15
#define QVAULT_GET_REVENUE_IN_QCAP_PER_EPOCH 16
#define QVAULT_GET_REVENUE_PER_SHARE 17
#define QVAULT_GET_AMOUNT_OF_SHARE_QVAULT_HOLD 18
#define QVAULT_GET_NUMBER_OF_HOLDER_AND_AVG_AM 19
#define QVAULT_GET_AMOUNT_FOR_QEARN_IN_UPCOMING_EPOCH 20

// QVAULT PROCEDURES
#define QVAULT_STAKE 1
#define QVAULT_UNSTAKE 2
#define QVAULT_SUBMIT_GP 3
#define QVAULT_SUBMIT_QCP 4
#define QVAULT_SUBMIT_IPOP 5
#define QVAULT_SUBMIT_QEARNP 6
#define QVAULT_SUBMIT_FUNDP 7
#define QVAULT_SUBMIT_MKTP 8
#define QVAULT_SUBMIT_ALLOP 9
#define QVAULT_VOTE_IN_PROPOSAL 11
#define QVAULT_BUY_QCAP 12
#define QVAULT_TRANSFER_SHARE_MANAGEMENT_RIGHTS 13

struct stake_input
{
    uint32_t amount;
};

struct stake_output
{
    int32_t returnCode;
};

struct unStake_input
{
    uint32_t amount;
};

struct unStake_output
{
    int32_t returnCode;
};

struct submitGP_input
{
    uint8_t url[256];
};

struct submitGP_output
{
    int32_t returnCode;
};

struct submitQCP_input
{
    uint32_t newQuorumPercent;
    uint8_t url[256];
};

struct submitQCP_output
{
    int32_t returnCode;
};

struct submitIPOP_input
{
    uint32_t ipoContractIndex;
    uint8_t url[256];
};

struct submitIPOP_output
{
    int32_t returnCode;
};

struct submitQEarnP_input
{
    uint64_t amountPerEpoch;
    uint32_t numberOfEpoch;
    uint8_t url[256];
};

struct submitQEarnP_output
{
    int32_t returnCode;
};

struct submitFundP_input
{
    uint64_t priceOfOneQcap;
    uint32_t amountOfQcap;
    uint8_t url[256];
};

struct submitFundP_output
{
    int32_t returnCode;
};

struct submitMKTP_input
{
    uint64_t amountOfQubic;
    uint64_t shareName;
    uint32_t amountOfQcap;
    uint32_t indexOfShare;
    uint32_t amountOfShare;
    uint8_t url[256];
};

struct submitMKTP_output
{
    int32_t returnCode;
};

struct submitAlloP_input
{
    uint32_t reinvested;
    uint32_t burn;
    uint32_t distribute;
    uint8_t url[256];
};

struct submitAlloP_output
{
    int32_t returnCode;
};

struct voteInProposal_input
{
    uint64_t priceOfIPO;
    uint32_t proposalType;
    uint32_t proposalId;
    bool yes;
};

struct voteInProposal_output
{
    int32_t returnCode;
};

struct buyQcap_input
{
    uint32_t amount;
};

struct buyQcap_output
{
    int32_t returnCode;
};

struct TransferShareManagementRights_input
{
    qpiAsset asset;
    int64_t numberOfShares;
    uint32_t newManagingContractIndex;
};
struct TransferShareManagementRights_output
{
    int64_t transferredNumberOfShares;
    int32_t returnCode;
};

namespace
{

bool copyUrlField(uint8_t* dest, const char* url)
{
    if (!dest || !url)
        return false;
    size_t len = strlen(url);
    if (len > 255)
        return false;
    memset(dest, 0, 256);
    memcpy(dest, url, len);
    return true;
}

bool encodeAssetName(const char* source, uint64_t& encoded)
{
    if (!source)
        return false;
    size_t len = strlen(source);
    if (len > 8)
        return false;
    char buffer[8] = {0};
    memcpy(buffer, source, len);
    memcpy(&encoded, buffer, sizeof(buffer));
    return true;
}

bool runQvaultFunction(const char* nodeIp, int nodePort,
                       unsigned short funcNumber,
                       void* inputPtr, size_t inputSize,
                       void* outputPtr, size_t outputSize)
{
    if (!runContractFunction(nodeIp, nodePort, QVAULT_CONTRACT_INDEX,
                             funcNumber, inputPtr, inputSize,
                             outputPtr, outputSize))
    {
        LOG("Failed to receive data\n");
        return false;
    }
    return true;
}

constexpr const char* EMPTY_IDENTITY = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFXIB";

} // namespace

void stake(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount)
{
    stake_input input{};
    input.amount = amount;
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_STAKE,
                            0,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void unStake(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount)
{
    unStake_input input{};
    input.amount = amount;
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_UNSTAKE,
                            0,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void submitGP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* url)
{
    submitGP_input input{};
    if (!copyUrlField(input.url, url))
    {
        printf("The url should be less than 255.\nThe command is failed");
        return;
    }
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_SUBMIT_GP,
                            QVAULT_PROPOSAL_CREATION_FEE,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void submitQCP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t newQuorumPercent, const char* url)
{
    submitQCP_input input{};
    input.newQuorumPercent = newQuorumPercent;
    if (!copyUrlField(input.url, url))
    {
        printf("The url should be less than 255.\nThe command is failed");
        return;
    }
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_SUBMIT_QCP,
                            QVAULT_PROPOSAL_CREATION_FEE,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void submitIPOP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t ipoContractIndex, const char* url)
{
    submitIPOP_input input{};
    input.ipoContractIndex = ipoContractIndex;
    if (!copyUrlField(input.url, url))
    {
        printf("The url should be less than 255.\nThe command is failed");
        return;
    }
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_SUBMIT_IPOP,
                            QVAULT_PROPOSAL_CREATION_FEE,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void submitQEarnP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t amountPerEpoch, uint32_t numberOfEpoch, const char* url)
{
    submitQEarnP_input input{};
    input.amountPerEpoch = amountPerEpoch;
    input.numberOfEpoch = numberOfEpoch;
    if (!copyUrlField(input.url, url))
    {
        printf("The url should be less than 255.\nThe command is failed");
        return;
    }
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_SUBMIT_QEARNP,
                            QVAULT_PROPOSAL_CREATION_FEE,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void submitFundP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t priceOfOneQcap, uint32_t amountOfQcap, const char* url)
{
    submitFundP_input input{};
    input.amountOfQcap = amountOfQcap;
    input.priceOfOneQcap = priceOfOneQcap;
    if (!copyUrlField(input.url, url))
    {
        printf("The url should be less than 255.\nThe command is failed");
        return;
    }
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_SUBMIT_FUNDP,
                            QVAULT_PROPOSAL_CREATION_FEE,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void submitMKTP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t amountOfQubic, const char* shareName, uint32_t amountOfQcap, uint32_t indexOfShare, uint32_t amountOfShare, const char* url)
{
    submitMKTP_input input{};
    if (!encodeAssetName(shareName, input.shareName))
    {
        printf("The share name should be at most 8 characters.\n");
        return;
    }
    if (!copyUrlField(input.url, url))
    {
        printf("The url should be less than 255.\nThe command is failed");
        return;
    }
    input.amountOfQcap = amountOfQcap;
    input.amountOfQubic = amountOfQubic;
    input.amountOfShare = amountOfShare;
    input.indexOfShare = indexOfShare;

    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_SUBMIT_MKTP,
                            QVAULT_PROPOSAL_CREATION_FEE,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void submitAlloP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t reinvested, uint32_t burn, uint32_t distribute, const char* url)
{
    submitAlloP_input input{};
    input.reinvested = reinvested;
    input.burn = burn;
    input.distribute = distribute;
    if (!copyUrlField(input.url, url))
    {
        printf("The url should be less than 255.\nThe command is failed");
        return;
    }
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_SUBMIT_ALLOP,
                            QVAULT_PROPOSAL_CREATION_FEE,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void voteInProposal(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t priceOfIPO, uint32_t proposalType, uint32_t proposalId, bool yes)
{
    voteInProposal_input input{};
    input.priceOfIPO = priceOfIPO;
    input.proposalType = proposalType;
    input.proposalId = proposalId;
    input.yes = yes;
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_VOTE_IN_PROPOSAL,
                            0,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void buyQcap(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount, uint64_t priceOfOneQcap)
{
    buyQcap_input input{};
    input.amount = amount;
    const uint64_t txAmount = priceOfOneQcap * amount;
    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_BUY_QCAP,
                            txAmount,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void TransferShareManagementRights(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* issuer, const char* assetName, int64_t numberOfShares, uint32_t newManagingContractIndex)
{
    uint64_t encodedAssetName = 0;
    if (!encodeAssetName(assetName, encodedAssetName))
    {
        printf("The asset name should be at most 8 characters.\n");
        return;
    }
    uint8_t pubKey[32] = {0};
    getPublicKeyFromIdentity(issuer, pubKey);

    TransferShareManagementRights_input input{};
    memcpy(input.asset.issuer, pubKey, sizeof(input.asset.issuer));
    input.asset.assetName = encodedAssetName;
    input.numberOfShares = numberOfShares;
    input.newManagingContractIndex = newManagingContractIndex;

    makeContractTransaction(nodeIp,
                            nodePort,
                            seed,
                            QVAULT_CONTRACT_INDEX,
                            QVAULT_TRANSFER_SHARE_MANAGEMENT_RIGHTS,
                            QVAULT_SHARE_MANAGEMENT_TRANSFER_FEE,
                            sizeof(input),
                            reinterpret_cast<const uint8_t*>(&input),
                            scheduledTickOffset);
}

void getData(const char* nodeIp, int nodePort)
{
    QVaultGetData_output result{};
    if (!runQvaultFunction(nodeIp, nodePort, QVAULT_GETDATA, nullptr, 0, &result, sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    printf("returnCode: %d\ntotalVotingPower: %llu\nproposalCreateFund: %llu\nreinvestingFund: %llu\ntotalEpochRevenue: %llu\nfundForBurn: %llu\ntotalStakedQcapAmount: %llu\nqcapMarketCap: %llu\nraisedFundByQcap: %llu\nlastRoundPriceOfQcap: %llu\nrevenueByQearn:%llu\nqcapSoldAmount: %u\nshareholderDividend: %u\nQCAPHolderPermille: %u\nreinvestingPermille: %u\nburnPermille: %u\nqcapBurnPermille: %u\nnumberOfStaker: %u\nnumberOfVotingPower: %u\nnumberOfGP: %u\nnumberOfQCP: %u\nnumberOfIPOP: %u\nnumberOfQEarnP: %u\nnumberOfFundP: %u\nnumberOfMKTP: %u\nnumberOfAlloP: %u\ntransferRightsFee: %u\nminQuorumRq: %u\nmaxQuorumRq: %u\ntotalQcapBurntAmount: %u\ncirculatingSupply: %u\nquorumPercent: %u\n", result.returnCode, result.totalVotingPower, result.proposalCreateFund, result.reinvestingFund, result.totalEpochRevenue, result.fundForBurn, result.totalStakedQcapAmount, result.qcapMarketCap, result.raisedFundByQcap, result.lastRoundPriceOfQcap, result.revenueByQearn, result.qcapSoldAmount, result.shareholderDividend, result.QCAPHolderPermille, result.reinvestingPermille, result.burnPermille, result.qcapBurnPermille, result.numberOfStaker, result.numberOfVotingPower, result.numberOfGP, result.numberOfQCP, result.numberOfIPOP, result.numberOfQEarnP, result.numberOfFundP, result.numberOfMKTP, result.numberOfAlloP, result.transferRightsFee, result.minQuorumRq, result.maxQuorumRq, result.totalQcapBurntAmount, result.circulatingSupply, result.quorumPercent);
}

void getStakedAmountAndVotingPower(const char* nodeIp, int nodePort, const char* address)
{
    QvaultGetStakedAmountAndVotingPower_input input{};
    getPublicKeyFromIdentity(address, input.address);

    QvaultGetStakedAmountAndVotingPower_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_STAKED_AMOUNT_AND_VOTING_POWER,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    printf("returnCode: %d\nstakedAmount: %u\nvotingPower: %u\n", result.returnCode, result.stakedAmount, result.votingPower);
}

void getGP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    QvaultGetGP_input input{};
    input.proposalId = proposalId;
    QvaultGetGP_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_GP,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, EMPTY_IDENTITY) == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("returnCode: %d\n%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\n", result.returnCode, result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getQCP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    QvaultGetQCP_input input{};
    input.proposalId = proposalId;
    QvaultGetQCP_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_QCP,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, EMPTY_IDENTITY) == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("returnCode: %d\n%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\nnewQuorumPercent: %u\n", result.returnCode, result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.newQuorumPercent);

    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 3)
    {
        printf("The proposal has been rejected due to another proposal with more yes vote!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getIPOP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    QvaultGetIPOP_input input{};
    input.proposalId = proposalId;
    QvaultGetIPOP_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_IPOP,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, EMPTY_IDENTITY) == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("returnCode: %d\n%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\ntotalWeight: %llu\nassignedFund: %llu\nipoContractIndex: %u\n", result.returnCode, result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.totalWeight, result.proposal.assignedFund, result.proposal.ipoContractIndex);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getQEarnP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    QvaultGetQEarnP_input input{};
    input.proposalId = proposalId;
    QvaultGetQEarnP_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_QEARNP,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, EMPTY_IDENTITY) == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("returnCode: %d\n%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\namountOfInvestPerEpoch: %llu\nassignedFundPerEpoch: %llu\nnumberOfEpoch: %u\n", result.returnCode, result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.amountOfInvestPerEpoch, result.proposal.assignedFundPerEpoch, result.proposal.numberOfEpoch);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getFundP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    QvaultGetFundP_input input{};
    input.proposalId = proposalId;
    QvaultGetFundP_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_FUNDP,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, EMPTY_IDENTITY) == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("returnCode: %d\n%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\npricePerOneQcap: %llu\namountOfQcap: %u\nrestSaleAmount: %u\n", result.returnCode, result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.pricePerOneQcap, result.proposal.amountOfQcap, result.proposal.restSaleAmount);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 3)
    {
        printf("The proposal has been rejected due to overflow sale amount per year!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getMKTP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    QvaultGetMKTP_input input{};
    input.proposalId = proposalId;
    QvaultGetMKTP_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_MKTP,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, EMPTY_IDENTITY) == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("returnCode: %d\n%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\namountOfQubic: %llu\nshareName: %llu\namountOfQcap: %u\nshareIndex: %u\namountOfShare: %u\n", result.returnCode, result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.amountOfQubic, result.proposal.shareName, result.proposal.amountOfQcap, result.proposal.shareIndex, result.proposal.amountOfShare);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 3)
    {
        printf("The proposal has been rejected due to overflow sale amount per year!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getAlloP(const char* nodeIp, int nodePort, uint32_t proposalId)
{
    QvaultGetAlloP_input input{};
    input.proposalId = proposalId;
    QvaultGetAlloP_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_ALLOP,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    char proposer[128] = {0};
    getIdentityFromPublicKey(result.proposal.proposer, proposer, false);

    if (strcmp(proposer, EMPTY_IDENTITY) == 0)
    {
        printf("ERROR: Didn't receive valid proposal with index %u\n", proposalId);
        return ;
    }

    printf("returnCode: %d\n%s\nproposer: %s\ncurrentTotalVotingPower: %u\nnumberOfYes: %u\nnumberOfNo: %u\nproposedEpoch: %u\ncurrentQuorumPercent: %u\nreinvested: %u\ndistributed: %u\nteam: %u\nburnQcap: %u\n", result.returnCode, result.proposal.url, proposer, result.proposal.currentTotalVotingPower, result.proposal.numberOfYes, result.proposal.numberOfNo, result.proposal.proposedEpoch, result.proposal.currentQuorumPercent, result.proposal.reinvested, result.proposal.distributed, result.proposal.team, result.proposal.burnQcap);
    if (result.proposal.result == 0)
    {
        printf("The proposal has been approved!\n");
    }
    else if (result.proposal.result == 1)
    {
        printf("The proposal has been rejected due to more no vote!\n");
    }
    else if (result.proposal.result == 2)
    {
        printf("The proposal has been rejected due to insufficient Quorum!\n");
    }
    else if (result.proposal.result == 3)
    {
        printf("The proposal has been rejected due to another proposal with more yes vote!\n");
    }
    else if (result.proposal.result == 4)
    {
        printf("Active proposal!\n");
    }
}

void getIdentitiesHvVtPw(const char* nodeIp, int nodePort, uint32_t offset, uint32_t count)
{
    QvaultGetIdentitiesHvVtPw_input input{};
    input.offset = offset;
    input.count = count;
    QvaultGetIdentitiesHvVtPw_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_IDENTITIES_HV_VT_PW,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    uint32_t maxEntries = std::min(count, QVAULT_MAX_URLS_COUNT);
    for (uint32_t i = 0; i < maxEntries; i++)
    {
        char stakerAddress[128] = {0};
        getIdentityFromPublicKey(result.idList[i], stakerAddress, false);
        if (strcmp(stakerAddress, EMPTY_IDENTITY) == 0)
        {
            break;
        }
        printf("stakerAddress: %s amount: %u\n", stakerAddress, result.amountList[i]);
    }
}

void ppCreationPower(const char* nodeIp, int nodePort, const char* address)
{
    QvaultppCreationPower_input input{};
    getPublicKeyFromIdentity(address, input.address);
    QvaultppCreationPower_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_PP_CREATION_POWER,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    printf("returnCode: %d\n%s\n", result.returnCode, (result.status != 0) ? "You have a power to create the proposal": "You can not create the proposal");
}

void getQcapBurntAmountInLastEpoches(const char* nodeIp, int nodePort, uint32_t numberOfLastEpoches)
{
    QvaultGetQcapBurntAmountInLastEpoches_input input{};
    input.numberOfLastEpoches = numberOfLastEpoches;
    QvaultGetQcapBurntAmountInLastEpoches_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_QCAP_BURNT_AMOUNT_IN_LAST_EPOCHES,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    printf("returnCode: %d\nburntAmount: %u\n", result.returnCode, result.burntAmount);
}

void getAmountToBeSoldPerYear(const char* nodeIp, int nodePort, uint32_t year)
{
    QvaultGetAmountToBeSoldPerYear_input input{};
    input.year = year;
    QvaultGetAmountToBeSoldPerYear_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_AMOUNT_TO_BE_SOLD_PER_YEAR,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    printf("%u\n", result.amount);
}

void getTotalRevenueInQcap(const char* nodeIp, int nodePort)
{
    QvaultGetTotalRevenueInQcap_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_TOTAL_REVENUE_IN_QCAP,
                           nullptr,
                           0,
                           &result,
                           sizeof(result)))
        return;

    printf("%llu\n", result.revenue);
}

void getRevenueInQcapPerEpoch(const char* nodeIp, int nodePort, uint32_t epoch)
{
    QvaultGetRevenueInQcapPerEpoch_input input{};
    input.epoch = epoch;
    QvaultGetRevenueInQcapPerEpoch_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_REVENUE_IN_QCAP_PER_EPOCH,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    printf("epochTotalRevenue: %llu\nepochOneQcapRevenue: %llu\nepochOneQvaultRevenue: %llu\nepochReinvestAmount: %llu\n", result.epochTotalRevenue, result.epochOneQcapRevenue, result.epochOneQvaultRevenue, result.epochReinvestAmount);
}

void getRevenuePerShare(const char* nodeIp, int nodePort, uint32_t contractIndex)
{
    QvaultGetRevenuePerShare_input input{};
    input.contractIndex = contractIndex;
    QvaultGetRevenuePerShare_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_REVENUE_PER_SHARE,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    printf("revenue: %llu\n", result.revenue);
}

void getAmountOfShareQvaultHold(const char* nodeIp, int nodePort, const char* assetName, const char* issuer)
{
    uint64_t encodedAssetName = 0;
    if (!encodeAssetName(assetName, encodedAssetName))
    {
        printf("The asset name should be at most 8 characters.\n");
        return;
    }
    QvaultGetAmountOfShareQvaultHold_input input{};
    getPublicKeyFromIdentity(issuer, input.assetInfo.issuer);
    input.assetInfo.assetName = encodedAssetName;

    QvaultGetAmountOfShareQvaultHold_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_AMOUNT_OF_SHARE_QVAULT_HOLD,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    printf("The amount of %s that SC held: %u\n", assetName, result.amount);
}

void getNumberOfHolderAndAvgAm(const char* nodeIp, int nodePort)
{
    QvaultGetNumberOfHolderAndAvgAm_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_NUMBER_OF_HOLDER_AND_AVG_AM,
                           nullptr,
                           0,
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    printf("returnCode: %d\nnumberOfQcapHolder: %u\navgAmount: %u\n", result.returnCode, result.numberOfQcapHolder, result.avgAmount);
}

void getAmountForQearnInUpcomingEpoch(const char* nodeIp, int nodePort, uint32_t epoch)
{
    QvaultGetAmountForQearnInUpcomingEpoch_input input{};
    input.epoch = epoch;
    QvaultGetAmountForQearnInUpcomingEpoch_output result{};
    if (!runQvaultFunction(nodeIp, nodePort,
                           QVAULT_GET_AMOUNT_FOR_QEARN_IN_UPCOMING_EPOCH,
                           &input,
                           sizeof(input),
                           &result,
                           sizeof(result)))
        return;

    if (result.returnCode != 0)
    {
        printf("Error: returnCode = %d\n", result.returnCode);
        return;
    }

    printf("returnCode: %d\namount: %llu\n", result.returnCode, result.amount);
}