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

#ifndef LOG_ENDPOINT_HPP
#define LOG_ENDPOINT_HPP
#include <platform/logging/LogMessage.hpp>
#include <memory>

namespace platform
{
	/**
	 * A logging endpoint describes the destination where a logging message should be written to.
	 */
	class LogEndpoint {
	public:

		template <typename T>
		explicit LogEndpoint(T impl);

		LogEndpoint(const LogEndpoint& endpoint);

		LogEndpoint& operator =(LogEndpoint sink);
		bool operator==(const LogEndpoint& sink) const;
		void forward(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) const;
	private:
		struct Concept {
			virtual ~Concept() {}
			virtual Concept* clone() const = 0;
			virtual void operator()(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) = 0;

			virtual void forward(
				const std::string& prefix,
				const LogMessage::Meta& meta,
				const std::string& message
				) = 0;
		};

		template <typename T>
		struct Model : Concept {

			Model(T impl);

			virtual Concept* clone() const override;

			virtual void operator()(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) override;

			void forward(const std::string& prefix, const LogMessage::Meta& meta, const std::string& message) override;

			T mImpl;
		};

		std::unique_ptr<Concept> mWrapper;
	};

	LogEndpoint makeConsoleSink();

	LogEndpoint makeFileSink(const std::string& filename);
}

#include <platform/logging/LogEndpoint.inl>
#endif