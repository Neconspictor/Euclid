#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;

namespace platform
{
	shared_ptr<LoggingServer> logServer(new LoggingServer());

	shared_ptr<LoggingServer> platform::getLogServer()
	{
		return logServer;
	}
}