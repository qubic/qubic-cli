#pragma once

#include "structs.h"

// SC structs

struct GPInfo                   // General proposal
{
    uint8_t proposer[32];
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint8_t url[256];
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

struct QCPInfo                   // Quorum change proposal
{
    uint8_t proposer[32];
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint32_t newQuorumPercent;
    uint8_t url[256];
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

struct IPOPInfo         // IPO participation
{
    uint8_t proposer[32];
    uint64_t totalWeight;
    uint64_t assignedFund;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t ipoContractIndex;
    uint32_t currentQuorumPercent;
    uint8_t url[256];
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient invest funds.
};

struct QEarnPInfo       // Qearn participation proposal
{
    uint8_t proposer[32];
    uint64_t amountOfInvestPerEpoch;
    uint64_t assignedFundPerEpoch;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint8_t url[256];
    uint8_t numberOfEpoch;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient funds.
};

struct FundPInfo            // Fundraising proposal
{
    uint8_t proposer[32];
    uint64_t pricePerOneQcap;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t amountOfQcap;
    uint32_t restSaleAmount;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint8_t url[256];
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

struct MKTPInfo                 //  Marketplace proposal
{
    uint8_t proposer[32];
    uint64_t amountOfQubic;
    uint64_t shareName;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t amountOfQcap;
    uint32_t currentQuorumPercent;
    uint32_t proposedEpoch;
    uint32_t shareIndex;
    uint32_t amountOfShare;
    uint8_t url[256];
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient funds. 4 is the insufficient Qcap.
};

struct AlloPInfo
{
    uint8_t proposer[32];
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint32_t reinvested;
    uint32_t distributed;
    uint32_t team;
    uint32_t burnQcap;
    uint8_t url[256];
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

struct MSPInfo
{
    uint8_t proposer[32];
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t muslimShareIndex;
    uint32_t currentQuorumPercent;
    uint8_t url[256];
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

struct QVaultGetData_input
{
};
struct QVaultGetData_output
{
    uint8_t adminAddress[32];
    uint64_t totalVotingPower;
    uint64_t proposalCreateFund;
    uint64_t reinvestingFund;
    uint64_t totalNotMSRevenue;
    uint64_t totalMuslimRevenue;
    uint64_t fundForBurn;
    uint64_t totalStakedQcapAmount;
    uint64_t qcapMarketCap;
    uint64_t raisedFundByQcap;
    uint64_t lastRoundPriceOfQcap;
    uint64_t revenueByQearn;
    uint32_t qcapSoldAmount;
    uint32_t shareholderDividend;
    uint32_t QCAPHolderPermille;
    uint32_t reinvestingPermille;
    uint32_t devPermille;
    uint32_t burnPermille;
    uint32_t qcapBurnPermille;
    uint32_t numberOfStaker;
    uint32_t numberOfVotingPower;
    uint32_t numberOfGP;
    uint32_t numberOfQCP;
    uint32_t numberOfIPOP;
    uint32_t numberOfQEarnP;
    uint32_t numberOfFundP;
    uint32_t numberOfMKTP;
    uint32_t numberOfAlloP;
    uint32_t transferRightsFee;
    uint32_t numberOfMuslim;
    uint32_t numberOfMuslimShare;
    uint32_t minQuorumRq;
    uint32_t maxQuorumRq;
    uint32_t totalQcapBurntAmount;
    uint32_t circulatingSupply;
    uint32_t quorumPercent;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetStakedAmountAndVotingPower_input
{
    uint8_t address[32];
};

struct QvaultGetStakedAmountAndVotingPower_output
{
    uint32_t stakedAmount;
    uint32_t votingPower;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetGP_input
{
    uint32_t proposalId;
};

struct QvaultGetGP_output
{
    GPInfo proposal;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetQCP_input
{
    uint32_t proposalId;
};

struct QvaultGetQCP_output
{
    QCPInfo proposal;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetIPOP_input
{
    uint32_t proposalId;
};

struct QvaultGetIPOP_output
{
    IPOPInfo proposal;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetQEarnP_input
{
    uint32_t proposalId;
};

struct QvaultGetQEarnP_output
{
    QEarnPInfo proposal;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetFundP_input
{
    uint32_t proposalId;
};

struct QvaultGetFundP_output
{
    FundPInfo proposal;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetMKTP_input
{
    uint32_t proposalId;
};

struct QvaultGetMKTP_output
{
    MKTPInfo proposal;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetAlloP_input
{
    uint32_t proposalId;
};

struct QvaultGetAlloP_output
{
    AlloPInfo proposal;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetMSP_input
{
    uint32_t proposalId;
};

struct QvaultGetMSP_output
{
    MSPInfo proposal;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetIdentitiesHvVtPw_input
{
    uint32_t offset;
    uint32_t count;
};

struct stakingInfo
{
    uint8_t stakerAddress[32];
    uint32_t amount;
};

struct QvaultGetIdentitiesHvVtPw_output
{
    stakingInfo list[256];

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultppCreationPower_input
{
    uint8_t address[32];
};

struct QvaultppCreationPower_output
{
    bool status;         // 0 means that there is no the proposal creation power, 1 means that there is.

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetQcapBurntAmountInLastEpoches_input
{
    uint32_t numberOfLastEpoches;
};

struct QvaultGetQcapBurntAmountInLastEpoches_output
{
    uint32_t burntAmount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetAmountToBeSoldPerYear_input
{
    uint32_t year;
};

struct QvaultGetAmountToBeSoldPerYear_output
{
    uint32_t amount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetTotalRevenueInQcap_input
{
};

struct QvaultGetTotalRevenueInQcap_output
{
    uint64_t revenue;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetRevenueInQcapPerEpoch_input
{
    uint32_t epoch;
};

struct QvaultGetRevenueInQcapPerEpoch_output
{
    uint64_t epochTotalRevenue;
    uint64_t epochOneQcapRevenue;
    uint64_t epochOneMuslimRevenue;
    uint64_t epochOneQvaultRevenue;
    uint64_t epochReinvestAmount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetRevenuePerShare_input
{
    uint32_t contractIndex;
};

struct QvaultGetRevenuePerShare_output
{
    uint64_t revenue;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct qpiAsset
{
    uint8_t issuer[32];
	uint64_t assetName;
};

struct QvaultGetAmountOfShareQvaultHold_input
{
    qpiAsset assetInfo;
};

struct QvaultGetAmountOfShareQvaultHold_output
{
    uint32_t amount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetNumberOfHolderAndAvgAm_input
{
};

struct QvaultGetNumberOfHolderAndAvgAm_output
{
    uint32_t numberOfQcapHolder;
    uint32_t avgAmount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct QvaultGetAmountForQearnInUpcomingEpoch_input
{
    uint32_t epoch;
};

struct QvaultGetAmountForQearnInUpcomingEpoch_output
{
    uint64_t amount;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

void stake(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount);
void unStake(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount);
void submitGP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* url);
void submitQCP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t newQuorumPercent, const char* url);
void submitIPOP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t ipoContractIndex, const char* url);
void submitQEarnP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t amountPerEpoch, uint32_t numberOfEpoch, const char* url);
void submitFundP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t priceOfOneQcap, uint32_t amountOfQcap, const char* url);
void submitMKTP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t amountOfQubic, const char* shareName, uint32_t amountOfQcap, uint32_t indexOfShare, uint32_t amountOfShare, const char* url);
void submitAlloP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t reinvested, uint32_t team, uint32_t burn, uint32_t distribute, const char* url);
void submitMSP(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t shareIndex, const char* url);
void voteInProposal(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint64_t priceOfIPO, uint32_t proposalType, uint32_t proposalId, bool yes);
void buyQcap(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, uint32_t amount, uint64_t priceOfOneQcap);
void TransferShareManagementRights(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset, const char* issuer, const char* assetName, int64_t numberOfShares, uint32_t newManagingContractIndex);
void submitMuslimId(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset);
void cancelMuslimId(const char* nodeIp, int nodePort, const char* seed, uint32_t scheduledTickOffset);

void getData(const char* nodeIp, int nodePort);
void getStakedAmountAndVotingPower(const char* nodeIp, int nodePort, const char* address);
void getGP(const char* nodeIp, int nodePort, uint32_t proposalId);
void getQCP(const char* nodeIp, int nodePort, uint32_t proposalId);
void getIPOP(const char* nodeIp, int nodePort, uint32_t proposalId);
void getQEarnP(const char* nodeIp, int nodePort, uint32_t proposalId);
void getFundP(const char* nodeIp, int nodePort, uint32_t proposalId);
void getMKTP(const char* nodeIp, int nodePort, uint32_t proposalId);
void getAlloP(const char* nodeIp, int nodePort, uint32_t proposalId);
void getMSP(const char* nodeIp, int nodePort, uint32_t proposalId);
void getIdentitiesHvVtPw(const char* nodeIp, int nodePort, uint32_t offset, uint32_t count);
void ppCreationPower(const char* nodeIp, int nodePort, const char* address);
void getQcapBurntAmountInLastEpoches(const char* nodeIp, int nodePort, uint32_t numberOfLastEpoches);
void getAmountToBeSoldPerYear(const char* nodeIp, int nodePort, uint32_t year);
void getTotalRevenueInQcap(const char* nodeIp, int nodePort);
void getRevenueInQcapPerEpoch(const char* nodeIp, int nodePort, uint32_t epoch);
void getRevenuePerShare(const char* nodeIp, int nodePort, uint32_t contractIndex);
void getAmountOfShareQvaultHold(const char* nodeIp, int nodePort, const char* assetName, const char* issuer);
void getNumberOfHolderAndAvgAm(const char* nodeIp, int nodePort);
void getAmountForQearnInUpcomingEpoch(const char* nodeIp, int nodePort, uint32_t epoch);
