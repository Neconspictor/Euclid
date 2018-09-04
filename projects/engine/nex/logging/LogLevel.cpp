#include <nex/logging/LogLevel.hpp>
#include <nex/util/StringUtils.hpp>

namespace nex::util
{
	/**
	* Maps log levels to a string representation.
	*/
	const static EnumString<LogLevel> converter[] = {
		{ Debug, "DEBUG" },
	{ Info, "INFO" },
	{ Warning, "WARNING" },
	{ Error, "ERROR" },
	{ Fault, "FAULT" },
	};
}

nex::LogLevel nex::stringToLogLevel(const std::string& str)
{
	return util::stringToEnum(str, nex::util::converter);
}

std::ostream& operator<<(std::ostream& os, const nex::LogLevel& level)
{
	using namespace nex;

	switch (level)
	{
	case Debug: os << "[DEBUG]"; break;
	case Info: os << "[INFO]"; break;
	case Warning: os << "[WARNING]"; break;
	case Error: os << "[ERROR]"; break;
	case Fault: os << "[FAULT]"; break;
	default: break;
	}
	return os;
}
