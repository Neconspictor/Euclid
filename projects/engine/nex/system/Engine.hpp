#pragma once

#include <nex/system/System.hpp>
#include <nex/event/TaskManager.hpp>
#include <memory>
#include <map>
#include <nex/system/Configuration.hpp>


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
	
	virtual void init();

	virtual void run() = 0;

	void setConfigFileName(const std::string& fileName);

	
	void stop();

private:

	void shutdownSystems();
	void initSystems();

	//TaskManager taskManager;
	//std::weak_ptr<EventChannel> eventChannel;
	GlobalEventChannel m_eventChannel;
	SystemMap m_systemMap;
	nex::LoggingClient m_logClient;
	Configuration m_config;
	std::string m_configFileName;
	std::string m_systemLogLevelStr;
	nex::LogLevel m_systemLogLevel;
};