#include <nex/system/System.hpp>


using namespace std;
using namespace nex;

System::System(const string& name) :
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
	m_logger.setPrefix(ss.str().c_str());
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