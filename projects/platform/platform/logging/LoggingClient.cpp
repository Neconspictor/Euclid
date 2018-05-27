#include <platform/logging/LoggingClient.hpp>
#include <platform/logging/LoggingServer.hpp>
#include <platform/logging/LogEndpoint.hpp>

using namespace std;

namespace platform
{
	LogMessage LoggingClient::operator()(LogLevel level, const string& file, const string& function, int line) const
	{
		// performs return value optimization. No copy will be created
		auto instance = server.lock();
		if (!instance) return LogMessage();

		if (!instance->isActive(level)) return LogMessage();
		LogMessage result(level, file,
			//util::stringFromRegex(function, "(.* .* )(.*\\(.*)", 1), 
			function,
			line,
			this);

		//return LogMessage (this);
		return result;
	}

	/*LoggingClient& LoggingClient::operator=(const LoggingClient& other)
	{
		if (this != &other) {
			LoggingClient tmp(other);
			this->swap(tmp);
		}

		return *this;
	}*/

	void LoggingClient::flush(const LogMessage& message) const
	{
		if (!server.lock()->isActive(message.meta.level)) return;

		// just forward the message to the logging server. Non blocking!
		server.lock()->send(*this, message);
	}

	LoggingClient::LoggingClient(const weak_ptr<LoggingServer>& server, bool useConsoleEndpoint,
		bool useFileEndpoint) :
		prefix(""), server(server)
	{
		if (useConsoleEndpoint)
		{
			add(server.lock()->getConsoleEndpoint());
		}

		if (useFileEndpoint)
		{
			add(server.lock()->getFileEndpoint());
		}
	}

	LoggingClient::LoggingClient(const LoggingClient& other): 
		prefix(other.prefix), server(other.server)
	{
		for (auto endpoint : other.endpoints)
			endpoints.push_back(endpoint);
	}

	LoggingClient::LoggingClient(LoggingClient&& other) :
		prefix(other.prefix), endpoints(other.endpoints), server(other.server)
	{
		//other.endpoints.clear();
	}

	LoggingClient::~LoggingClient()
	{
	}

	void LoggingClient::add(const LogEndpoint& endpoint)
	{
		endpoints.push_back(endpoint);
	}

	const string& LoggingClient::getPrefix() const
	{
		return prefix;
	}

	const vector<LogEndpoint>& LoggingClient::getEndpoints() const
	{
		return endpoints;
	}

	void LoggingClient::remove(const LogEndpoint& endpoint)
	{
		auto it = find(endpoints.begin(), endpoints.end(), endpoint);

		if (it == endpoints.end())
			throw runtime_error("Tried to remove a logging endpoint that was not added yet");

		endpoints.erase(it);
	}

	void LoggingClient::setPrefix(const string& prefix)
	{
		this->prefix = string(prefix);
	}

	void LoggingClient::swap(LoggingClient& other)
	{
		this->endpoints.swap(other.endpoints);
		this->prefix.swap(other.prefix);
		this->server.swap(other.server);
	}
}