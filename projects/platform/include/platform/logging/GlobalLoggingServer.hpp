#ifndef PLATFORM_LOGGING_GLOBAL_LOGGING_SERVER
#define PLATFORM_LOGGING_GLOBAL_LOGGING_SERVER
#pragma optimize( "", on )

#include <platform/logging/LoggingServer.hpp>

namespace platform
{
	std::shared_ptr<LoggingServer> getLogServer();
}

#endif