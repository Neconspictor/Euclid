#include <nex/logging/GlobalLoggingServer.hpp>

using namespace std;

namespace nex
{
	LoggingServer* getLogServer()
	{
		static LoggingServer logServer;
		return &logServer;
	}

	void shutdownLogServer()
	{
		getLogServer()->terminate();
	}
}
