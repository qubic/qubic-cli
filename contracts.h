#include <stdint.h>

// Change this number when new contracts are added, also add new indices to functions in contracts.cpp and to
// shareholderProposalSupportPerContract below
#define CONTRACT_COUNT 23

enum {
	NoShareholderProposalSupported,
	DefaultYesNoSingleVarShareholderProposalSupported,
	V1ScalarSingleVarShareholderProposalSupported,
};

const int shareholderProposalSupportPerContract[] = {
	NoShareholderProposalSupported, // 0: no contract
	NoShareholderProposalSupported, // 1: QX
	NoShareholderProposalSupported, // 2: QUOTTERY
	NoShareholderProposalSupported, // 3: RANDOM
	DefaultYesNoSingleVarShareholderProposalSupported, // 4: QUTIL
	NoShareholderProposalSupported, // 5: MLM
	NoShareholderProposalSupported, // 6: GQMPROP
	NoShareholderProposalSupported, // 7: SWATCH
	NoShareholderProposalSupported, // 8: CCF
	NoShareholderProposalSupported, // 9: QEARN
	NoShareholderProposalSupported, // 10: QVAULT
	NoShareholderProposalSupported, // 11: MSVAULT
	NoShareholderProposalSupported, // 12: QBAY
	NoShareholderProposalSupported, // 13: QSWAP
	NoShareholderProposalSupported, // 14: NOST
	NoShareholderProposalSupported, // 15: QDRAW
	NoShareholderProposalSupported, // 16: RL
	NoShareholderProposalSupported, // 17: QBOND
	NoShareholderProposalSupported, // 18: QIP
	NoShareholderProposalSupported, // 19: QRAFFLE
	NoShareholderProposalSupported, // 20: QRWA
	NoShareholderProposalSupported, // 21: QRP
	NoShareholderProposalSupported, // 22: QTF
	NoShareholderProposalSupported, // 23: QDUEL
	// add new contracts here
	DefaultYesNoSingleVarShareholderProposalSupported, // N+1: TESTEXA
	V1ScalarSingleVarShareholderProposalSupported, // N+2: TESTEXB
	NoShareholderProposalSupported, // N+3: TESTEXC
	NoShareholderProposalSupported, // N+4: TESTEXD
};

#define TESTEXA_CONTRACT_INDEX (CONTRACT_COUNT + 1)
#define TESTEXB_CONTRACT_INDEX (CONTRACT_COUNT + 2)
#define TESTEXC_CONTRACT_INDEX (CONTRACT_COUNT + 3)
#define TESTEXD_CONTRACT_INDEX (CONTRACT_COUNT + 3)

extern bool g_enableTestContracts;

uint32_t getContractIndex(const char* str, bool enableTestContracts = g_enableTestContracts);
const char* getContractName(uint32_t contractIndex, bool enableTestContracts = g_enableTestContracts);
