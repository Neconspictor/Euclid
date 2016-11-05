#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <platform/util/Util.hpp>
#include <platform/logging/LogMessage.hpp>
#include <platform/util/concurrent/Active.hpp>

namespace platform
{
	class Logger {
	public:
		void send(const LoggingClient& client, const LogMessage& message) const;
		void terminate() const;

		static Logger* getInstance();


	private:
		std::unique_ptr<util::Active> mActive;

		Logger(); // singleton class needs private constructor
	};
}

#endif LOGGER_HPP