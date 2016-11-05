#include <platform/logging/LoggingClient.hpp>
#include <platform/logging/Logger.hpp>
#include <platform/logging/LogSink.hpp>

using namespace std;

namespace platform
{
	LogMessage LoggingClient::operator()(LogLevel level, const string& file, const string& function, int line) const
	{
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

	LoggingClient& LoggingClient::operator=(const LoggingClient& other)
	{
		return LoggingClient(other);
	}

	void LoggingClient::flush(const LogMessage& message) const
	{
		if (!isActive(message.meta.level)) return;

		// just forward the message to the logging server. Non blocking!
		server->send(*this, message);
	}

	LoggingClient::LoggingClient(Logger* server) :
		prefix(""), currentLogLevel(Debug), server(server){}

	LoggingClient::LoggingClient(const LoggingClient& other): 
		currentLogLevel(other.currentLogLevel), prefix(other.prefix),
		server(other.server)
	{
		for (auto& sink : other.sinks)
			sinks.push_back(sink);
	}

	LoggingClient::LoggingClient(LoggingClient&& other) :
		currentLogLevel(other.currentLogLevel), prefix(other.prefix),
		server(other.server), sinks(other.sinks)
	{
		other.sinks.clear();
	}

	void LoggingClient::add(const LogSink& sink)
	{
		sinks.push_back(sink);
	}

	const string& LoggingClient::getPrefix() const
	{
		return prefix;
	}

	const vector<LogSink>& LoggingClient::getSinks() const
	{
		return sinks;
	}

	bool LoggingClient::isActive(LogLevel level) const
	{
		return level >= currentLogLevel;
	}

	void LoggingClient::remove(const LogSink& sink)
	{
		auto it = find(sinks.begin(), sinks.end(), sink);

		if (it == sinks.end())
			throw runtime_error("Tried to remove a sink that was not added yet");

		sinks.erase(it);
	}

	void LoggingClient::setPrefix(const string& prefix)
	{
		this->prefix = string(prefix);
	}

	void LoggingClient::setLogLevel(LogLevel newLevel)
	{
		currentLogLevel = newLevel;
	}
}