#include <system/System.hpp>
#include <fstream>
#include <platform/logging/Logger.hpp>
#include <platform/logging/LogSink.hpp>


using namespace platform;

System::System(std::string name) : logClient(Logger::getInstance()),
name(name)
{
}

System::~System()
{
}

void System::init()
{
	channel.broadcast(SystemInitEvent(this));

	logClient.setPrefix(name);
	logClient.add(makeFileSink(getName() + ".log"));
	logClient.add(makeConsoleSink());
}

void System::shutdown()
{
	channel.broadcast(SystemShutdownEvent(this));
}

void System::update()
{
}

const std::string& System::getName() const
{
	return name;
}

void System::enableUpdater(unsigned taskFlags)
{
	updater.reset(new SystemUpdater(this, taskFlags));
}