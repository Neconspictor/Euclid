#pragma once
#pragma optimize( "", on )

#include <platform/logging/LoggingServer.hpp>

namespace platform
{
	LoggingServer* getLogServer();

	void shutdownLogServer();
}