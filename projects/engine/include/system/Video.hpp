#ifndef NEC_ENGINE_VIDEO_HPP
#define NEC_ENGINE_VIDEO_HPP
#include <system/System.hpp>

class Video : public System
{
public:
	Video(const std::string& name, const std::weak_ptr<platform::LoggingServer>& server);
	void handle(const CollectOptions& config) override;
};

#endif