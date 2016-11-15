#include <system/System.hpp>
#include <fstream>
#include <platform/logging/LoggingServer.hpp>
#include <platform/logging/LogEndpoint.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>


using namespace std;
using namespace platform;

System::System(const string& name) : logClient(getLogServer()),
name(name)
{
}

System::~System()
{
}

void System::init()
{
	//channel.broadcast(SystemInitEvent(this));

	stringstream ss; ss << "[" << name << "]";
	logClient.setPrefix(ss.str());
	logClient.add(makeFileEndpoint(ss.str() + ".log"));
	logClient.add(makeConsoleEndpoint());
}

void System::shutdown()
{
	//channel.broadcast(SystemShutdownEvent(this));
}

void System::update()
{
}

void System::setLogLevel(LogLevel logLevel)
{
	logClient.setLogLevel(logLevel);
}

const string& System::getName() const
{
	return name;
}

void System::enableUpdater(unsigned taskFlags)
{
	updater.reset(new SystemUpdater(this, taskFlags));
}