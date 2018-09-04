#include <nex/logging/LoggingClient.hpp>
#include <nex/logging/LoggingServer.hpp>
#include <nex/logging/LogEndpoint.hpp>
#include <nex/util/ExceptionHandling.hpp>

using namespace std;

namespace nex
{
	LogMessage LoggingClient::operator()(LogLevel level, const string& file, const string& function, int line) const
	{
		// performs return value optimization. No copy will be created
		auto instance = server;
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
		if (!server->isActive(message.meta.level)) return;

		// just forward the message to the logging server. Non blocking!
		server->send(*this, message);
	}

	LoggingClient::LoggingClient(LoggingServer* server, bool useConsoleEndpoint,
		bool useFileEndpoint)
	{
		prefix = "";
		this->server = server;
		if (useConsoleEndpoint)
		{
			add(server->getConsoleEndpoint());
		}

		if (useFileEndpoint)
		{
			add(server->getFileEndpoint());
		}
	}

	LoggingClient::LoggingClient(const LoggingClient& other): 
		prefix(other.prefix), server(other.server)
	{
		for (auto endpoint : other.endpoints)
			endpoints.push_back(endpoint);
	}

	LoggingClient::LoggingClient(LoggingClient&& other) :
		prefix(std::move(other.prefix)), endpoints(std::move(other.endpoints)), server(other.server)
	{
		//other.endpoints.clear();
		other.server = nullptr;
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
			throw_with_trace(runtime_error("Tried to remove a logging endpoint that was not added yet"));

		endpoints.erase(it);
	}

	void LoggingClient::setPrefix(const string& prefix)
	{
		this->prefix = prefix;
	}

	void LoggingClient::swap(LoggingClient& other)
	{
		this->endpoints.swap(other.endpoints);
		this->prefix = std::move(other.prefix);

		this->server =  other.server;
		other.server = nullptr;
	}
}