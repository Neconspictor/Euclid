#pragma once

#include <nex/system/System.hpp>
#include <nex/event/TaskManager.hpp>
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

	nex::LogLevel getLogLevel();
	
	void init();

	// @deprecated
	void run(const TaskManager::TaskPtr& mainLoop);

	void setConfigFileName(const std::string& fileName);

	
	void stop();

private:

	void shutdownSystems();
	void initSystems();

	//TaskManager taskManager;
	//std::weak_ptr<EventChannel> eventChannel;
	GlobalEventChannel eventChannel;
	SystemMap systemMap;
	nex::LoggingClient logClient;
	Configuration config;
	std::string configFileName;
	std::string systemLogLevelStr;
	nex::LogLevel systemLogLevel;
};