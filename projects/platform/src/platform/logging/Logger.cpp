#include <platform/logging/Logger.hpp>
#include <platform/logging/LogSink.hpp>
#include <platform/logging/LoggingClient.hpp>

using namespace std;

namespace platform
{
	void Logger::send(const LoggingClient& client, const LogMessage& message) const {
		//string asString = message.mBuffer.str();
		//string time = util::getCurrentTime();
		//cout << time << "[" << message.mMeta.mFunction << "] " << asString << endl; // auto-append newlines

		auto&& sinks = client.getSinks();
		auto&& meta = message.meta;
		auto msg = message.buffer.str();
		auto&& prefixCpy = client.getPrefix();

		mActive->send([=] {
			for (auto&& sink : sinks)
				sink.forward(prefixCpy, meta, msg);
		});
	}

	Logger::Logger()
	{
		mActive = util::Active::create();
	}

	void Logger::terminate() const
	{
		mActive->terminate();
	}

	Logger* Logger::getInstance()
	{
		static Logger logger;
		return &logger;
	}
}