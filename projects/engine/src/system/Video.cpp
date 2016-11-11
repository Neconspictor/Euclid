#include <system/Video.hpp>

Video::Video(const std::string& name, const std::weak_ptr<platform::LoggingServer>& server) :
	System(name, server)
{
}

void Video::handle(const CollectOptions& config)
{
	//TODO
}