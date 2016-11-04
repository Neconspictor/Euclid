#include <system/System.hpp>
#include <fstream>

System::System(std::string name) : name(name)
{
}

System::~System()
{
}

void System::init()
{
	channel.broadcast(SystemInitEvent(this));

	logger.setPrefix(name);
	logger.add(platform::makeFileSink(getName() + ".log"));
	logger.add(platform::makeConsoleSink());
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

void System::enableUpdater(unsigned int taskFlags)
{
	updater.reset(new SystemUpdater(this, taskFlags));
}