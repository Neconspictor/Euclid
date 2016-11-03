#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <platform/util/Util.hpp>
#include <platform/logging/LogMessage.hpp>
#include <boost/current_function.hpp>
#include <platform/logging/LogSink.hpp>
#include <platform/logging/LogLevel.hpp>
#include <platform/util/concurrent/Active.hpp>

#define LOG(logger, logLevel) \
	logger(logLevel, __FILE__, \
	BOOST_CURRENT_FUNCTION, \
	__LINE__)


/*#define logDetailed(logger, logLevel) \
logger(logLevel, util::relativePathToBuildDirectory(__FILE__), \
util::stringFromRegex(BOOST_CURRENT_FUNCTION, "(.* .* )(.*\\(.*)", 1), \
__LINE__)*/

/*#define log(x) x("", \
	util::stringFromRegex(BOOST_CURRENT_FUNCTION, "(.* .* )(.*\\(.*)", 1), \
	__LINE__)*/
namespace platform
{
	class Logger {
	public:
		LogMessage __cdecl operator()(LogLevel level, const std::string& file, const std::string& function, int line) const;

		void flush(const LogMessage& message) const;

		Logger();

		void add(const LogSink& sink);
		void remove(const LogSink& sink);

		void setPrefix(const std::string& prefix);

		void setLogLevel(LogLevel newLevel);

		bool isActive(LogLevel level) const;

		void terminate() const;

	private:
		std::vector<LogSink> mSinks;
		LogLevel currentLogLevel;
		std::unique_ptr<util::Active> mActive;
		std::string prefix;
	};
}

#endif LOGGER_HPP