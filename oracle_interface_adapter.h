#pragma once

#if !defined(NDEBUG)
#define NDEBUG
#endif

#include "core/src/contract_core/pre_qpi_def.h"
#include "core/src/contracts/qpi.h"
#include "core/src/oracle_core/oracle_interfaces_def.h"

#include "key_utils.h"

#include <string>
#include <vector>

// define static functions declared by qpi.h
static void* __acquireScratchpad(unsigned long long size, bool initZero) { return nullptr; }
static void __releaseScratchpad(void* ptr) {}


static std::string toString(const QPI::id& id)
{
	// check if ID is printable as a string
	bool printable = true;
	if (id.m256i_u8[31] == 0)
	{
		printable = true;
		for (int i = 0; i < 31 && id.m256i_u8[i] != 0; ++i)
		{
			if (!isprint(id.m256i_u8[i]))
			{
				printable = false;
				break;
			}
		}
	}

	if (printable)
	{
		return std::string((const char*) id.m256i_u8);
	}
	else
	{
		char buffer[64];
		getIdentityFromPublicKey(id.m256i_u8, buffer, true);
		return std::string(buffer);
	}
}

static std::string toString(const QPI::DateAndTime& dt)
{
	char buffer[100];
	if (dt.getMicrosecDuringMillisec())
		sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d'%03d", dt.getYear(), dt.getMonth(), dt.getDay(), dt.getHour(), dt.getMinute(), dt.getSecond(), dt.getMillisec(), dt.getMicrosecDuringMillisec());
	else
		sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d", dt.getYear(), dt.getMonth(), dt.getDay(), dt.getHour(), dt.getMinute(), dt.getSecond(), dt.getMillisec());
	std::string str(buffer);
	if (!dt.isValid())
		str += " (invalid date)";
	return str;
}

static std::string oracleQueryToString(const typename OI::Price::OracleQuery& query)
{
	return std::string("Price::OracleQuery(oracle = ") + toString(query.oracle) +
		", currency1 = " + toString(query.currency1) +
		", currency2 = " + toString(query.currency2) +
		", timestamp = " + toString(query.timestamp) + ")";
}

static std::string oracleReplyToString(const typename OI::Price::OracleReply& reply)
{
	return std::string("Price::OracleReply(numerator = ") + std::to_string(reply.numerator) + ", denominator = " + std::to_string(reply.denominator) + ")";
}

static std::string oracleQueryToString(const typename OI::Mock::OracleQuery& query)
{
	return std::string("Mock::OracleQuery(value = ") + std::to_string(query.value) + ")";
}

static std::string oracleReplyToString(const typename OI::Mock::OracleReply& reply)
{
	return std::string("Mock::OracleReply(echoedValue = ") + std::to_string(reply.echoedValue) + ", doubledValue = " + std::to_string(reply.doubledValue) + ")";
}

template <typename OracleInterface>
static std::string callOracleQueryToString(const std::vector<uint8_t>& query)
{
	typedef typename OracleInterface::OracleQuery T;
	if (query.size() != sizeof(T))
		return "[error: unexpected query size]";
	return oracleQueryToString(*reinterpret_cast<const T*>(query.data()));
}

static std::string oracleQueryToString(uint32_t interfaceIndex, const std::vector<uint8_t>& query)
{
	switch (interfaceIndex)
	{
	case OI::Price::oracleInterfaceIndex:
		return callOracleQueryToString<OI::Price>(query);
	case OI::Mock::oracleInterfaceIndex:
		return callOracleQueryToString<OI::Mock>(query);
	default:
		return "[error: unsupported interface]";
	}
}

template <typename OracleInterface>
static std::string callOracleReplyToString(const std::vector<uint8_t>& query)
{
	typedef typename OracleInterface::OracleReply T;
	if (query.size() != sizeof(T))
		return "[error: unexpected reply size]";
	return oracleReplyToString(*reinterpret_cast<const T*>(query.data()));
}

static std::string oracleReplyToString(uint32_t interfaceIndex, const std::vector<uint8_t>& reply)
{
	switch (interfaceIndex)
	{
	case OI::Price::oracleInterfaceIndex:
		return callOracleReplyToString<OI::Price>(reply);
	case OI::Mock::oracleInterfaceIndex:
		return callOracleReplyToString<OI::Mock>(reply);
	default:
		return "[error: unsupported interface]";
	}
}
