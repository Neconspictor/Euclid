#include <platform/logging/Logger.hpp>
#include <iostream>
#include <platform/logging/LogSink.hpp>

using namespace std;

namespace platform
{
	LogMessage  Logger::operator()(LogLevel level, const string& file, const string& function, int line) const {
		// performs return value optimization. No copy will be created
		if (!isActive(level)) return LogMessage();
		LogMessage result(level, file,
			//util::stringFromRegex(function, "(.* .* )(.*\\(.*)", 1), 
			function,
			line,
			this);

		//return LogMessage (this);
		return result;
	}

	void Logger::flush(const LogMessage& message) const {
		//string asString = message.mBuffer.str();
		//string time = util::getCurrentTime();
		//cout << time << "[" << message.mMeta.mFunction << "] " << asString << endl; // auto-append newlines

		auto&& sinks = mSinks;
		auto&& meta = message.mMeta;
		auto msg = message.mBuffer.str();
		auto&& prefixCpy = prefix;

		if (!isActive(meta.level)) return;

		mActive->send([=] {
			for (auto&& sink : sinks)
				sink.forward(prefixCpy, meta, msg);
		});
	}

	Logger::Logger()
	{
		currentLogLevel = Debug;
		mActive = util::Active::create();
	}

	void Logger::add(const LogSink& sink)
	{
		mSinks.push_back(sink);
	}

	void Logger::remove(const LogSink& sink)
	{
		auto it = find(mSinks.begin(), mSinks.end(), sink);

		if (it == mSinks.end())
			throw runtime_error("Tried to remove a sink that was not added yet");

		mSinks.erase(it);
	}

	void Logger::setPrefix(const string& prefix)
	{
		this->prefix = string(prefix);
	}

	void Logger::setLogLevel(LogLevel newLevel)
	{
		currentLogLevel = newLevel;
	}

	bool Logger::isActive(LogLevel level) const
	{
		return level >= currentLogLevel;
	}

	void Logger::terminate() const
	{
		mActive->terminate();
	}
}