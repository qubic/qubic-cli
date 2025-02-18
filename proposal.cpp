#include <iostream>
#include <algorithm>
#include <cctype>

#include "proposal.h"
#include "walletUtils.h"
#include "keyUtils.h"
#include "sanityCheck.h"
#include "commonFunctions.h"
#include "utils.h"

#define GQMPROP_CONTRACT_INDEX 6

#define GQMPROP_FUNC_GET_PROPOSAL_INDICES 1
#define GQMPROP_FUNC_GET_PROPOSAL 2
#define GQMPROP_FUNC_GET_VOTE 3
#define GQMPROP_FUNC_GET_VOTING_RESULT 4
#define GQMPROP_FUNC_GET_REV_DONATION 5

#define GQMPROP_PROC_SET_PROPOSAL 1
#define GQMPROP_PROC_VOTE 2


#define CCF_CONTRACT_INDEX 8

#define CCF_FUNC_GET_PROPOSAL_INDICES 1
#define CCF_FUNC_GET_PROPOSAL 2
#define CCF_FUNC_GET_VOTE 3
#define CCF_FUNC_GET_VOTING_RESULT 4
#define CCF_FUNC_GET_LATEST_TRANSFERS 5
#define CCF_FUNC_GET_PROPOSAL_FEE 6

#define CCF_PROC_SET_PROPOSAL 1
#define CCF_PROC_VOTE 2


void toLower(std::string& data)
{
	std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return std::tolower(c); });
}

void convertProposalDataToYesNo(const ProposalDataV1& pv1, ProposalDataYesNo& pyn)
{
	memcpy(&pyn, &pv1, sizeof(pyn));
}

void convertProposalDataToV1(const ProposalDataYesNo& pyn, ProposalDataV1& pv1)
{
	memcpy(&pv1, &pyn, sizeof(pyn));
	memset(((char*)&pv1) + sizeof(pyn), 0, sizeof(pv1) - sizeof(pyn));
}

bool printAndCheckProposal(const ProposalDataV1& p, int contract, const uint8_t* proposerPublicKey = nullptr, int proposalIndex = -1)
{
	bool okay = true;

	std::cout << "Proposal:\n";
	if (proposalIndex >= 0)
		std::cout << "\tproposalIndex = " << proposalIndex << std::endl;
	if (proposerPublicKey)
	{
		char proposerIdentity[128] = { 0 };
		bool isLowerCase = false;
		getIdentityFromPublicKey(proposerPublicKey, proposerIdentity, isLowerCase);
		std::cout << "\tproposer = " << proposerIdentity << std::endl;
	}

	std::cout << "\turl = " << p.url << std::endl;

	uint16 cls = ProposalTypes::cls(p.type);
	uint16 options = ProposalTypes::optionCount(p.type);
	std::string clsStr;
	if (cls == ProposalTypes::Class::GeneralOptions)
	{
		std::cout << "\ttype = GeneralOptions with " << options << " options";
		if (options < 2 || options > 8)
		{
			std::cout << " (ERROR: unsupported option count; select between 2 and 8)";
			okay = false;
		}
		else
		{
			std::cout << " (vote value 0 ... " << options - 1 << ")";
			
		}
		std::cout << std::endl;
	}
	else if (cls == ProposalTypes::Class::Transfer)
	{
		const auto& transfer = p.data.transfer;
		std::cout << "\ttype = Transfer with " << options << " options";
		if (options < 2 || options > 5)
		{
			std::cout << " (ERROR: unsupported option count; select between 2 and 5)";
			okay = false;
		}
		std::cout << std::endl;

		char dstIdentity[128] = { 0 };
		bool isLowerCase = false;
		getIdentityFromPublicKey(transfer.destination, dstIdentity, isLowerCase);
		std::cout << "\tdestination address = " << dstIdentity;
		uint64_t* dstU64 = (uint64_t*)transfer.destination;
		if (dstU64[1] == 0 && dstU64[2] == 0 && dstU64[3] == 0)
		{
			if (dstU64[0] == 0)
				std::cout << " (ERROR: invalid destination address)";
			else
				std::cout << " (contract " << dstU64[0] << ")";
		}
		std::cout << std::endl;

		std::cout << "\t\tvote value 0 = " << ((contract == CCF_CONTRACT_INDEX) ? "no transfer" : "no change to current status") << std::endl;
		for (int i = 0; i < std::min(options - 1, 4); ++i)
		{
			std::cout << "\t\tvote value " << i + 1 << " = ";
			if (contract == GQMPROP_CONTRACT_INDEX)
				std::cout << double(transfer.amounts[i]) / 10000.0 << "%"; // For GQMPROP amount is relative in millionth
			else
				std::cout << transfer.amounts[i];
			if (p.data.transfer.amounts[i] < 0)
			{
				std::cout << " (ERROR: negative amount)";
				okay = false;
			}
			else if (i >= 1)
			{
				if (transfer.amounts[i] < transfer.amounts[i - 1])
				{
					std::cout << " (ERROR: amounts not sorted)";
					okay = false;
				}
				else if (transfer.amounts[i] == transfer.amounts[i - 1])
				{
					std::cout << " (ERROR: duplicate amounts are not allowed)";
					okay = false;
				}
			}
			std::cout << std::endl;
		}
	}
	else if (cls == ProposalTypes::Class::Variable)
	{
		std::cout << "\ttype = Variable with " << options << " options";
		if (options == 0)
			std::cout << " (scalar votes)";
		else if (options < 2 || options > 5)
		{
			std::cout << " (ERROR: unsupported option count; select between 2 and 5)";
			okay = false;
		}
		std::cout << std::endl;

		if (options == 0)
		{
			// scalar votes
			const auto& variableScalar = p.data.variableScalar;
			std::cout << "\t\tvariable index = " << variableScalar.variable << std::endl;
			std::cout << "\t\tminimum allowed value = " << variableScalar.minValue;
			if (variableScalar.minValue < variableScalar.minSupportedValue)
			{
				std::cout << " (ERROR: value to small)";
				okay = false;
			}
			std::cout << std::endl;
			std::cout << "\t\tmaximum allowed value = " << variableScalar.maxValue;
			if (variableScalar.maxValue > variableScalar.maxSupportedValue)
			{
				std::cout << " (ERROR: value to large)";
				okay = false;
			}
			std::cout << std::endl;
			std::cout << "\t\tproposed value = " << variableScalar.proposedValue;
			if (variableScalar.proposedValue < variableScalar.minValue)
			{
				std::cout << " (ERROR: value to small)";
				okay = false;
			}
			if (variableScalar.proposedValue > variableScalar.maxValue)
			{
				std::cout << " (ERROR: value to large)";
				okay = false;
			}
			std::cout << std::endl;
		}
		else
		{
			// option votes
			const auto& variableOptions = p.data.variableOptions;
			std::cout << "\t\tvote value 0 = no change to current value of varibale" << std::endl;
			for (int i = 0; i < options - 1; ++i)
			{
				std::cout << "\t\tvote value " << i + 1 << " = set var value to " << variableOptions.values[i];
				if (i >= 1)
				{
					if (variableOptions.values[i] < variableOptions.values[i - 1])
					{
						std::cout << " (ERROR: values not sorted)";
						okay = false;
					}
					else if (variableOptions.values[i] == variableOptions.values[i - 1])
					{
						std::cout << " (ERROR: duplicate values are not allowed)";
						okay = false;
					}
				}
				std::cout << std::endl;
			}
		}
	}
	else
	{
		std::cout << "\ttype = unsupported class " << cls << std::endl;
		okay = false;
	}


	if (p.epoch == 1)
		std::cout << "\tepoch = current" << std::endl;
	else
		std::cout << "\tepoch = " << p.epoch << std::endl;
	if (p.tick == 0)
		std::cout << "\ttick = uninitialized" << std::endl;
	else
		std::cout << "\ttick = " << p.tick << std::endl;
	
	return okay;
}

bool printAndCheckProposal(const ProposalDataYesNo& pyn, int contract, const uint8_t* proposerPublicKey = nullptr, int proposalIndex = -1)
{
	ProposalDataV1 pv1;
	convertProposalDataToV1(pyn, pv1);
	return printAndCheckProposal(pv1, contract, proposerPublicKey, proposalIndex);
}

void printVotingResults(ProposalSummarizedVotingDataV1& results, bool quorumRule)
{
	std::cout << "Proposal voting results:"
		<< "\n\tproposal index = " << results.proposalIndex
		<< "\n\tproposal tick = " << results.proposalTick
		<< "\n\ttotal votes = " << results.totalVotes << " / " << results.authorizedVoters << std::endl;
	if (results.optionCount)
	{
		// option voting
		int mostVotedOption = 0;
		for (uint16 i = 0; i < results.optionCount; ++i)
		{
			std::cout << "\tvotes for option " << i << " = " << results.optionVoteCount[i] << std::endl;
			if (results.optionVoteCount[mostVotedOption] < results.optionVoteCount[i])
			{
				mostVotedOption = i;
			}
		}
		std::cout << "\tmost voted option = " << mostVotedOption;
		if (quorumRule)
		{
			if (results.totalVotes <= results.authorizedVoters * 2 / 3)
				std::cout << " (total votes not sufficient for acceptance of proposal)";
			else if (results.optionVoteCount[mostVotedOption] <= results.authorizedVoters / 3)
				std::cout << " (votes for this option not sufficient for acceptance of proposal)";
			else
				std::cout << " (sufficient votes to accept proposal option " << mostVotedOption << ")";
		}
		std::cout << std::endl;
	}
	else
	{
		// scalar voting
		// TODO
	}
}

void printSetProposalHelp()
{
	std::cout << "\nYou have to pass a proposal string with the following structure:\n\n";
	std::cout << "\t\"[ProposalTypeClass]|[NumberOfVoteOptions]:[AdditionalProposalData]|[URL]\"\n\n";
	std::cout << "[ProposalTypeClass]: Should be one of the following values:\n";
	std::cout << "\t- GeneralOptions: Normal proposal without automated execution.\n";
	std::cout << "\t\tNumberOfVoteOptions is 2, 3, ..., 7, or 8. AdditionalProposalData is unused/empty.\n";	
	std::cout << "\t- Transfer: Propose to transfer/donate an amount to a destination address.\n";
	std::cout << "\t\tAdditionalProposalData is comma-separated list of destination identity, amount of option 1,\n";
	std::cout << "\t\tamount of option 2, ... (options 0 is no change to current state. In GQMPROP contract, the\n";
	std::cout << "\t\tamount is relative in millionth, for example 150000 = 15% and 1000 = 0.1%.\n";
	std::cout << "\t- Variable: Propose to set variable in contract state to specific value. Not supported yet.\n";
	std::cout << std::endl;
}

bool parseProposalString(const char* proposalString, ProposalDataV1& p)
{
	memset(&p, 0, sizeof(p));
	if (!proposalString)
	{
		printSetProposalHelp();
		return false;
	}
	std::cout << "Parsing proposal string \"" << proposalString << "\" ..." << std::endl;

	// split proposal string in main parts
	char* writableProposalString = strdup(proposalString);
	std::string typeClassStr = strtok2string(writableProposalString, "|");
	std::string proposalDataStr = strtok2string(NULL, "|");
	std::string url = strtok2string(NULL, "|");
	free(writableProposalString);
	writableProposalString = strdup(proposalDataStr.c_str());
	std::string numberOptionStr = strtok2string(writableProposalString, ":");
	proposalDataStr = strtok2string(NULL, ":");
	free(writableProposalString);

	toLower(typeClassStr);
	uint16 typeClass = 0;
	if (typeClassStr == "transfer")
		typeClass = ProposalTypes::Class::Transfer;
	else if (typeClassStr == "generaloptions")
		typeClass = ProposalTypes::Class::GeneralOptions;
	else if (typeClassStr == "variable")
		typeClass = ProposalTypes::Class::Variable;
	else
	{
		std::cout << "ERROR: ProposalTypeClass must be one of the following:\n"
			<< "   - GeneralOptions\n"
			<< "   - Transfer\n"
			<< "   - Variable\n"
			<< std::endl;
		printSetProposalHelp();
		return false;
	}

	unsigned int numberOptions;
	uint16 typeOptions;
	if (sscanf(numberOptionStr.c_str(), "%u", &numberOptions) == 1 && numberOptions < 256)
		typeOptions = numberOptions;
	else
	{
		std::cout << "ERROR: NumberOfVoteOptions must be a positive 1-byte integer or 0 to use scalar voting!" << std::endl;
		std::cout << "Got NumberOfVoteOptions=\"" << numberOptionStr << "\"" << std::endl;
		printSetProposalHelp();
		return false;
	}

	if (url.find("://") == std::string::npos || url.length() > 255)
	{
		std::cout << "ERROR: URL must be a valid web URL of max length 255!" << std::endl;
		printSetProposalHelp();
		return false;
	}

	if (typeClass == ProposalTypes::Class::GeneralOptions)
	{
		if (proposalDataStr != "")
		{
			std::cout << "With GeneralOptions proposal class, AdditionalProposalData should be empty!" << std::endl;
			std::cout << "Got AdditionalProposalData=\"" << proposalDataStr << "\", which is ignored." << std::endl;
		}
	}
	else if (typeClass == ProposalTypes::Class::Transfer)
	{
		// split proposal string in main parts
		writableProposalString = strdup(proposalDataStr.c_str());
		std::string dstIdentity = strtok2string(writableProposalString, ",");
		std::cout << "Checking destination identity " << dstIdentity << " ...\n";
		sanityCheckIdentity(dstIdentity.c_str());
		getPublicKeyFromIdentity(dstIdentity.c_str(), p.data.transfer.destination);
		for (unsigned int i = 0; i < std::min(numberOptions - 1, 4u); ++i)
		{
			std::string amountStr = strtok2string(NULL, ",");
			sint64 amountInt;
			if (sscanf(amountStr.c_str(), "%lli", &amountInt) == 1)
			{
				p.data.transfer.amounts[i] = amountInt;
			}
			else
			{
				std::cout << "ERROR: option " << i+1 << " amount should be integer number." << std::endl;
				std::cout << "Got amount=\"" << amountStr << "\"." << std::endl;
				printSetProposalHelp();
				return false;
			}
		}
		free(writableProposalString);
	}
	else if (typeClass == ProposalTypes::Class::Variable)
	{
		std::cout << "ERROR: Variable not implemented yet." << std::endl;
		return false;
	}
	
	strcpy(p.url, url.c_str());
	p.type = ProposalTypes::type(typeClass, typeOptions);
	p.epoch = 1;
	return true;
}

void getProposalIndices(const char* nodeIp, int nodePort,
	uint32_t contractIndex,
	uint16_t inputType,
	bool activeProposals,
	std::vector<uint16_t>& proposalIndices)
{
	proposalIndices.clear();
}

template <typename ProposalDataType>
struct GetProposal_output
{
	bool okay;
	uint8_t _padding[7];
	uint8_t proposerPubicKey[32];
	ProposalDataType proposal;
};

template <typename ProposalDataType>
void getProposal(const char* nodeIp, int nodePort,
	uint32_t contractIndex,
	uint16_t inputType,
	uint16_t proposalIndex,
	GetProposal_output<ProposalDataType>& outProposal,
	QCPtr* qcPtr = nullptr)
{
	struct GetProposal_input
	{
		uint16 proposalIndex;
	} input;
	input.proposalIndex = proposalIndex;

	if (!runContractFunction(nodeIp, nodePort, contractIndex, inputType,
		&input, sizeof(input), &outProposal, sizeof(outProposal),
		qcPtr))
	{
		outProposal.okay = false;
	}
}

template <typename ProposalDataType>
void getAndPrintProposal(const char* nodeIp, int nodePort,
	uint32_t contractIndex,
	uint16_t inputType,
	uint16_t proposalIndex,
	QCPtr* qcPtr = nullptr)
{
	GetProposal_output<ProposalDataType> outProposal;
	getProposal(nodeIp, nodePort, contractIndex, inputType, proposalIndex, outProposal, qcPtr);
	if (outProposal.okay)
	{
		printAndCheckProposal(outProposal.proposal, contractIndex, outProposal.proposerPubicKey, proposalIndex);
	}
	else
	{
		std::cout << "ERROR: Didn't receive valid proposal with index " << proposalIndex << "!" << std::endl;
	}
}

bool getProposalIndices(const char* nodeIp, int nodePort,
	uint32_t contractIndex,
	uint16_t inputType,
	bool activeProposals,
	std::vector<uint16_t>& proposalIndices,
	QCPtr* qcPtr = nullptr)
{
	struct GetProposalIndices_input
	{
		bool activeProposals;		// Set true to return indices of active proposals, false for proposals of prior epochs
		int32_t prevProposalIndex;  // Set -1 to start getting indices. If returned index array is full, call again with highest index returned.
	} input;
	struct GetProposalIndices_output
	{
		uint16 numOfIndices;		// Number of valid entries in indices. Call again if it is 64.
		uint16 indices[64];	        // Requested proposal indices. Valid entries are in range 0 ... (numOfIndices - 1).
	} output;

	input.activeProposals = activeProposals;
	input.prevProposalIndex = -1;
	proposalIndices.clear();

	do
	{
		if (!runContractFunction(nodeIp, nodePort, contractIndex, inputType,
			&input, sizeof(input), &output, sizeof(output),
			qcPtr) || output.numOfIndices > 64)
		{
			std::cout << "ERROR: Didn't receive proposal indices!" << std::endl;
			return false;
		}
		
		for (uint16 i = 0; i < output.numOfIndices; ++i)
		{
			if (output.indices[i] >= NUMBER_OF_COMPUTORS)
			{
				std::cout << "WARNING: Unexpectedly large proposal index " << output.indices[i] << std::endl;
			}
			proposalIndices.push_back(output.indices[i]);
		}
		
		if (output.numOfIndices > 0)
			input.prevProposalIndex = output.indices[output.numOfIndices - 1];
	} 
	while (output.numOfIndices == 64);

	return true;
}


template <typename ProposalDataType>
void getAndPrintProposalsCommand(const char* nodeIp, int nodePort,
	const char* proposalIndexString,
	uint32_t contractIndex,
	uint16_t getProposalInputType,
	uint16_t getProposalIndicesInputType,
	QCPtr* qcPtr = nullptr)
{
	unsigned int proposalIndex;
	if (sscanf(proposalIndexString, "%ui", &proposalIndex) == 1 && proposalIndex <= 0xffffu)
	{
		// Print specific proposal
		getAndPrintProposal<ProposalDataType>(nodeIp, nodePort, contractIndex, getProposalInputType, proposalIndex);
	}
	else
	{
		// List all active or finished proposals
		bool activeProposals = (strcmp(proposalIndexString, "active") == 0);
		bool finisedProposals = (strcmp(proposalIndexString, "finished") == 0);
		if (!activeProposals && !finisedProposals)
		{
			std::cout << "ERROR: Proposal index must be positive integer up to 65535, \"active\" or \"finished\".\n"
				<< "Got: " << proposalIndexString << std::endl;
			return;
		}

		// Reuse connection
		auto qc = make_qc(nodeIp, nodePort);

		// Get indices
		std::vector<uint16> proposalIndices;
		if (getProposalIndices(nodeIp, nodePort, contractIndex, getProposalIndicesInputType, activeProposals, proposalIndices, &qc))
			std::cout << "Received list of " << proposalIndices.size() << " proposals" << std::endl;

		// Get and print all proposals
		for (auto idx : proposalIndices)
		{
			getAndPrintProposal<ProposalDataType>(nodeIp, nodePort, contractIndex, getProposalInputType, idx, &qc);
		}
	}
}

template <typename ProposalDataType>
void getVotingResults(const char* nodeIp, int nodePort, const char* proposalIndexString, uint32_t contractIdx, uint16_t getProposalInputType, uint16_t getVotingResultsInputType)
{
	unsigned int proposalIndex;
	if (sscanf(proposalIndexString, "%ui", &proposalIndex) != 1 || proposalIndex > 0xffffu)
	{
		std::cout << "ERROR: Proposal index must be positive integer up to 65535.\n"
			<< "Got: " << proposalIndexString << std::endl;
		return;
	}

	// Reuse connection
	auto qc = make_qc(nodeIp, nodePort);

	// Get proposal
	std::cout << "Querying data of proposal " << proposalIndex << " ..." << std::endl;
	GetProposal_output<ProposalDataType> outProposal;
	getProposal(nodeIp, nodePort,
		contractIdx, getProposalInputType,
		proposalIndex, outProposal, &qc);
	if (!outProposal.okay)
	{
		std::cout << "ERROR: Didn't receive valid proposal with index " << proposalIndex << "!" << std::endl;
		return;
	}
	printAndCheckProposal(outProposal.proposal, contractIdx, outProposal.proposerPubicKey, proposalIndex);

	// Get voting results
	struct GetVotingResults_input
	{
		uint16 proposalIndex;
	} input;
	struct GetVotingResults_output
	{
		bool okay;
		ProposalSummarizedVotingDataV1 results;
	} output;
	input.proposalIndex = proposalIndex;

	if (!runContractFunction(nodeIp, nodePort, contractIdx,
		getVotingResultsInputType, &input, sizeof(input), &output, sizeof(output), &qc) || !output.okay)
	{
		std::cout << "ERROR: Didn't receive valid response from GetVotingResults!" << std::endl;
		return;
	}

	printVotingResults(output.results, true);
}

// Get vote of proposal with given index and voter with given voterPublicKey if not nullptr or voterSeed
void getVote(const char* nodeIp, int nodePort, const char* proposalIndexString, const char* voterIdentity, const char* voterSeed, uint32_t contractIdx, uint16_t getVoteInputType)
{
	uint8_t voterPublicKey[32] = { 0 };
	if (voterIdentity)
	{
		// Get public key from identity
		sanityCheckIdentity(voterIdentity);
		getPublicKeyFromIdentity(voterIdentity, voterPublicKey);
	}
	else if (voterSeed)
	{
		// Get public key from seed
		sanityCheckSeed(voterSeed);
		uint8_t privateKey[32] = { 0 };
		uint8_t subseed[32] = { 0 };
		uint8_t digest[32] = { 0 };
		uint8_t signature[64] = { 0 };
		char publicIdentity[128] = { 0 };
		getSubseedFromSeed((uint8_t*)voterSeed, subseed);
		getPrivateKeyFromSubSeed(subseed, privateKey);
		getPublicKeyFromPrivateKey(privateKey, voterPublicKey);
	}
	else
	{
		std::cout << "ERROR: Either seed or voter identity must be passed!" << std::endl;
	}

	unsigned int proposalIndex;
	if (sscanf(proposalIndexString, "%ui", &proposalIndex) != 1 || proposalIndex > 0xffffu)
	{
		std::cout << "ERROR: Proposal index must be positive integer up to 65535.\n"
			<< "Got: " << proposalIndexString << std::endl;
		return;
	}

	struct GetVote_input
	{
		char voter[32];
		uint16 proposalIndex;
	} input;

	memcpy(input.voter, voterPublicKey, 32);
	input.proposalIndex = proposalIndex;

	struct GetVote_output
	{
		bool okay;
		ProposalSingleVoteDataV1 vote;
	} output;

	if (!runContractFunction(nodeIp, nodePort, contractIdx,
		getVoteInputType, &input, sizeof(input), &output, sizeof(output)))
	{
		std::cout << "ERROR: Didn't receive valid response from GetVote!" << std::endl;
		return;
	}
	if (!output.okay)
	{
		std::cout << "ERROR: No vote value available for you. Probably you have no right to vote." << std::endl;
		return;
	}

	std::cout << "Vote information:"
		<< "\n\tproposal index = " << output.vote.proposalIndex
		<< "\n\tproposal tick = " << output.vote.proposalTick
		<< "\n\tvote value = ";
	if (output.vote.voteValue == NO_VOTE_VALUE)
		std::cout << "no vote";
	else
		std::cout << output.vote.voteValue;
	std::cout << std::endl;
}

template <typename ProposalDataType>
void castVote(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalIndexString,
	const char* voteValueString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidVote,
	uint32_t contractIdx, uint16_t getProposalInputType, uint16_t castVoteInputType)
{
	// Check proposal index
	unsigned int proposalIndex;
	if (sscanf(proposalIndexString, "%ui", &proposalIndex) != 1 || proposalIndex > 0xffffu)
	{
		std::cout << "ERROR: Proposal index must be positive integer up to 65535.\n"
			<< "Got: " << proposalIndexString << std::endl;
		return;
	}

	// First check of vote value
	sint64 voteValue;
	if (sscanf(voteValueString, "%lli", &voteValue) != 1)
	{
		if (strcmp(voteValueString, "none") == 0)
		{
			voteValue = NO_VOTE_VALUE;
		}
		else
		{
			std::cout << "ERROR: Proposal index must be positive integer up to 65535.\n"
				<< "Got: " << proposalIndexString << std::endl;
			return;
		}
	}

	// Reuse connection
	auto qc = make_qc(nodeIp, nodePort);

	// Get proposal
	std::cout << "Querying data of proposal " << proposalIndex << " ..." << std::endl;
	GetProposal_output<ProposalDataType> outProposal;
	getProposal(nodeIp, nodePort,
		contractIdx, getProposalInputType,
		proposalIndex, outProposal, &qc);
	if (!outProposal.okay)
	{
		std::cout << "ERROR: Didn't receive valid proposal with index " << proposalIndex << "!" << std::endl;
		return;
	}
	printAndCheckProposal(outProposal.proposal, contractIdx, outProposal.proposerPubicKey, proposalIndex);

	// Check vote value vs proposal?
	if (!forceSendingInvalidVote)
	{
		uint16 typeClass = ProposalTypes::cls(outProposal.proposal.type);
		uint16 optionCount = ProposalTypes::optionCount(outProposal.proposal.type);
		if (typeClass == ProposalTypes::Class::Variable)
		{
			std::cout << "WARNING: Unexpected proposal for setting variable! Vote is not checked." << std::endl;
		}
		else
		{
			if (voteValue != NO_VOTE_VALUE && (voteValue < 0 || voteValue >= optionCount))
			{
				std::cout << "ERROR: Invalid vote value \"" << voteValue << "\". Pass an integer selecting an option from proposal above or \"none\" to remove your vote.";
				return;
			}
		}
	}

	ProposalSingleVoteDataV1 v;
	v.proposalTick = outProposal.proposal.tick;
	v.proposalType = outProposal.proposal.type;
	v.proposalIndex = proposalIndex;
	v.voteValue = voteValue;
	std::cout << "Vote value to cast: ";
	if (voteValue == NO_VOTE_VALUE)
		std::cout << "none (remove vote)\n";
	else
		std::cout << voteValue << "\n";

	std::cout << "\nSending vote (you need to be computor to do so) ..." << std::endl;
	makeContractTransaction(nodeIp, nodePort, seed, contractIdx, castVoteInputType, 0, sizeof(v), (uint8_t*)&v, scheduledTickOffset);
}

void gqmpropSetProposal(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidProposal)
{
	ProposalDataV1 p;
	bool proposalOkay = parseProposalString(proposalString, p) && printAndCheckProposal(p, GQMPROP_CONTRACT_INDEX);
	if (p.type == ProposalTypes::VariableScalarMean)
	{
		std::cout << "ERROR: VariableScalarMean is not supported by general quorum proposal contract!" << std::endl;
		proposalOkay = false;
	}
	if (!proposalOkay && !forceSendingInvalidProposal)
	{
		std::cout << "\nCancelling to send general quorum proposal due to errors in proposal..." << std::endl;
		return;
	}

	std::cout << "\nSending transaction to set your general quorum proposal (you need to be computor to do so)..." << std::endl;
	makeContractTransaction(nodeIp, nodePort, seed,
		GQMPROP_CONTRACT_INDEX, GQMPROP_PROC_SET_PROPOSAL, 0,
		sizeof(p), (uint8_t*)&p, scheduledTickOffset);
}

void gqmpropClearProposal(const char* nodeIp, int nodePort, const char* seed,
	uint32_t scheduledTickOffset)
{
	std::cout << "Sending transaction to clear your general quorum proposal (you need to be computor to do so)..." << std::endl;
	ProposalDataV1 p;
	memset(&p, 0, sizeof(p));
	p.epoch = 0;	// epoch 0 clears proposal
	makeContractTransaction(nodeIp, nodePort, seed,
		GQMPROP_CONTRACT_INDEX, GQMPROP_PROC_SET_PROPOSAL, 0,
		sizeof(p), (uint8_t*)&p, scheduledTickOffset);
}

void gqmpropGetProposals(const char* nodeIp, int nodePort, const char* proposalIndexString)
{
	getAndPrintProposalsCommand<ProposalDataV1>(nodeIp, nodePort,
		proposalIndexString, GQMPROP_CONTRACT_INDEX,
		GQMPROP_FUNC_GET_PROPOSAL, GQMPROP_FUNC_GET_PROPOSAL_INDICES);
}


void gqmpropVote(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalIndexString,
	const char* voteValueString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidVote)
{
	castVote<ProposalDataV1>(nodeIp, nodePort, seed, proposalIndexString, voteValueString, scheduledTickOffset, forceSendingInvalidVote,
		GQMPROP_CONTRACT_INDEX, GQMPROP_FUNC_GET_PROPOSAL, GQMPROP_PROC_VOTE);
}

// Get vote of proposal with given index and voter with given voterPublicKey if not nullptr or voterSeed
void gqmpropGetVote(const char* nodeIp, int nodePort, const char* proposalIndexString, const char* voterIdentity, const char* voterSeed)
{
	getVote(nodeIp, nodePort, proposalIndexString, voterIdentity, voterSeed, GQMPROP_CONTRACT_INDEX, GQMPROP_FUNC_GET_VOTE);
}

void gqmpropGetVotingResults(const char* nodeIp, int nodePort, const char* proposalIndexString)
{
	getVotingResults<ProposalDataV1>(nodeIp, nodePort, proposalIndexString, GQMPROP_CONTRACT_INDEX, GQMPROP_FUNC_GET_PROPOSAL, GQMPROP_FUNC_GET_VOTING_RESULT);
}

void gqmpropGetRevenueDonationTable(const char* nodeIp, int nodePort)
{
	struct RevenueDonationEntry
	{
		uint8 destinationPublicKey[32];
		sint64 millionthAmount;
		uint16 firstEpoch;
	};
	constexpr int numEntries = 128;
	struct RevenueDonationTable
	{
		RevenueDonationEntry tab[numEntries];
	} output;

	if (!runContractFunction(nodeIp, nodePort, GQMPROP_CONTRACT_INDEX,
		GQMPROP_FUNC_GET_REV_DONATION, nullptr, 0, &output, sizeof(output)))
	{
		std::cout << "ERROR: Didn't receive response from GetRevenueDonation!" << std::endl;
		return;
	}

	std::cout << "Revenue donation table:";
	for (int i = 0; i < numEntries; ++i)
	{
		if (!isZeroPubkey(output.tab[i].destinationPublicKey))
		{
			char identity[100] = { 0 };
			getIdentityFromPublicKey(output.tab[i].destinationPublicKey, identity, false);
			std::cout << "\n- " << i << ": " << double(output.tab[i].millionthAmount) / 10000.0 << "% (starting epoch "
				<< output.tab[i].firstEpoch << ") to " << identity;
			uint64_t* dstU64 = (uint64_t*)output.tab[i].destinationPublicKey;
			if (dstU64[1] == 0 && dstU64[2] == 0 && dstU64[3] == 0)
				std::cout << " (contract " << dstU64[0] << ")";
		}
	}
	std::cout << std::endl;
}

/*
Test cases:
- no proposals -> get indices (active/finished), get proposals with wrong index, get results
- set proposal -> get all active/finished proposals, get by index
- set proposal should only work for computor seeds

- get results -> 0 votes
- get vote -> no vote
- cast votes
- get results -> n votes
- get vote -> vote successful?
- overwrite proposal -> votes must disappear

*/

void ccfSetProposal(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidProposal)
{
	// TODO: query and check fee
	uint64_t fee = 1000000;

	// Parse and check
	ProposalDataV1 pv1;
	bool proposalOkay = parseProposalString(proposalString, pv1) && printAndCheckProposal(pv1, CCF_CONTRACT_INDEX);
	if (pv1.type != ProposalTypes::TransferYesNo)
	{
		std::cout << "ERROR: Only Transfer|2 (TransferYesNo) is supported as proposal type by CCF contract!" << std::endl;
		proposalOkay = false;
	}
	if (!proposalOkay && !forceSendingInvalidProposal)
	{
		std::cout << "\nCancelling to send CCF proposal due to errors in proposal ..." << std::endl;
		return;
	}

	// Transfer data to yes/no proposal data structure
	ProposalDataYesNo pyn;
	convertProposalDataToYesNo(pv1, pyn);

	// Send transaction
	std::cout << "\nSending transaction to set your CCF proposal ..." << std::endl;
	makeContractTransaction(nodeIp, nodePort, seed,
		CCF_CONTRACT_INDEX, CCF_PROC_SET_PROPOSAL, fee,
		sizeof(pyn), (uint8_t*)&pyn, scheduledTickOffset);
}

void ccfClearProposal(const char* nodeIp, int nodePort, const char* seed,
	uint32_t scheduledTickOffset)
{
	// TODO: query and check fee
	uint64_t fee = 1000000;

	std::cout << "Sending transaction to clear your CCF proposal ..." << std::endl;
	ProposalDataYesNo p;
	memset(&p, 0, sizeof(p));
	p.epoch = 0;	// epoch 0 clears proposal
	makeContractTransaction(nodeIp, nodePort, seed,
		CCF_CONTRACT_INDEX, CCF_PROC_SET_PROPOSAL, fee,
		sizeof(p), (uint8_t*)&p, scheduledTickOffset);
}

void ccfGetProposals(const char* nodeIp, int nodePort, const char* proposalIndexString)
{
	getAndPrintProposalsCommand<ProposalDataYesNo>(nodeIp, nodePort,
		proposalIndexString, CCF_CONTRACT_INDEX,
		CCF_FUNC_GET_PROPOSAL, CCF_FUNC_GET_PROPOSAL_INDICES);
}

void ccfGetVotingResults(const char* nodeIp, int nodePort, const char* proposalIndexString)
{
	getVotingResults<ProposalDataYesNo>(nodeIp, nodePort, proposalIndexString, CCF_CONTRACT_INDEX, CCF_FUNC_GET_PROPOSAL, CCF_FUNC_GET_VOTING_RESULT);
}

// Get vote of proposal with given index and voter with given voterPublicKey if not nullptr or voterSeed
void ccfGetVote(const char* nodeIp, int nodePort, const char* proposalIndexString, const char* voterIdentity, const char* voterSeed)
{
	getVote(nodeIp, nodePort, proposalIndexString, voterIdentity, voterSeed, CCF_CONTRACT_INDEX, CCF_FUNC_GET_VOTE);
}

void ccfVote(const char* nodeIp, int nodePort, const char* seed,
	const char* proposalIndexString,
	const char* voteValueString,
	uint32_t scheduledTickOffset,
	bool forceSendingInvalidVote)
{
	castVote<ProposalDataYesNo>(nodeIp, nodePort, seed, proposalIndexString, voteValueString, scheduledTickOffset, forceSendingInvalidVote,
		CCF_CONTRACT_INDEX, CCF_FUNC_GET_PROPOSAL, CCF_PROC_VOTE);
}

void ccfGetLatestTransfers(const char* nodeIp, int nodePort)
{
	struct LatestTransfersEntry
	{
		uint8 destination[32];
		uint8 url[256];
		sint64 amount;
		uint32 tick;
		bool success;
	};
	constexpr int numEntries = 128;
	struct RevenueDonationTable
	{
		LatestTransfersEntry tab[numEntries];
	};
	RevenueDonationTable* output = new RevenueDonationTable();

	if (!runContractFunction(nodeIp, nodePort, CCF_CONTRACT_INDEX,
		CCF_FUNC_GET_LATEST_TRANSFERS, nullptr, 0, output, sizeof(RevenueDonationTable)))
	{
		std::cout << "ERROR: Didn't receive response from GetLatestTransfers!" << std::endl;
		return;
	}

	// Find highest tick to start output with
	uint32 maxTick = 0;
	int maxTickIdx = 0;
	for (int i = 0; i < numEntries; ++i)
	{
		if (output->tab[i].tick > maxTick)
		{
			maxTick = output->tab[i].tick;
			maxTickIdx = i;
		}
	}

	std::cout << "Latest outgoing funding transfers of CCF (most recent on top):\n";
	for (int i = numEntries - 1; i >= 0; --i)
	{
		int idx = (i + maxTickIdx) % numEntries;
		const LatestTransfersEntry& t = output->tab[idx];
		if (!isZeroPubkey(t.destination))
		{
			char identity[100] = { 0 };
			getIdentityFromPublicKey(t.destination, identity, false);
			std::cout << " - proposal ";
			for (int j = 0; j < 256 && t.url[j]; ++j)
				std::cout << t.url[j];
			std::cout << "\n";
			std::cout << "   " << t.amount << " qus to " << identity;
			uint64_t* dstU64 = (uint64_t*)t.destination;
			if (dstU64[1] == 0 && dstU64[2] == 0 && dstU64[3] == 0)
				std::cout << " (contract " << dstU64[0] << ")";
			if (t.success)
				std::cout << "\n   accepted and transfered in tick " << t.tick << std::endl;
			else
				std::cout << "\n   accepted by quorum but not transfered due to insufficient funds in CCF (in tick " << t.tick << ")" << std::endl;
		}
	}

	delete output;
}

/*
Test cases:
- no proposals -> get indices (active/finished), get proposals with wrong index, get results
- set proposal -> get all active/finished proposals, get by index
- set proposal should work with any seed that has enough QU for fee

- get results -> 0 votes
- get vote -> no vote
- cast votes
- get results -> n votes
- get vote -> vote successful?
- overwrite proposal -> votes must disappear

*/


