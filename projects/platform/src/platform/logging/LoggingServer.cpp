#include <platform/logging/LoggingServer.hpp>
#include <platform/logging/LogSink.hpp>
#include <platform/logging/LoggingClient.hpp>

using namespace std;


namespace platform
{
	void LoggingServer::send(const LoggingClient& client, const LogMessage& message) const {
		auto&& sinks = client.getSinks();
		auto&& meta = message.meta;
		auto msg = message.buffer.str();
		auto&& prefixCpy = client.getPrefix();

		active->send([=] {
			for (auto&& sink : sinks)
				sink.forward(prefixCpy, meta, msg);
		});
	}

	LoggingServer::LoggingServer()
	{
		active = util::Active::create();
	}

	void LoggingServer::terminate() const
	{
		active->terminate();
	}

	LoggingServer::~LoggingServer()
	{
		//destructor of active will block till of queued 
		// log messages are processed!
	}
}