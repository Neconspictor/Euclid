#include <system/Engine.hpp>
#include <platform/logging/LoggingClient.hpp>

using namespace std;
using namespace platform;

Engine::Engine(const weak_ptr<LoggingServer>& logger) :
	logClient(logger), config(logger)
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
	logClient.add(makeConsoleEndpoint());
	logClient.setLogLevel(Debug);
	logClient.setPrefix("[Engine]");

	// merge all system settings into config
	for (auto it : systemMap)
	{
		SystemPtr system = it.second;
		config.settings().add(system->settings);
	}

	LOG(logClient, Info) << "Loading configuration file...";
	if (!config.load("config.ini"))
	{
		LOG(logClient, Fault) << "Configuration file couldn't be read. Aborting...";
		stringstream ss;
		ss << BOOST_CURRENT_FUNCTION << ": Configuration file couldn't be read." << endl;
		throw(runtime_error(ss.str()));
	}
	LOG(logClient, Info) << "Configuration file loaded.";

	initSystems();
	taskManager.start();
	shutdownSystems();
}

void Engine::stop()
{
	eventChannel.broadcast(TaskManager::StopEvent());
}

void Engine::add(SystemPtr system)
{
	if (systemMap.find(system->getName()) != systemMap.end())
	{
		LOG(logClient, platform::Warning) << "System already added: " << system->getName();
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
		LOG(logClient, Warning) << "Cannot find System" << name;
		return nullptr;
	}

	return it->second;
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