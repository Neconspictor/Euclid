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
	channel.broadcast(new SystemInitEvent(this));

	logger.setPrefix(name);
	logger.add(platform::makeFileSink(getName() + ".log"));
}

void System::shutdown()
{
	channel.broadcast(new SystemShutdownEvent(this));
}

void System::update()
{
}

const std::string& System::getName() const
{
	return name;
}
