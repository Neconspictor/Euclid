#pragma once
#pragma optimize( "", on )

#include <platform/util/Util.hpp>
#include <platform/logging/LogMessage.hpp>
#include <platform/util/concurrent/Active.hpp>
#include <atomic>
#include <platform/logging/LogEndpoint.hpp>

namespace platform
{
	/**
	 * A logging facility class, that provides central access to a logging server.
	 * This class is designed to be thread safe and provides its functionality in a non blocking 
	 * fashion if not the opposite is explicitly stated by one of its member functions. 
	 * Internally it uses one seperate thread through an active object (see also
	 * platform/util/concurrent/Active.hpp). 
	 */
	class LoggingServer {
	public:

		/**
		 * Creates and starts a new logging thread.
		 */
		LoggingServer();

		/**
		* Releases any allocated memory and shuts down the internal logging thread.
		* All queued log messages will be processed before shuting down, but no new
		* log requests (after the call of this function) will be processed.
		* NOTE: This function is blocking!
		*/
		virtual ~LoggingServer();

		/**
		 * Checks if a message with a given log level is processed by this log server.
		 */
		bool isActive(const LogLevel level) const;

		/**
				 * Sends a log message from a given client to this server. The server will
				 * process the log message using the endpoints added to the client.
				 * @param message : The message to be logged
				 * @param client : The logging client
				 */
		void send(const LoggingClient& client, const LogMessage& message) const;

		/**
		 * Sets the minimum log level. All messages having a log level smaller the specified log level
		 * won't be processed anymore.
		 */
		void setMinLogLevel(LogLevel level);
		
		/**
		 * Terminates this logging server immediately. 
		 * Any queued log messages will be discarded!
		 */
		void terminate() const;

		/**
		 * A log server manages a file log endpoint.
		 * This methods grants read access to this endpoint.
		 */
		const LogEndpoint& getFileEndpoint();

		/**
		* A log server manages a console log endpoint.
		* This methods grants read access to this endpoint.
		*/
		const LogEndpoint& getConsoleEndpoint();

	private:
		std::unique_ptr<util::Active> active;
		std::atomic<LogLevel> minLogLevel;
		LogEndpoint fileEndpoint;
		LogEndpoint consoleEndpoint;
	};
}