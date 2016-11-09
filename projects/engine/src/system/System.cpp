#include <system/System.hpp>
#include <fstream>
#include <platform/logging/LoggingServer.hpp>
#include <platform/logging/LogEndpoint.hpp>


using namespace std;
using namespace platform;

System::System(string name, const weak_ptr<LoggingServer>& server) : logClient(server),
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
	logClient.add(makeFileEndpoint(getName() + ".log"));
	logClient.add(makeConsoleEndpoint());
}

void System::shutdown()
{
	channel.broadcast(SystemShutdownEvent(this));
}

void System::update()
{
}

const string& System::getName() const
{
	return name;
}

void System::enableUpdater(unsigned taskFlags)
{
	updater.reset(new SystemUpdater(this, taskFlags));
}