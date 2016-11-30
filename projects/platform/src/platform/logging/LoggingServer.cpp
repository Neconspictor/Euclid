#include <platform/logging/LoggingServer.hpp>
#include <platform/logging/LogEndpoint.hpp>
#include <platform/logging/LoggingClient.hpp>

using namespace std;


namespace platform
{
	void LoggingServer::send(const LoggingClient& client, const LogMessage& message) const {
		if (!isActive(message.meta.level)) return;
		auto&& endpoints = client.getEndpoints();
		auto&& meta = message.meta;
		auto msg = message.buffer.str();
		auto&& prefixCpy = client.getPrefix();

		/*for (auto&& endpoint : endpoints)
			endpoint.log(prefixCpy, meta, msg);*/

		active->send([=] {
			for (auto&& endpoint : endpoints)
				endpoint.log(prefixCpy, meta, msg);
		});
	}

	void LoggingServer::setMinLogLevel(LogLevel level)
	{
		minLogLevel = level;
	}

	LoggingServer::LoggingServer() : 
		fileEndpoint(makeFileEndpoint("log.log")),
		consoleEndpoint(makeConsoleEndpoint())
	{
		active = util::Active::create();
		minLogLevel = Debug;
	}

	void LoggingServer::terminate() const
	{
		active->terminate();
	}

	const LogEndpoint & LoggingServer::getFileEndpoint()
	{
		return fileEndpoint;
	}

	const LogEndpoint & LoggingServer::getConsoleEndpoint()
	{
		return consoleEndpoint;
	}

	LoggingServer::~LoggingServer()
	{
		//destructor of active will block till of queued 
		// log messages are processed!
	}
	bool LoggingServer::isActive(const LogLevel level) const
	{
		return minLogLevel <= level;
	}
}