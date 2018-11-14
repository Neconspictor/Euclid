#include "Log.hpp"

using namespace ext;

std::ostream& operator<<(std::ostream& os, ext::LogLevel type)
{
	switch(type)
	{
		case LogLevel::Always:	os << "ALWAYS";		break;
		case LogLevel::Debug:	os << "DEBUG";		break;
		case LogLevel::Info:		os << "INFO";		break;
		case LogLevel::Warning:	os << "WARNING";	break;
		case LogLevel::Error:	os << "ERROR";		break;
		case LogLevel::Fault:	os << "FAULT";		break;
		default: ;
	}

	return os;
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
	}
}

const char* Logger::getPrefix() const
{
	return mPrefix.c_str();
}

bool Logger::isActive(LogLevel level) const
{
	return mLogMask & level;
}

unsigned char Logger::getLogMask() const noexcept
{
	return mLogMask;
}

void Logger::setLogMask(unsigned char mask) noexcept
{
	this->mLogMask = Always | mask;
}

void Logger::setMinLogLevel(LogLevel level)
{
	mLogMask = Always;
	unsigned char end = LogLevel::Fault + 1;

	for (unsigned char it = Debug; it <= Fault; it = it << 1)
	{
		if (it >= level) mLogMask |= it;
	}
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