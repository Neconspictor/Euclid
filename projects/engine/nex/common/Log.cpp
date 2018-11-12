#include "Log.hpp"

std::ostream& operator<<(std::ostream& os, LogType type)
{
	switch(type)
	{
		case LogType::Debug:	os << "Debug";		break;
		case LogType::Info:		os << "Info";		break;
		case LogType::Warning:	os << "Warning";	break;
		case LogType::Error:	os << "Error";		break;
		case LogType::Fault:	os << "Fault";		break;
		default: ;
	}

	return os;
}

LogMessage::LogMessage(const Logger* logger, LogType type, const char* file, const char* function, int line) :
	mLogger(logger), mType(type)
{
	const char* prefix = mLogger->getPrefix();

	mBuffer << "[" << mType << "]" << "[" << prefix << "]" << "(" << file << "," << function << ", line " << line << "): ";
}

LogMessage::LogMessage(const Logger* logger, LogType type) : mLogger(logger), mType(type)
{
	const char* prefix = mLogger->getPrefix();
	mBuffer << "[" << mType << "]" << "[" << prefix << "]: ";
}

LogMessage::~LogMessage()
{
	if (mLogger)
	{
		mLogger->log(mBuffer.str().c_str());
	}
	mLogger = nullptr;
}

LogMessage& LogMessage::operator<<(std::ostream&(* F)(std::ostream&))
{
	F(mBuffer); 
	return *this;
}

Logger::Logger() : mPrefix("")
{
}

Logger::Logger(const char* prefix) : mPrefix(prefix)
{
}

Logger::~Logger()
{
}

void Logger::log(const char* msg) const
{
	for (auto stream : LogSink::get()->getLogStreams())
		*stream << msg << std::endl;
}

void Logger::setPrefix(const char* prefix)
{
	if (strcmp(prefix, "") != 0)
	{
		mPrefix.assign(prefix);
		mPrefix.push_back(' ');
	}
}

const char* Logger::getPrefix() const
{
	return mPrefix.c_str();
}

LogMessage Logger::operator()(const char* file, const char* function, int line, LogType type) const
{
	return log(file, function, line, type);
}

LogMessage Logger::operator()(LogType type) const
{
	return log(type);
}

LogMessage Logger::log(const char* file, const char* function, int line, LogType type) const
{
	return LogMessage(this, type, file, function, line);
}

LogMessage Logger::log(LogType type) const
{
	return LogMessage(this, type);
}

LogSink* LogSink::get()
{
	static LogSink sink;
	return &sink;
}

void LogSink::registerStream(std::ostream* stream)
{
	mStreams.push_back(stream);
}

const std::vector<std::ostream*>& LogSink::getLogStreams() const
{
	return mStreams;
}