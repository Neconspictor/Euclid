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

	class LogMessage {
	public:
		struct Meta {
			LogLevel level;
			std::string mFile;
			std::string mFunction;
			int mLine;
		};

		friend class LoggingServer;
		friend class LoggingClient; // only allow the Logger and LoggingClient classes to instantiate a LogMessage
		template <typename T>
		LogMessage& operator << (const T& value);
		LogMessage& operator << (std::ostream& (*fn)(std::ostream& os));

		~LogMessage();

	private:

		std::ostringstream buffer;
		const LoggingClient* client;
		Meta meta;

		LogMessage(LogMessage&);
		LogMessage(LogMessage&&);
		LogMessage(LogLevel level, const std::string& file, const std::string& function, int line, const LoggingClient* owner);
		LogMessage();
	};

	template <typename T>
	LogMessage& LogMessage::operator << (const T& value) {
		buffer << value;
		return *this;
	}
}

#endif
