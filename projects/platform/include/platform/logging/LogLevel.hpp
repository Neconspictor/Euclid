#ifndef PLATFORM_LOG_LEVEL_HPP
#define PLATFORM_LOG_LEVEL_HPP
#include <ostream>

namespace platform{
	enum LogLevel
	{
		Debug,
		Info,
		Warning,
		Error,
		Fault
	};
}

std::ostream& operator << (std::ostream& os, const platform::LogLevel& level);

#endif