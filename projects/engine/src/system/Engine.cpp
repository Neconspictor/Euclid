#include <system/Engine.hpp>

using namespace std;
using namespace platform;

Engine::Engine()
{
	logger.add(makeConsoleSink());
	logger.setLogLevel(Debug);
	logger.setPrefix("[Engine]");

	log(logger, Info) << "Engine created!";
}

Engine::~Engine()
{
}

void Engine::run()
{
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
		log(logger, platform::Warning) << "System already added: " << system->getName();
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
		log(logger, Warning) << "Cannot find System" << name;
		return nullptr;
	}

	return it->second;
}

void Engine::initSystems()
{
	for (auto it : systemMap)
	{
		SystemPtr system = it.second;
		log(logger, Info) << "Initializing" << system->getName();
		system->init();
	}
}

void Engine::shutdownSystems()
{
	for (auto it : systemMap)
	{
		SystemPtr system = it.second;
		log(logger, Info) << "Shutting down" << system->getName();
		system->shutdown();
	}
}