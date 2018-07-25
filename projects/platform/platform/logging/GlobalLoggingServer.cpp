#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;

namespace platform
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
