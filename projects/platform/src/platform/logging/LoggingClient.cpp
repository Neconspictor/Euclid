#include <platform/logging/LoggingClient.hpp>
#include <platform/logging/Logger.hpp>

using namespace std;

namespace platform
{
	LoggingClient::LoggingClient(Logger* server) :
		prefix(""), currentLogLevel(Debug), server(server){}

	LoggingClient::LoggingClient(const LoggingClient& other): 
		currentLogLevel(other.currentLogLevel), prefix(other.prefix),
		server(other.server)
	{
		for (auto sink : other.sinks)
			sinks.push_back(sink);
	}

	LoggingClient::LoggingClient(LoggingClient&& other) :
		currentLogLevel(other.currentLogLevel), prefix(other.prefix),
		server(other.server), sinks(other.sinks)
	{
		other.sinks.clear();
	}
}