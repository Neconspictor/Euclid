#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <boost/optional/optional.hpp>


#define LOG(Logger, type) Logger(__FILE__, __func__, __LINE__, type)

namespace nex
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

	/**
	 * Maps a string to log level enumeration.
	 * @param str: The string to be mapped.
	 * @return: The mapped log level
	 *
	 * ATTENTION: If the string can't be mapped to a log level,
	 * a EnumFormatException is thrown!
	 */
	LogLevel stringToLogLevel(const std::string& str);

	struct LogMessage
	{
		/**
		 * A log message contains meta information: the log (priority) level, the file and the function and
		 * the line the logging is taken on.
		 */
		struct Meta {
			boost::optional<std::string> file;
			boost::optional<std::string> function;
			boost::optional<int> line;
		};


		LogMessage(const Logger* logger, LogLevel type, const char* file, const char* function, int line);
		LogMessage(const Logger* logger, LogLevel type, Meta meta);
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

	private:
		static bool hasOneItemSet(Meta* meta);
		static void unsetItem(Meta* meta, unsigned index);
		static void formatMessageHeader(LogLevel level, const char* prefix, Meta* meta, std::stringstream* buffer);
		static void* getOptionalByIndex(Meta* meta, unsigned int index);
	};

	class Logger
	{
	public:

		Logger(unsigned char mask = Always);
		explicit Logger(const char* prefix, unsigned char mask = Always);

		virtual ~Logger();

		//unsigned char getLogMask() const noexcept;
		const char* getPrefix() const;
		bool isActive(LogLevel level) const;
		virtual void log(const char* msg, LogLevel level = Always) const;
		//void setLogMask(unsigned char mask) noexcept;
		//void setMinLogLevel(LogLevel level);
		void setPrefix(const char* prefix);

		LogMessage operator()(const char* file, const char* function, int line, LogLevel type = LogLevel::Info) const;
		LogMessage operator()(LogLevel type = LogLevel::Info) const;

		virtual LogMessage log(const char* file, const char* function, int line, LogLevel type = LogLevel::Info) const;
		virtual LogMessage log(const char* file, LogMessage::Meta meta, LogLevel type = LogLevel::Info) const;
		virtual LogMessage log(LogLevel type = LogLevel::Info) const;

	private:

		friend LogMessage;

		std::string mPrefix;
		//unsigned char mLogMask;
	};

	class LoggerManager
	{
	public:
		static LoggerManager* get();

		unsigned char getLogMask() const;

		void setLogMask(unsigned char mask);
		void setMinLogLevel(LogLevel level);
		

	private:
		LoggerManager();
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

std::ostream& operator<<(std::ostream& os, nex::LogLevel type);