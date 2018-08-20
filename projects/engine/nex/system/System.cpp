#include <nex/system/System.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>


using namespace std;
using namespace nex;

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
}

void System::shutdown()
{
	//channel.broadcast(SystemShutdownEvent(this));
}

void System::update()
{
}

const string& System::getName() const
{
	return name;
}

/*void System::enableUpdater(unsigned taskFlags)
{
	updater.reset(new SystemUpdater(this, taskFlags));
}*/