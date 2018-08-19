#include <system/Engine.hpp>
#include <platform/logging/LoggingClient.hpp>
#include <platform/exception/EnumFormatException.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;
using namespace platform;

Engine::Engine() :
	logClient(getLogServer()), config(), systemLogLevel(Debug)
{
	configFileName = "config.ini";
	config.addOption("Logging", "logLevel", &systemLogLevelStr, string(""));
}

Engine::Engine(const Engine& other): logClient(other.logClient), 
	systemLogLevel(other.systemLogLevel),
	configFileName(other.configFileName)
{
	config.addOption("Logging", "logLevel", &systemLogLevelStr, string(""));
}

Engine::~Engine()
{
}

void Engine::run(const TaskManager::TaskPtr& mainLoop)
{

	//taskManager.add(mainLoop);
	//taskManager.start();
	//shutdownSystems();
}

void Engine::setConfigFileName(const string & fileName)
{
	configFileName = fileName;
}

void Engine::stop()
{
	eventChannel.broadcast(TaskManager::StopEvent());
	shutdownSystems();
}

void Engine::add(SystemPtr system)
{
	if (systemMap.find(system->getName()) != systemMap.end())
	{
		LOG(logClient, platform::Warning) << "System already added: " << system->getName();
		return;
	}
	//if (system->updater.get() != nullptr)
	//	taskManager.add(system->updater);

	systemMap.insert(make_pair(system->getName(), system));
	eventChannel.add<Configuration&>(*system.get());
}

Engine::SystemPtr Engine::get(const string& name) const
{
	auto it = systemMap.find(name);

	if (it == systemMap.end())
	{
		LOG(logClient, Warning) << "Cannot find System" << name;
		return nullptr;
	}

	return it->second;
}

LogLevel Engine::getLogLevel()
{
	return systemLogLevel;
}

void Engine::init()
{
	logClient.setPrefix("[Engine]");

	eventChannel.broadcast<Configuration&>(config);


	LOG(logClient, Info) << "Loading configuration file...";
	if (!config.load(configFileName))
	{
		LOG(logClient, Warning) << "Configuration file couldn't be read. Default values are used.";
	}
	else
	{
		LOG(logClient, Info) << "Configuration file loaded.";
	}

	try
	{
		systemLogLevel = stringToLogLevel(systemLogLevelStr);
	}
	catch (const EnumFormatException& e)
	{

		//log error and set default log level
		LOG(logClient, Error) << e.what();

		LOG(logClient, Warning) << "Couldn't get log level from " << systemLogLevelStr << endl
			<< "Log level is set now to 'Warning'" << endl;

		systemLogLevel = Warning;
		systemLogLevelStr = "Warning";
	}
	getLogServer()->setMinLogLevel(systemLogLevel);
	config.write(configFileName);

	initSystems();
}

void Engine::initSystems()
{
	for (auto it : systemMap)
	{
		SystemPtr system = it.second;
		LOG(logClient, Info) << "Initializing " << system->getName();
		system->init();
	}
}

void Engine::shutdownSystems()
{
	for (auto it : systemMap)
	{
		SystemPtr system = it.second;
		LOG(logClient, Info) << "Shutting down" << system->getName();
		system->shutdown();
	}
}