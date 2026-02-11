#include "qpi_adapter.h"

#include "key_utils.h"
#include "utils.h"
#include "logger.h"

#include <chrono>


std::string toString(const QPI::id& id)
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

std::string toString(const QPI::DateAndTime& dt)
{
	char buffer[100];
	if (dt.getMicrosecDuringMillisec())
		sprintf(buffer, "%04d-%02d-%02d_%02d:%02d:%02d.%03d'%03d", dt.getYear(), dt.getMonth(), dt.getDay(), dt.getHour(), dt.getMinute(), dt.getSecond(), dt.getMillisec(), dt.getMicrosecDuringMillisec());
	else
		sprintf(buffer, "%04d-%02d-%02d_%02d:%02d:%02d.%03d", dt.getYear(), dt.getMonth(), dt.getDay(), dt.getHour(), dt.getMinute(), dt.getSecond(), dt.getMillisec());
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

std::string oracleQueryToString(uint32_t interfaceIndex, const std::vector<uint8_t>& query)
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

std::string oracleReplyToString(uint32_t interfaceIndex, const std::vector<uint8_t>& reply)
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

std::string oracleInterfaceToString(uint32_t interfaceIndex)
{
	switch (interfaceIndex)
	{
	case OI::Price::oracleInterfaceIndex:
		return "Price";
	case OI::Mock::oracleInterfaceIndex:
		return "Mock";
	default:
		return "[error: unsupported interface]";
	}
}

uint32_t stringToOracleInterface(const char* oracleInterfaceString)
{
	if (strcasecmp(oracleInterfaceString, "Price") == 0)
	{
		return OI::Price::oracleInterfaceIndex;
	}
	else if (strcasecmp(oracleInterfaceString, "Mock") == 0)
	{
		return OI::Mock::oracleInterfaceIndex;
	}
	else
	{
		return UINT32_MAX;
	}
}

void printOracleQueryParamHelp(uint32_t interfaceIndex)
{
	switch (interfaceIndex)
	{
	case OI::Price::oracleInterfaceIndex:
		LOG("As [QUERY_PARAMS] pass a oracle, currency1, currency2, and UTC time separated with commas without any spaces.\n");
		LOG("UTC time is optional, default is now.\n");
		LOG("Examples:\n");
		LOG("    binance,btc,usdt,now                (current BTC/USDT price from binance)\n");
		LOG("    mexc,ETH,BTC,2026-01-31_15:20:40    (ETH/BTC price from MEXC at the given time)\n");
		LOG("    gate,Qubic,USDT,2026-02-09_0:0:0.0  (QUBIC/USDT price from Gate.io at the given time)\n");
		LOG("    gate_mexc,QUBIC,USDT                (current average price from gate.io and mexc)\n");
		break;
	case OI::Mock::oracleInterfaceIndex:
		LOG("As [QUERY_PARAMS] pass an unsigned integer value.\n");
		break;
	}
}

static bool parseOracleQueryParams(const char* paramString, typename OI::Mock::OracleQuery& query)
{
	try
	{
		query.value = std::stoull(paramString);
	}
	catch (...)
	{
		LOG("Expected unsigned integer, found %s!\n", paramString);
		return false;
	}
	return true;
}

static bool parseOracleQueryParams(const char* paramString, typename OI::Price::OracleQuery& query)
{
	std::vector<std::string> parts = splitString(paramString, ",");
	if (parts.size() < 3 || parts.size() > 4)
	{
		LOG("As [QUERY_PARAMS] pass \"[oracle],[currency1],[currency2]\" or \"[oracle],[currency1],[currency2],[utc_time]\".\n");
		return false;
	}

	const std::string& oracle = parts[0];
	if (oracle.size() > 31)
	{
		LOG("The [oracle] must have 31 characters at most.\n");
		return false;
	}
	memcpy(&query.oracle, oracle.data(), oracle.size());

	const std::string& currency1 = parts[1];
	const std::string& currency2 = parts[2];
	if (currency1.size() > 31 && currency2.size() > 31)
	{
		LOG("Both [currency1] and [currency2] must have 31 characters at most.\n");
		return false;
	}
	memcpy(&query.currency1, currency1.data(), currency1.size());
	memcpy(&query.currency2, currency2.data(), currency2.size());

	if (parts.size() < 4 || strcasecmp(parts[3].c_str(), "now") == 0)
	{
		// use current UTC time
		using namespace std::chrono;
		auto now = system_clock::now();
		auto tt = system_clock::to_time_t(now);
		std::tm tm;
#if defined(_WIN32)
		gmtime_s(&tm, &tt);   // (UTC)
#else
		gmtime_r(&tt, &tm);   // (UTC)
#endif
		query.timestamp.set(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	else
	{
		unsigned int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, millisecond = 0;
		bool okay;
		if (parts[3].find('.') != std::string::npos)
			okay = (sscanf(parts[3].c_str(), "%u-%u-%u_%u:%u:%u.%u", &year, &month, &day, &hour, &minute, &second, &millisecond) == 7);
		else
			okay = (sscanf(parts[3].c_str(), "%u-%u-%u_%u:%u:%u", &year, &month, &day, &hour, &minute, &second) == 6);
		if (!okay || !query.timestamp.setIfValid(year, month, day, hour, minute, second, millisecond))
		{
			LOG("Unable to parse time string \"%s\" into valid date/time.\n", parts[3].c_str());
			LOG("It should follow the pattern \"YEAR-MONTH-DAY_HOUR:MINUTE:SECOND.MILLISECOND\". \".MILLISECOND\" is optional.\n");
			return false;
		}
	}

	return true;
}

template <typename OracleInterface>
static std::vector<uint8_t> callParseOracleQueryParams(const char* oracleQueryParams)
{
	typedef typename OracleInterface::OracleQuery T;
	std::vector<uint8_t> query(sizeof(T), 0);
	if (!parseOracleQueryParams(oracleQueryParams, *reinterpret_cast<T*>(query.data())))
	{
		// error
		LOG("\nMore help:\n");
		printOracleQueryParamHelp(OracleInterface::oracleInterfaceIndex);
		query.clear();
	}

	return query;
}

std::vector<uint8_t> parseOracleQueryParams(uint32_t interfaceIndex, const char* oracleQueryString)
{
	switch (interfaceIndex)
	{
	case OI::Price::oracleInterfaceIndex:
		return callParseOracleQueryParams<OI::Price>(oracleQueryString);
	case OI::Mock::oracleInterfaceIndex:
		return callParseOracleQueryParams<OI::Mock>(oracleQueryString);
	default:
		LOG("Error: unsupported interface\n");
		return {};
	}
}
