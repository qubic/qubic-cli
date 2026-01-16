#pragma once

#if !defined(NDEBUG)
#define NDEBUG
#endif

#include "core/src/contract_core/pre_qpi_def.h"
#include "core/src/contracts/qpi.h"
#include "core/src/oracle_core/oracle_interfaces_def.h"

#include <string>
#include <vector>

static std::string oracleQueryToString(const typename OI::Price::OracleQuery& query)
{
	return "";
}

static std::string oracleReplyToString(const typename OI::Price::OracleReply& reply)
{
	return "";
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
	// TODO: add price interface
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
		// TODO: add price interface
	case OI::Mock::oracleInterfaceIndex:
		return callOracleReplyToString<OI::Mock>(reply);
	default:
		return "[error: unsupported interface]";
	}
}
