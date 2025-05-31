#pragma once

#include "structs.h"

struct projectInfo
{
    uint8_t creator[32];
    uint64_t tokenName;
    uint64_t supplyOfToken;
    uint32_t startDate;
    uint32_t endDate;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    bool isCreatedFundarasing;
};

struct fundaraisingInfo
{
    uint64_t tokenPrice;
    uint64_t soldAmount;
    uint64_t requiredFunds;
    uint64_t raisedFunds;
    uint32_t indexOfProject;
    uint32_t firstPhaseStartDate;
    uint32_t firstPhaseEndDate;
    uint32_t secondPhaseStartDate;
    uint32_t secondPhaseEndDate;
    uint32_t thirdPhaseStartDate;
    uint32_t thirdPhaseEndDate;
    uint32_t listingStartDate;
    uint32_t cliffEndDate;
    uint32_t vestingEndDate;
    uint8_t threshold;
    uint8_t TGE;
    uint8_t stepOfVesting;
    bool isCreatedToken;
};

struct NOSTROMOGetStats_input
{

};

struct NOSTROMOGetStats_output
{
    uint64_t epochRevenue, totalPoolWeight;
    uint32_t numberOfRegister, numberOfCreatedProject, numberOfFundaraising;

    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct NOSTROMOGetTierLevelByUser_input
{
    uint8_t userId[32];
};

struct NOSTROMOGetTierLevelByUser_output
{
    uint8_t tierLevel;
    
    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct NOSTROMOGetUserVoteStatus_input
{
    uint8_t userId[32];
};

struct NOSTROMOGetUserVoteStatus_output
{
    uint32_t numberOfVotedProjects;
    uint32_t projectIndexList[64];
    
    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct NOSTROMOCheckTokenCreatability_input
{
    uint64_t tokenName;
};

struct NOSTROMOCheckTokenCreatability_output
{
    bool result;             // result = 1 is the token already issued by SC
    
    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct NOSTROMOGetNumberOfInvestedAndClaimedProjects_input
{
    uint8_t userId[32];
};

struct NOSTROMOGetNumberOfInvestedAndClaimedProjects_output
{
    uint32_t numberOfInvestedProjects;
    uint32_t numberOfClaimedProjects;
    
    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct NOSTROMOGetProjectByIndex_input
{
    uint32_t indexOfProject;
};

struct NOSTROMOGetProjectByIndex_output
{
    projectInfo project;
    
    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct NOSTROMOGetFundarasingByIndex_input
{
    uint32_t indexOfFundarasing;
};

struct NOSTROMOGetFundarasingByIndex_output
{
    fundaraisingInfo fundarasing;
    
    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

struct NOSTROMOGetProjectIndexListByCreator_input
{
    uint8_t creator[32];
};

struct NOSTROMOGetProjectIndexListByCreator_output
{
    uint32_t indexListForProjects[64];
    
    static constexpr unsigned char type()
    {
        return RespondContractFunction::type();
    }
};

void registerInTier(const char* nodeIp, int nodePort,
                    const char* seed, 
                    uint32_t scheduledTickOffset, 
                    uint32_t tierLevel);

void logoutFromTier(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset);

void createProject(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    const char* tokenName,
                    uint64_t supply,
                    uint32_t startYear,
                    uint32_t startMonth,
                    uint32_t startDay,
                    uint32_t startHour,
                    uint32_t endYear,
                    uint32_t endMonth,
                    uint32_t endDay,
                    uint32_t endHour);

void voteInProject(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint32_t indexOfProject,
		            bool decision);

void createFundaraising(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint64_t tokenPrice,
                    uint64_t soldAmount,
                    uint64_t requiredFunds,

                    uint32_t indexOfProject,
                    uint32_t firstPhaseStartYear,
                    uint32_t firstPhaseStartMonth,
                    uint32_t firstPhaseStartDay,
                    uint32_t firstPhaseStartHour,
                    uint32_t firstPhaseEndYear,
                    uint32_t firstPhaseEndMonth,
                    uint32_t firstPhaseEndDay,
                    uint32_t firstPhaseEndHour,

                    uint32_t secondPhaseStartYear,
                    uint32_t secondPhaseStartMonth,
                    uint32_t secondPhaseStartDay,
                    uint32_t secondPhaseStartHour,
                    uint32_t secondPhaseEndYear,
                    uint32_t secondPhaseEndMonth,
                    uint32_t secondPhaseEndDay,
                    uint32_t secondPhaseEndHour,

                    uint32_t thirdPhaseStartYear,
                    uint32_t thirdPhaseStartMonth,
                    uint32_t thirdPhaseStartDay,
                    uint32_t thirdPhaseStartHour,
                    uint32_t thirdPhaseEndYear,
                    uint32_t thirdPhaseEndMonth,
                    uint32_t thirdPhaseEndDay,
                    uint32_t thirdPhaseEndHour,
                    uint32_t listingStartYear,
                    uint32_t listingStartMonth,
                    uint32_t listingStartDay,
                    uint32_t listingStartHour,

                    uint32_t cliffEndYear,
                    uint32_t cliffEndMonth,
                    uint32_t cliffEndDay,
                    uint32_t cliffEndHour,

                    uint32_t vestingEndYear,
                    uint32_t vestingEndMonth,
                    uint32_t vestingEndDay,
                    uint32_t vestingEndHour,

                    uint8_t threshold,
                    uint8_t TGE,
                    uint8_t stepOfVesting);

void investInProject(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint32_t indexOfFundaraising, uint64_t amount);

void claimToken(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint64_t amount,
		            uint32_t indexOfFundaraising);

void getStats(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset);

void getTierLevelByUser(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    const char* userId);

void getUserVoteStatus(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    const char* userId);

void checkTokenCreatability(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    const char* tokenName);

void getNumberOfInvestedAndClaimedProjects(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    const char* userId);

void getProjectByIndex(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint32_t indexOfProject);

void getFundarasingByIndex(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    uint32_t indexOfFundarasing);

void getProjectIndexListByCreator(const char* nodeIp, int nodePort, 
                    const char* seed, 
                    uint32_t scheduledTickOffset,
                    const char* creator);