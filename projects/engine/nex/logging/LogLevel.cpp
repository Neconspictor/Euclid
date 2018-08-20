#include <nex/logging/LogLevel.hpp>

std::ostream& operator<<(std::ostream& os, const nex::LogLevel& level)
{
	using namespace nex;

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