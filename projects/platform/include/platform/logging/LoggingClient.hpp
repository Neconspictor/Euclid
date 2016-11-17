#ifndef PLATFORM_LOGGING_LOGGING_CLIENT_HPP
#define PLATFORM_LOGGING_LOGGING_CLIENT_HPP
#include <string>
#include <vector>
#include <boost/current_function.hpp>
#include <platform/logging/LogMessage.hpp>
#include <platform/logging/LogEndpoint.hpp>
#include <platform/logging/LoggingServer.hpp>
#include <platform/logging/LogLevel.hpp>

/**
 * Macro for logging into a logging client. The user specifies the logging client and the wished log level.
 * Additionally to that, the logging client is feed by the current file, function and line the logging is done.
 */
#define LOG(loggingClient, logLevel) \
	loggingClient(logLevel, __FILE__, \
	BOOST_CURRENT_FUNCTION, \
	__LINE__)


namespace platform
{	
	/**
	 * A logging client brokers logging messages and logging endpoints to a logging server which is responsible for executing the
	 * logging. 
	 */
	class LoggingClient
	{
	public:
		/**
		* Constructs a new logging client that will send logging messages to the specified logging server.
		*/
		LoggingClient(const std::weak_ptr<LoggingServer>& logger, bool useConsoleEndpoint = true, 
			bool useFileEndpoint = true);

		/**
		* Copy constructor.
		*/
		LoggingClient(const LoggingClient& other);

		/**
		* Move constructor.
		*/
		LoggingClient(LoggingClient&& other);

		virtual ~LoggingClient();


		LogMessage __cdecl operator()(LogLevel level, const std::string& file, const std::string& function, int line) const;

		LoggingClient& operator=(const LoggingClient&);


		/** 
		 * Adds a new endpoint that this logging client. All receiving logging messages
		 * will be logged to that endpoint.
		 */
		void add(const LogEndpoint& endpoint);

		/**
		* Sends a specific logging message to the logging server. The logging server logs than the registered logging endpoints.
		*/
		void flush(const LogMessage& message) const;

		/**
		* Provides immutable access to the registered logging endpoints.
		*/
		const std::vector<LogEndpoint>& getEndpoints() const;

		/**
		 * Provides the current prefix, that all logging mesages will have brokered by this logging client.
		 */
		const std::string& getPrefix() const;


		/**
		* Checks, if logging messages having the given log level, will be processed by this logging client.
		* If this function returns false, the given log level is ignored.
		*/
		bool isActive(LogLevel level) const;

		/**
		 * Removes a logging endpoint.
		 * NOTE: This function will throw a runtime error, 
		 * if the endpoint isn't registered to this logging client!
		 */
		void remove(const LogEndpoint& endpoint);

		/**
		* Sets the minimimal log level. All log messages that have a lower priority log level won't be processed
		* by this logging client.
		*/
		void setLogLevel(LogLevel newLevel);

		/**
		* A logging client will put the specified prefix in front of all logging messages, thus 
		* customizing the logging messages.
		*/
		void setPrefix(const std::string& prefix);

	private:
		std::string prefix;
		LogLevel currentLogLevel;

		std::vector<LogEndpoint> endpoints;
		const std::weak_ptr<LoggingServer> server;
	};
}

#endif