#include <nex/system/Engine.hpp>
#include <nex/logging/LoggingClient.hpp>
#include <nex/exception/EnumFormatException.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include "nex/util/Globals.hpp"

using namespace std;
using namespace nex;

Engine::Engine() :
	m_logClient(getLogServer()), m_systemLogLevel(Debug)
{
	m_configFileName = "config.ini";
	m_config.addOption("Logging", "logLevel", &m_systemLogLevelStr, string(""));
	m_config.addOption("General", "rootDirectory", &m_systemLogLevelStr, string("./"));
}

Engine::Engine(const Engine& other): m_logClient(other.m_logClient),
	m_systemLogLevel(other.m_systemLogLevel),
	m_configFileName(other.m_configFileName)
{
	m_config.addOption("Logging", "logLevel", &m_systemLogLevelStr, string(""));
	m_config.addOption("General", "rootDirectory", &m_systemLogLevelStr, string("./"));
}

Engine::~Engine()
{
}

void Engine::setConfigFileName(const string & fileName)
{
	m_configFileName = fileName;
}

void Engine::stop()
{
	m_eventChannel.broadcast(TaskManager::StopEvent());
	shutdownSystems();
}

void Engine::add(SystemPtr system)
{
	if (m_systemMap.find(system->getName()) != m_systemMap.end())
	{
		LOG(m_logClient, Warning) << "System already added: " << system->getName();
		return;
	}
	//if (system->updater.get() != nullptr)
	//	taskManager.add(system->updater);

	m_systemMap.insert(make_pair(system->getName(), system));
	m_eventChannel.add<Configuration&>(*system.get());
}

Engine::SystemPtr Engine::get(const string& name) const
{
	auto it = m_systemMap.find(name);

	if (it == m_systemMap.end())
	{
		LOG(m_logClient, Warning) << "Cannot find System" << name;
		return nullptr;
	}

	return it->second;
}

LogLevel Engine::getLogLevel()
{
	return m_systemLogLevel;
}

void Engine::init()
{
	m_logClient.setPrefix("[Engine]");

	m_eventChannel.broadcast<Configuration&>(m_config);


	LOG(m_logClient, Info) << "Loading configuration file...";
	if (!m_config.load(m_configFileName))
	{
		LOG(m_logClient, Warning) << "Configuration file couldn't be read. Default values are used.";
	}
	else
	{
		LOG(m_logClient, Info) << "Configuration file loaded.";
	}

	try
	{
		m_systemLogLevel = stringToLogLevel(m_systemLogLevelStr);
	}
	catch (const EnumFormatException& e)
	{

		//log error and set default log level
		LOG(m_logClient, Error) << e.what();

		LOG(m_logClient, Warning) << "Couldn't get log level from " << m_systemLogLevelStr << endl
			<< "Log level is set now to 'Warning'" << endl;

		m_systemLogLevel = Warning;
		m_systemLogLevelStr = "Warning";
	}
	getLogServer()->setMinLogLevel(m_systemLogLevel);
	m_config.write(m_configFileName);

	Configuration::setGlobalConfiguration(&m_config);

	::util::Globals::initGlobals();

	LOG(m_logClient, Info) << "root Directory = " << ::util::Globals::getRootDirectory();

	initSystems();
}

void Engine::initSystems()
{
	for (auto it : m_systemMap)
	{
		SystemPtr system = it.second;
		LOG(m_logClient, Info) << "Initializing " << system->getName();
		system->init();
	}
}

void Engine::shutdownSystems()
{
	for (auto it : m_systemMap)
	{
		SystemPtr system = it.second;
		LOG(m_logClient, Info) << "Shutting down" << system->getName();
		system->shutdown();
	}
}