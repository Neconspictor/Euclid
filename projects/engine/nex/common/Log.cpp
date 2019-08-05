#include "Log.hpp"
#include <nex/util/StringUtils.hpp>

using namespace nex;

namespace nex::util
{
	/**
	* Maps log levels to a string representation.
	*/
	const static EnumString<LogLevel> converter[] = {
	{ Always, "ALWAYS" },
	{ Debug, "DEBUG" },
	{ Info, "INFO" },
	{ Warning, "WARNING" },
	{ Error, "ERROR" },
	{ Fault, "FAULT" },
	};
}

std::ostream& operator<<(std::ostream& os, nex::LogLevel type)
{
	switch(type)
	{
	case LogLevel::Always:	os << nex::util::converter[0].strValue;		break;
		case LogLevel::Debug:	os << nex::util::converter[1].strValue;		break;
		case LogLevel::Info:		os << nex::util::converter[2].strValue;		break;
		case LogLevel::Warning:	os << nex::util::converter[3].strValue;	break;
		case LogLevel::Error:	os << nex::util::converter[4].strValue;		break;
		case LogLevel::Fault:	os << nex::util::converter[5].strValue;		break;
		default: ;
	}

	return os;
}

LogLevel nex::stringToLogLevel(const std::string& str)
{
	return stringToEnum(str, util::converter);
}

LogMessage::LogMessage(const Logger* logger, LogLevel type, const char* file, const char* function, int line) :
	mLogger(logger), mType(type)
{
	if (logger->isActive(mType))
	{
		const char* prefix = mLogger->getPrefix();
		Meta meta;
		meta.file = file;
		meta.function = function;
		meta.line = line;
		formatMessageHeader(type, prefix, &meta, &mBuffer);
	}
}

LogMessage::LogMessage(const Logger* logger, LogLevel type, Meta meta) : mLogger(logger), mType(type)
{
	if (logger->isActive(mType))
	{
		const char* prefix = mLogger->getPrefix();
		formatMessageHeader(type, prefix, &meta, &mBuffer);
	}
}

LogMessage::LogMessage(const Logger* logger, LogLevel type) : mLogger(logger), mType(type)
{
	if (logger->isActive(mType))
	{
		const char* prefix = mLogger->getPrefix();
		Meta meta;
		formatMessageHeader(type, prefix, &meta, &mBuffer);
	}
}

LogMessage::~LogMessage()
{
	if (mLogger && mLogger->isActive(mType))
	{
		mLogger->log(mBuffer.str().c_str(), mType);
	}
	mLogger = nullptr;
}

LogMessage& LogMessage::operator<<(std::ostream&(* F)(std::ostream&))
{
	F(mBuffer); 
	return *this;
}

bool LogMessage::hasOneItemSet(Meta* meta)
{
	return meta->file || meta->function || meta->line;
}

void LogMessage::unsetItem(Meta* meta, unsigned index)
{
	switch (index)
	{
	case 0: meta->file.reset(); return;
	case 1: meta->function.reset(); return;
	case 2: meta->line.reset(); return;

	default: ;
	}
}

void LogMessage::formatMessageHeader(LogLevel level, const char* prefix, Meta* meta, std::stringstream* buffer)
{
	*buffer << "[" << prefix << "]" << " [" << level << "]";

	bool minimalOneItem = hasOneItemSet(meta);

	if (minimalOneItem)
		*buffer << "(";


	for (unsigned int i = 0; i < 3; )
	{
		void* content = getOptionalByIndex(meta, i);
		if (content != nullptr)
		{
			if (i < 2)
				*buffer << *(std::string*)content;
			else
				*buffer << *(int*)content;
		}

		unsetItem(meta, i);

		++i;

		if (i == 3)
		{
			if (minimalOneItem)
				*buffer << ")";
			*buffer << ": ";
		} else if (hasOneItemSet(meta))
		{
			*buffer << ", ";
		}
	}
}

void* LogMessage::getOptionalByIndex(Meta* meta, unsigned index)
{
	switch(index)
	{
		case 0: return meta->file.get_ptr();
		case 1: return meta->function.get_ptr();
		case 2: return meta->line.get_ptr();
		default: return nullptr;
	}
}

Logger::Logger(unsigned char mask) : mPrefix("")
{
}

Logger::Logger(const char* prefix, unsigned char mask) : mPrefix(prefix)
{
}

Logger::~Logger()
{
}

void Logger::log(const char* msg, LogLevel level) const
{
	if (!isActive(level)) {
		return;
	}

	auto* sink = LogSink::get();

	if (!sink) return;

	for (auto stream : sink->getLogStreams()) {
		if (!stream) continue;
		*stream << msg << std::endl;
	}
		
}

void Logger::setPrefix(const char* prefix)
{
	if (strcmp(prefix, "") != 0)
	{
		mPrefix.assign(prefix);
	}
}

const char* Logger::getPrefix() const
{
	return mPrefix.c_str();
}

bool Logger::isActive(LogLevel level) const
{
	return LoggerManager::get()->getLogMask() & level;
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

LogMessage Logger::log(const char* file, LogMessage::Meta meta, LogLevel type) const
{
	return LogMessage(this, type, std::move(meta));
}

LogMessage Logger::log(LogLevel type) const
{
	return LogMessage(this, type);
}

LoggerManager* LoggerManager::get()
{
	static LoggerManager instance;
	return &instance;
}

unsigned char LoggerManager::getLogMask() const
{
	return mLogMask;
}

void LoggerManager::setLogMask(unsigned char mask)
{
	mLogMask = Always | mask;
}

void LoggerManager::setMinLogLevel(LogLevel level)
{
	mLogMask = Always;
	unsigned char end = LogLevel::Fault + 1;

	for (unsigned char it = Debug; it <= Fault; it = it << 1)
	{
		if (it >= level) mLogMask |= it;
	}
}

LoggerManager::LoggerManager() : mLogMask(Always)
{
}

LogSink::LogSink()
{
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