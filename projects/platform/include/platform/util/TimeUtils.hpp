#ifndef PLATFORM_UTIL_TIME_UTILS_HPP
#define PLATFORM_UTIL_TIME_UTILS_HPP
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace platform {
	namespace util {
		inline std::string getCurrentTime()
		{
			auto currentTime = time(nullptr);
			tm localTime;
			localtime_s(&localTime, &currentTime);

			std::ostringstream oss;
			oss << std::put_time(&localTime, "[%H:%M:%S]");
			return oss.str();
		}
	}
}


#endif