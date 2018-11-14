#pragma once
#include <string>
#include <sstream>
#include <vector>


#define LOG_EXT(Logger, type) Logger(__FILE__, __func__, __LINE__, type)

namespace ext
{
	class Logger;

	enum LogLevel
	{
		Always = 1 << 0,
		Debug = 1 << 1,
		Info = 1 << 2,
		Warning = 1 << 3,
		Error =  1 << 4,
		Fault = 1 << 5
	};

	struct LogMessage
	{
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


		LogMessage(const Logger* logger, LogLevel type, const char* file, const char* function, int line);
		LogMessage(const Logger* logger, LogLevel type);
		~LogMessage();

		/**
		* Templated overloaded streaming operator for forwarding objects of any type to the log message.
		*/
		template <typename T>
		LogMessage& operator << (const T& value);
		template <typename T>
		LogMessage& operator << (const T* value);
		LogMessage& operator<<(std::ostream& (*F)(std::ostream&));


		std::stringstream mBuffer;
		LogLevel mType;
		const Logger* mLogger;
		Meta meta;
	};

	class Logger
	{
	public:

		Logger(unsigned char mask = Always);
		explicit Logger(const char* prefix, unsigned char mask = Always);

		virtual ~Logger();
		virtual void log(const char* msg, LogLevel level = Always) const;

		void setPrefix(const char* prefix);

		const char* getPrefix() const;

		unsigned char getLogMask() const noexcept;

		void setLogMask(unsigned char mask) noexcept;

		LogMessage operator()(const char* file, const char* function, int line, LogLevel type = LogLevel::Info) const;
		LogMessage operator()(LogLevel type = LogLevel::Info) const;

		virtual LogMessage log(const char* file, const char* function, int line, LogLevel type = LogLevel::Info) const;
		virtual LogMessage log(LogLevel type = LogLevel::Info) const;

	private:

		friend LogMessage;

		std::string mPrefix;
		unsigned char mLogMask;
	};

	class LogSink
	{
	public:

		LogSink();

		static LogSink* get();
		void registerStream(std::ostream* stream);
		const std::vector<std::ostream*>& getLogStreams() const;
		std::vector<std::ostream*> mStreams;
	};

	class LogManager
	{
		
	};

	template <typename T>
	LogMessage& LogMessage::operator << (const T& value) {
		mBuffer << value;
		return *this;
	}

	template <typename T>
	LogMessage& LogMessage::operator << (const T* value) {
		mBuffer << value;
		return *this;
	}
}

std::ostream& operator<<(std::ostream& os, ext::LogLevel type);