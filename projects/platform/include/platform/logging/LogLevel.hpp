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

#ifndef PLATFORM_LOG_LEVEL_HPP
#define PLATFORM_LOG_LEVEL_HPP
#include <ostream>
#include <platform/util/StringUtils.hpp>

namespace platform{

	/**
	 * A log level specifies the priority of a log message resp. how important a log message is.
	 */
	enum LogLevel
	{
		/**
		 * Use this level, if the log message has a debug purpose.
		 */
		Debug = 0,

		/**
		 * Use this level to log common informations
		 */
		Info,

		/**
		 * Use this level to log something that isn't technically an error but maybe is something
		 * unexpected or potentially dangerous.
		 */
		Warning,

		/**
		 * Use this level if an error occurred, but this situation can be handled. 
		 */
		Error,

		/**
		 * Use this level if an error occurred from there no meaningful recovering is possible.
		 */
		Fault
	};

	/**
	 * Maps log levels to a string representation.
	 */
	const static util::EnumString<LogLevel> converter[] = {
		{ Debug, "DEBUG" },
		{ Info, "INFO" },
		{ Warning, "WARNING" },
		{ Error, "ERROR" },
		{ Fault, "FAULT" },
	};

	/**
	 * Maps a string to log level enumeration. 
	 * @param str: The string to be mapped.
	 * @return: The mapped log level
	 *
	 * ATTENTION: If the string can't be mapped to a log level,
	 * a EnumFormatException is thrown!
	 */
	static LogLevel stringToLogLevel(const std::string& str)
	{
		return stringToEnum(str, converter);
	}
}

/**
 * the overloaded streaming operator enables to put a string representation of a log level
 * into an output stream. The output stream has to support std::string
 */
std::ostream& operator << (std::ostream& os, const platform::LogLevel& level);

#endif