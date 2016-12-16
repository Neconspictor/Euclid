#pragma once
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace platform {
	namespace util {

		/**
		 * Provides the current time in the following format:
		 * [hours:minutes:seconds]
		 */
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