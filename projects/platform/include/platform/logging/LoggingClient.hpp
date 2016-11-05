#ifndef PLATFORM_LOGGING_LOGGING_CLIENT_HPP
#define PLATFORM_LOGGING_LOGGING_CLIENT_HPP
#include <string>
#include <vector>

namespace platform
{
	class Logger;
	class LogMessage;
	class LogSink;
	enum LogLevel;
	
	class LoggingClient
	{
	public:
		LogMessage __cdecl operator()(LogLevel level, const std::string& file, const std::string& function, int line) const;

		void flush(const LogMessage& message) const;

		LoggingClient(Logger* server);
		LoggingClient(const LoggingClient& other);
		LoggingClient(LoggingClient&& other);


		void add(const LogSink& sink);
		void remove(const LogSink& sink);

		void setPrefix(const std::string& prefix);

		void setLogLevel(LogLevel newLevel);

		bool isActive(LogLevel level) const;

	private:
		std::string prefix;
		LogLevel currentLogLevel;
		std::vector<LogSink> sinks;
		Logger* server;
	};
}

#endif