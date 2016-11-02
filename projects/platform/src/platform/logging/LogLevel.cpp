#include <platform/logging/LogLevel.hpp>

using namespace platform;

std::ostream& operator<<(std::ostream& os, const LogLevel& level)
{
	switch (level)
	{
	case Debug: os << "[DEBUG]"; break;
	case Info: os << "[INFO]"; break;
	case Warning: os << "[WARNING]"; break;
	case Error: os << "[ERROR]"; break;
	case Fault: os << "[FAULT]"; break;
	default: break;
	}
	return os;
}