#include "Log.hpp"

using namespace ext;

std::ostream& operator<<(std::ostream& os, ext::LogLevel type)
{
	switch(type)
	{
		case LogLevel::Debug:	os << "Debug";		break;
		case LogLevel::Info:		os << "Info";		break;
		case LogLevel::Warning:	os << "Warning";	break;
		case LogLevel::Error:	os << "Error";		break;
		case LogLevel::Fault:	os << "Fault";		break;
		default: ;
	}

	return os;
}

LogMessage::LogMessage(const Logger* logger, LogLevel type, const char* file, const char* function, int line) :
	mLogger(logger), mType(type)
{
	const char* prefix = mLogger->getPrefix();

	mBuffer << "[" << mType << "]" << "[" << prefix << "]" << "(" << file << "," << function << ", line " << line << "): ";
	meta.level = type;
	meta.mFile = file;
	meta.mFunction = function;
	meta.mLine = line;
}

LogMessage::LogMessage(const Logger* logger, LogLevel type) : mLogger(logger), mType(type)
{
	const char* prefix = mLogger->getPrefix();
	mBuffer << "[" << mType << "]" << "[" << prefix << "]: ";
	meta.level = type;
	meta.mFile = "";
	meta.mFunction = "";
	meta.mLine = -1;
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

Logger::Logger(unsigned char mask) : mPrefix(""), mLogMask(Always  | mask)
{
}

Logger::Logger(const char* prefix, unsigned char mask) : mPrefix(prefix), mLogMask(Always | mask)
{
}

Logger::~Logger()
{
}

void Logger::log(const char* msg, LogLevel level) const
{
	if (!(mLogMask & level)) {
		return;
	}

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

unsigned char Logger::getLogMask() const noexcept
{
	return mLogMask;
}

void Logger::setLogMask(unsigned char mask) noexcept
{
	this->mLogMask = Always | mask;
}

LogMessage Logger::operator()(const char* file, const char* function, int line, LogLevel type) const
{
	return log(file, function, line, type);
}

LogMessage Logger::operator()(LogLevel type) const
{
	return log(type);
}

LogMessage Logger::log(const char* file, const char* function, int line, LogLevel type) const
{
	return LogMessage(this, type, file, function, line);
}

LogMessage Logger::log(LogLevel type) const
{
	return LogMessage(this, type);
}

LogSink::LogSink()
{
	registerStream(&std::cout);
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