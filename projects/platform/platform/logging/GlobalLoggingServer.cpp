#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;

namespace platform
{
	shared_ptr<LoggingServer> getLogServer()
	{
		static shared_ptr<LoggingServer> logServer = std::make_shared<LoggingServer>();
		return logServer;
	}
}