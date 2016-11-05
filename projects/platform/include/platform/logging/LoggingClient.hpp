#ifndef PLATFORM_LOGGING_LOGGING_CLIENT_HPP
#define PLATFORM_LOGGING_LOGGING_CLIENT_HPP
#include <string>
#include <vector>
#include <boost/current_function.hpp>
#include <platform/logging/LogMessage.hpp>
#include <platform/logging/LogSink.hpp>
#include <platform/logging/Logger.hpp>
#include <platform/logging/LogLevel.hpp>

#define LOG(loggingClient, logLevel) \
	loggingClient(logLevel, __FILE__, \
	BOOST_CURRENT_FUNCTION, \
	__LINE__)


/*#define logDetailed(logger, logLevel) \
logger(logLevel, util::relativePathToBuildDirectory(__FILE__), \
util::stringFromRegex(BOOST_CURRENT_FUNCTION, "(.* .* )(.*\\(.*)", 1), \
__LINE__)*/

/*#define log(x) x("", \
util::stringFromRegex(BOOST_CURRENT_FUNCTION, "(.* .* )(.*\\(.*)", 1), \
__LINE__)*/

namespace platform
{	
	class LoggingClient
	{
	public:
		LogMessage __cdecl operator()(LogLevel level, const std::string& file, const std::string& function, int line) const;

		LoggingClient& operator=(const LoggingClient&);

		void flush(const LogMessage& message) const;

		LoggingClient(Logger* server);
		LoggingClient(const LoggingClient& other);
		LoggingClient(LoggingClient&& other);


		void add(const LogSink& sink);

		const std::string& getPrefix() const;

		void remove(const LogSink& sink);

		void setPrefix(const std::string& prefix);

		void setLogLevel(LogLevel newLevel);

		bool isActive(LogLevel level) const;

		const std::vector<LogSink>& getSinks() const;

	private:
		std::string prefix;
		LogLevel currentLogLevel;
		std::vector<LogSink> sinks;
		Logger* server;
	};
}

#endif