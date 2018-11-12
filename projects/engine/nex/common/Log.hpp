#pragma once
#include <string>
#include <sstream>
#include <vector>


#define LOG(Logger, type) Logger(__FILE__, __func__, __LINE__, type)

class Logger;

enum LogType
{
	Debug,
	Info,
	Warning,
	Error,
	Fault
};

std::ostream& operator<<(std::ostream& os, LogType type);

struct LogMessage
{
	LogMessage(const Logger* logger, LogType type, const char* file, const char* function, int line);
	LogMessage(const Logger* logger, LogType type);
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
	LogType mType;
	const Logger* mLogger;
};

class Logger
{
public:

	Logger();
	explicit Logger(const char* prefix);

	virtual ~Logger();
	virtual void log(const char* msg) const;

	void setPrefix(const char* prefix);

	const char* getPrefix() const;

	LogMessage operator()(const char* file, const char* function, int line, LogType type = LogType::Info) const;
	LogMessage operator()(LogType type = LogType::Info) const;

	virtual LogMessage log(const char* file, const char* function, int line, LogType type = LogType::Info) const;
	virtual LogMessage log(LogType type = LogType::Info) const;

private:

	friend LogMessage;

	std::string mPrefix;
};

class LogSink
{
public:

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