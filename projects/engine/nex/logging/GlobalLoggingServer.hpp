#pragma once
#pragma optimize( "", on )

#include <nex/logging/LoggingServer.hpp>

namespace nex
{
	LoggingServer* getLogServer();

	void shutdownLogServer();
}