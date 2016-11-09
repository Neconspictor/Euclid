/*
* Modified code from: https://github.com/IanBullard/event_taskmanager
*
* Copyright (c) 2014 GrandMaster (gijsber@gmail)
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef LOG_LEVEL_HPP
#define LOG_LEVEL_HPP

#include <platform/logging/LogLevel.hpp>
#include <sstream>

namespace platform
{
	class LoggingClient;

	/**
	 * A log message is a facility for logging string messages. It uses overlaoded streaming operators (<<) for allowing
	 * to log any object type, that can be streamed to a string.
	 * the 
	 */
	class LogMessage {
	public:
		/**
		 * A log message contains meta information: the log (priority) level, the file and the function and
		 * the line the logging is taken on.
		 */
		struct Meta {
			LogLevel level;
			std::string mFile;
			std::string mFunction;
			int mLine;
		};

		/**
		 * Templated overloaded streaming operator for forwarding objects of any type to the log message.
		 */
		template <typename T>
		LogMessage& operator << (const T& value);

		LogMessage& operator << (std::ostream& (*fn)(std::ostream& os));

		~LogMessage();

	private:

		// only allow the logging client and server classes to instantiate a LogMessage
		friend class LoggingServer;
		friend class LoggingClient;

		std::ostringstream buffer;
		const LoggingClient* client;
		Meta meta;

		/**
		 * Copy constructor
		 */
		LogMessage(LogMessage&);
		
		/**
		* Move constructor
		*/
		LogMessage(LogMessage&&);
		
		/**
		* Constructs a new log message using a given log level, file and function name and the line number for
		* meta information. The specified logging client is considered to be the owner of this log message, that 
		* will also handle it.
		*
		* NOTE: This constructor is private, as the creation of log messages is done by the logging clients. 
		*/
		LogMessage(LogLevel level, const std::string& file, const std::string& function, int line, const LoggingClient* owner);
		
		/**
		* Default constructor
		*/
		LogMessage();
	};

	template <typename T>
	LogMessage& LogMessage::operator << (const T& value) {
		buffer << value;
		return *this;
	}
}

#endif