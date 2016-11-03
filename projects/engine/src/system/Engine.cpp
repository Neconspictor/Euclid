#include <system/Engine.hpp>

using namespace std;
using namespace platform;

Engine::Engine()
{
	//logger.add(makeConsoleSink());
	//logger.setLogLevel(Debug);
	//logger.setPrefix("[Engine]");

	//LOG(logger, Info) << "Engine created!";
}

Engine::~Engine()
{
}

void Engine::run()
{
	config.init();
	logger.add(makeConsoleSink());
	logger.setLogLevel(Debug);
	logger.setPrefix("[Engine]");

	// merge all system settings into config
	for (auto it : systemMap)
	{
		SystemPtr system = it.second;
		config.settings().add(system->settings);
	}

	LOG(logger, Info) << "Loading configuration file...";
	if (!config.load("config.ini"))
	{
		LOG(logger, Fault) << "Configuration file couldn't be read. Aborting...";
		return;
	}
	LOG(logger, Info) << "Configuration file loaded.";

	initSystems();
	taskManager.start();
	shutdownSystems();
}

void Engine::stop()
{
	channel.broadcast(TaskManager::StopEvent());
}

void Engine::add(SystemPtr system)
{
	if (systemMap.find(system->getName()) != systemMap.end())
	{
		LOG(logger, platform::Warning) << "System already added: " << system->getName();
		return;
	}
	if (system->updater.get() != nullptr)
		taskManager.add(system->updater);

	systemMap.insert(make_pair(system->getName(), system));
}

Engine::SystemPtr Engine::get(const string& name) const
{
	auto it = systemMap.find(name);

	if (it == systemMap.end())
	{
		LOG(logger, Warning) << "Cannot find System" << name;
		return nullptr;
	}

	return it->second;
}

void Engine::initSystems()
{
	for (auto it : systemMap)
	{
		SystemPtr system = it.second;
		LOG(logger, Info) << "Initializing " << system->getName();
		system->init();
	}
}

void Engine::shutdownSystems()
{
	for (auto it : systemMap)
	{
		SystemPtr system = it.second;
		LOG(logger, Info) << "Shutting down" << system->getName();
		system->shutdown();
	}
}