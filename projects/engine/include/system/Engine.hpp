#ifndef NEC_ENGINE_SYSTEM_ENGINE_HPP
#define NEC_ENGINE_SYSTEM_ENGINE_HPP

#include <system/System.hpp>
#include <platform/event/TaskManager.hpp>
#include <memory>
#include <map>
#include "Configuration.hpp"


class Engine
{
public:
	using SystemPtr = std::shared_ptr<System>;
	using SystemMap = std::map<std::string, SystemPtr>;

	Engine(const std::weak_ptr<platform::LoggingServer>& logger);

	virtual ~Engine();

	void run();

	void stop();

	void add(SystemPtr system);

	SystemPtr get(const std::string& name) const;

private:
	void initSystems();
	void shutdownSystems();

	TaskManager taskManager;
	//std::weak_ptr<EventChannel> eventChannel;
	GlobalEventChannel eventChannel;
	SystemMap systemMap;
	platform::LoggingClient logClient;
	Configuration config;
};

#endif