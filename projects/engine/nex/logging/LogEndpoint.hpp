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

#pragma once
#pragma optimize( "", on )

#include <nex/logging/LogMessage.hpp>
#include <memory>

namespace nex
{
	/**
	 * A logging endpoint describes the destination where a logging message should be written to.
	 */
	class LogEndpoint {
	public:

		/**
		 * Creates a new logging endpoint of type T. T is suppossed to be a class or function, on that the following
		 * operator can be called: 
		 * ---------------------------------------------------------------------------------------------------------
		 * void operator()(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message);
		 *----------------------------------------------------------------------------------------------------------
		 * This operator handles the logging of a log message.
		 * Using templates enables a user also to provide lambda expressions for creating a LogEndpoint.
		 * E.g. a user can do the following for creating a new logging endpoint:
		 * LogEndpoint myLogEndpoint([] (const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) {
		 * //logging stuff...
		 * });
		 */
		template <typename T>
		explicit LogEndpoint(T impl);

		/**
		 * Copy constructor
		 */
		LogEndpoint(const LogEndpoint& endpoint);

		/**
		* Assignment operator
		*/
		LogEndpoint& operator =(LogEndpoint endpoint);

		/**
		 * Comparator operator
		 */
		bool operator==(const LogEndpoint& endpoint) const;

		/**
		 * Logs a message with some meta information and a prefix.
		 */
		void log(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) const;
	private:

		/**
		 * Abstraction class for wrapping the implementation of the logging endpoint.
		 */
		struct Wrapper {
			virtual ~Wrapper() {}
			virtual std::unique_ptr<Wrapper> clone() const = 0;
			//virtual void operator()(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) = 0;

			/**
			* Forwards the log message to the implementation.
			*/
			virtual void forward(
				const std::string& prefix,
				const LogMessage::Meta& meta,
				const std::string& message
				) = 0;
		};

		/**
		 * The used implementation of the wrapper abstraction class used internally in the LogEndpoint class.
		 * 
		 */
		template <typename T>
		struct WrapperImpl : Wrapper {

			WrapperImpl(T impl);

			/**
			 * Creates a deep copy of this wrapper object.
			 */
			std::unique_ptr<Wrapper> clone() const override;

			//virtual void operator()(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) override;

			/**
			 * @coypdoc Wrapper::forward(const std::string&, const LogMessage::Meta&, const std::string&)
			 */
			void forward(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) override;

			T mImpl;
		};

		std::unique_ptr<Wrapper> mWrapper;
	};

	/**
	 * Creates a logging endpoint that writes log messages to the standard output stream (std::cout).
	 */
	LogEndpoint makeConsoleEndpoint();

	/**
	* Creates a logging endpoint that writes log messages to a file named by the parameter fileName.
	* All log messages will be appended to the end of the file. 
	* NOTE: If the file already exists the old file will be overwritten.
	*/
	LogEndpoint makeFileEndpoint(const std::string& fileName);
}

#include <nex/logging/LogEndpoint.inl>