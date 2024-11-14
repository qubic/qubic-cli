#pragma once

#include <stdint.h>

void gqmpropSetProposal(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidProposal);
void gqmpropClearProposal(const char* nodeIp, int nodePort, const char* seed,
	uint32_t scheduledTickOffset);
void gqmpropGetProposals(const char* nodeIp, int nodePort, const char* proposalIndexString);
void gqmpropVote(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalIndexString,
	const char* voteValueString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidVote);
void gqmpropGetVote(const char* nodeIp, int nodePort, const char* proposalIndexString,
	const char* voterIdentity, const char* voterSeed);
void gqmpropGetVotingResults(const char* nodeIp, int nodePort, const char* proposalIndexString);
void gqmpropGetRevenueDonationTable(const char* nodeIp, int nodePort);

void ccfSetProposal(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidProposal);
void ccfClearProposal(const char* nodeIp, int nodePort, const char* seed,
	uint32_t scheduledTickOffset);
void ccfGetProposals(const char* nodeIp, int nodePort, const char* proposalIndexString);
void ccfVote(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalIndexString,
	const char* voteValueString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidVote);
void ccfGetVote(const char* nodeIp, int nodePort, const char* proposalIndexString,
	const char* voterIdentity, const char* voterSeed);
void ccfGetVotingResults(const char* nodeIp, int nodePort, const char* proposalIndexString);
void ccfGetLatestTransfers(const char* nodeIp, int nodePort);


typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int64_t sint64;
typedef uint64_t uint64;


constexpr uint16 INVALID_PROPOSAL_INDEX = 0xffff;
constexpr uint32 INVALID_VOTER_INDEX = 0xffffffff;
constexpr sint64 NO_VOTE_VALUE = 0x8000000000000000;

// Single vote for all types of proposals defined in August 2024.
// Input data for contract procedure call
struct ProposalSingleVoteDataV1
{
	// Index of proposal the vote is about (can be requested with proposal voting API)
	uint16 proposalIndex;

	// Type of proposal, see ProposalTypes
	uint16 proposalType;

	// Tick when proposal has been set (to make sure that proposal version known by the voter matches the current version).
	uint32 proposalTick;

	// Value of vote. NO_VOTE_VALUE means no vote for every type.
	// For proposals types with multiple options, 0 is no, 1 to N are the other options in order of definition in proposal.
	// For scalar proposal types the value is passed directly.
	sint64 voteValue;
};
static_assert(sizeof(ProposalSingleVoteDataV1) == 16, "Unexpected struct size.");

// Voting result summary for all types of proposals defined in August 2024.
// Output data for contract function call for getting voting results.
struct ProposalSummarizedVotingDataV1
{
	// Index of proposal the vote is about (can be requested with proposal voting API)
	uint16 proposalIndex;

	// Count of options in proposal type (number of valid elements in optionVoteCount, 0 for scalar voting)
	uint16 optionCount;

	// Tick when proposal has been set (useful for checking if cached ProposalData is still up to date).
	uint32 proposalTick;

	// Number of voter who have the right to vote
	uint32 authorizedVoters;

	// Number of total votes casted
	uint32 totalVotes;

	// Voting results
	union
	{
		// Number of votes for different options (0 = no change, 1 to N = yes to specific proposed value)
		uint32 optionVoteCount[8];

		// Scalar voting result (currently only for proposalType VariableScalarMean, mean value of all valid votes)
		sint64 scalarVotingResult;
	};
};
static_assert(sizeof(ProposalSummarizedVotingDataV1) == 16 + 8 * 4, "Unexpected struct size.");

// Proposal type constants and functions.
// Each proposal type is composed of a class and a number of options. As an alternative to having N options (option votes),
// some proposal classes (currently the one to set a variable) may allow to vote with a scalar value in a range defined
// by the proposal (scalar voting).
struct ProposalTypes
{
	// Class of proposal type
	struct Class
	{
		// Options without extra data. Supported options: 2 <= N <= 8.
		static constexpr uint16 GeneralOptions = 0;

		// Propose to transfer amount to address. Supported options: 2 <= N <= 5.
		static constexpr uint16 Transfer = 0x100;

		// Propose to set variable to a value. Supported options: 2 <= N <= 5; N == 0 means scalar voting.
		static constexpr uint16 Variable = 0x200;
	};

	// Options yes and no without extra data -> result is histogram of options
	static constexpr uint16 YesNo = Class::GeneralOptions | 2;

	// 3 options without extra data -> result is histogram of options
	static constexpr uint16 ThreeOptions = Class::GeneralOptions | 3;

	// 3 options without extra data -> result is histogram of options
	static constexpr uint16 FourOptions = Class::GeneralOptions | 4;

	// Transfer given amount to address with options yes/no
	static constexpr uint16 TransferYesNo = Class::Transfer | 2;

	// Transfer amount to address with two options of amounts and option "no change"
	static constexpr uint16 TransferTwoAmounts = Class::Transfer | 3;

	// Transfer amount to address with three options of amounts and option "no change"
	static constexpr uint16 TransferThreeAmounts = Class::Transfer | 4;

	// Transfer amount to address with four options of amounts and option "no change"
	static constexpr uint16 TransferFourAmounts = Class::Transfer | 5;

	// Set given variable to proposed value with options yes/no
	static constexpr uint16 VariableYesNo = Class::Variable | 2;

	// Set given variable to proposed value with two options of values and option "no change"
	static constexpr uint16 VariableTwoValues = Class::Variable | 3;

	// Set given variable to proposed value with three options of values and option "no change"
	static constexpr uint16 VariableThreeValues = Class::Variable | 4;

	// Set given variable to proposed value with four options of values and option "no change"
	static constexpr uint16 VariableFourValues = Class::Variable | 5;

	// Set given variable to value, allowing to vote with scalar value, voting result is mean value
	static constexpr uint16 VariableScalarMean = Class::Variable | 0;


	// Contruct type from class + number of options (no checking if type is valid)
	static constexpr uint16 type(uint16 cls, uint16 options)
	{
		return cls | options;
	}

	// Return option count for a given proposal type (including "no change" option),
	// 0 for scalar voting (no checking if type is valid).
	static uint16 optionCount(uint16 proposalType)
	{
		return proposalType & 0x00ff;
	}

	// Return class of proposal type (no checking if type is valid).
	static uint16 cls(uint16 proposalType)
	{
		return proposalType & 0xff00;
	}
};

// Proposal data struct for all types of proposals defined in August 2024.
// Input data for contract procedure call, usable as ProposalDataType in ProposalVoting (persisted in contract states).
// You have to choose, whether to support scalar votes next to option votes. Scalar votes require 8x more storage in the state.
struct ProposalDataV1
{
	// URL explaining proposal, zero-terminated string.
	char url[256];

	// Epoch, when proposal is active. For setProposal(), 0 means to clear proposal and non-zero means the current epoch.
	uint16 epoch;

	// Type of proposal, see ProposalTypes.
	uint16 type;

	// Tick when proposal has been set. Output only, overwritten in setProposal().
	uint32 tick;

	// Proposal payload data (for all except types with class GeneralProposal)
	union Data
	{
		// Used if type class is Transfer
		struct Transfer
		{
			uint8 destination[32];
			sint64 amounts[4];   // N first amounts are the proposed options (non-negative, sorted without duplicates), rest zero
		} transfer;

		// Used if type class is Variable and type is not VariableScalarMean
		struct VariableOptions
		{
			uint64 variable;     // For identifying variable (interpreted by contract only)
			sint64 values[4];    // N first amounts are proposed options sorted without duplicates, rest zero
		} variableOptions;

		// Used if type is VariableScalarMean
		struct VariableScalar
		{
			uint64 variable;            // For identifying variable (interpreted by contract only)
			sint64 minValue;            // Minimum value allowed in proposedValue and votes, must be > NO_VOTE_VALUE
			sint64 maxValue;            // Maximum value allowed in proposedValue and votes, must be >= minValue
			sint64 proposedValue;       // Needs to be in range between minValue and maxValue

			static constexpr sint64 minSupportedValue = 0x8000000000000001;
			static constexpr sint64 maxSupportedValue = 0x7fffffffffffffff;
		} variableScalar;
	} data;
};
static_assert(sizeof(ProposalDataV1) == 256 + 8 + 64, "Unexpected struct size.");

// Proposal data struct for 2-option proposals (requires less storage space).
// Input data for contract procedure call, usable as ProposalDataType in ProposalVoting
struct ProposalDataYesNo
{
	// URL explaining proposal, zero-terminated string.
	uint8 url[256];

	// Epoch, when proposal is active. For setProposal(), 0 means to clear proposal and non-zero means the current epoch.
	uint16 epoch;

	// Type of proposal, see ProposalTypes.
	uint16 type;

	// Tick when proposal has been set. Output only, overwritten in setProposal().
	uint32 tick;

	// Proposal payload data (for all except types with class GeneralProposal)
	union Data
	{
		// Used if type class is Transfer
		struct Transfer
		{
			uint8 destination[32];
			sint64 amount;		// Amount of proposed option (non-negative)
		} transfer;

		// Used if type class is Variable and type is not VariableScalarMean
		struct VariableOptions
		{
			uint64 variable;    // For identifying variable (interpreted by contract only)
			sint64 value;		// Value of proposed option, rest zero
		} variableOptions;
	} data;

	// Whether to support scalar votes next to option votes.
	static constexpr bool supportScalarVotes = false;
};
static_assert(sizeof(ProposalDataYesNo) == 256 + 8 + 40, "Unexpected struct size.");
