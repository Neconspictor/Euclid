#pragma once
#pragma optimize( "", on )

#include <platform/logging/LoggingServer.hpp>

namespace platform
{
	std::shared_ptr<LoggingServer> getLogServer();
}