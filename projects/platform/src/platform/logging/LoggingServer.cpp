#include <platform/logging/LoggingServer.hpp>
#include <platform/logging/LogEndpoint.hpp>
#include <platform/logging/LoggingClient.hpp>

using namespace std;


namespace platform
{
	void LoggingServer::send(const LoggingClient& client, const LogMessage& message) const {
		auto&& endpoints = client.getEndpoints();
		auto&& meta = message.meta;
		auto msg = message.buffer.str();
		auto&& prefixCpy = client.getPrefix();

		active->send([=] {
			for (auto&& endpoint : endpoints)
				endpoint.log(prefixCpy, meta, msg);
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