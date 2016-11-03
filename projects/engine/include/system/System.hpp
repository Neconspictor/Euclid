#ifndef NEC_ENGINE_SYSTEM_HPP
#define NEC_ENGINE_SYSTEM_HPP
#include <platform/event/EventChannel.hpp>
#include <platform/event/Task.hpp>
#include <platform/logging/Logger.hpp>

class Engine;

class System
{
public:

	struct SystemInitEvent
	{
		SystemInitEvent(System* system) : system(system){}
		System* system;
	};

	struct SystemShutdownEvent
	{
		SystemShutdownEvent(System* system) : system(system) {}
		System* system;
	};

	struct SystemUpdater : Task
	{
		SystemUpdater(System* s, unsigned int flags = SINGLETHREADED_REPEATING) :
			Task(flags), system(s)
		{}

		virtual void run() override
		{
			system->update();
		}

		System* system;
	};

	System(std::string name);

	virtual ~System();

	virtual void init();

	virtual void shutdown();

	virtual void update();

	const std::string& getName() const;

protected:
	friend class Engine;
	EventChannel channel;
	std::shared_ptr<SystemUpdater> updater;
	platform::Logger logger;
	std::string name;
};

#endif