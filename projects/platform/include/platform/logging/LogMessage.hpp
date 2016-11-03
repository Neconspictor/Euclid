#ifndef LOG_LEVEL_HPP
#define LOG_LEVEL_HPP

#include <platform/logging/LogLevel.hpp>
#include <sstream>

namespace platform
{
	class LogMessage {
	public:
		struct Meta {
			LogLevel level;
			std::string mFile;
			std::string mFunction;
			int mLine;
		};

		friend class Logger; // only allow the Logger class to instantiate a LogMessage
		template <typename T>
		LogMessage& operator << (const T& value);
		LogMessage& operator << (std::ostream& (*fn)(std::ostream& os));

		~LogMessage();

	private:

		std::ostringstream mBuffer;
		const Logger* mOwner;
		Meta mMeta;

		LogMessage(LogMessage&);
		LogMessage(LogMessage&&);
		LogMessage(LogLevel level, const std::string& file, const std::string& function, int line, const Logger* owner);
		LogMessage();
	};

	template <typename T>
	LogMessage& LogMessage::operator << (const T& value) {
		mBuffer << value;
		return *this;
	}
}

#endif
