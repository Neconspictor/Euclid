#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;

namespace platform
{
	shared_ptr<LoggingServer> platform::getLogServer()
	{
		static shared_ptr<LoggingServer> logServer(new LoggingServer());
		return logServer;
	}
}