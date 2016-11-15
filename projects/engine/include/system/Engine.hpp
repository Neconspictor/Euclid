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

	Engine();

	Engine(const Engine&);

	virtual ~Engine();

	void add(SystemPtr system);

	SystemPtr get(const std::string& name) const;

	platform::LogLevel getLogLevel();
	
	void init();

	void run(const TaskManager::TaskPtr& mainLoop);

	void setConfigFileName(const std::string& fileName);

	void stop();

private:
	void initSystems();
	void shutdownSystems();

	TaskManager taskManager;
	//std::weak_ptr<EventChannel> eventChannel;
	GlobalEventChannel eventChannel;
	SystemMap systemMap;
	platform::LoggingClient logClient;
	Configuration config;
	std::string configFileName;
	std::string systemLogLevelStr;
	platform::LogLevel systemLogLevel;
};

#endif