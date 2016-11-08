#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <platform/util/Util.hpp>
#include <platform/logging/LogMessage.hpp>
#include <platform/util/concurrent/Active.hpp>

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
		 * Sends a log message from a given client to this server. The server will
		 * process the log message using the LogSinks added to the client.
		 * @param message : The message to be logged
		 * @param client : The logging client
		 */
		void send(const LoggingClient& client, const LogMessage& message) const;
		
		/**
		 * Terminates this logging server immediately. 
		 * Any queued log messages will be discarded!
		 */
		void terminate() const;

	private:
		std::unique_ptr<util::Active> active;
	};
}

#endif LOGGER_HPP