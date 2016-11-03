#ifndef NEC_ENGINE_SYSTEM_ENGINE_HPP
#define NEC_ENGINE_SYSTEM_ENGINE_HPP

#include <system/System.hpp>
#include <platform/event/TaskManager.hpp>
#include <memory>
#include <list>
#include <map>


class Engine
{
public:
	using SystemPtr = std::shared_ptr<System>;
	using SystemMap = std::map<std::string, SystemPtr>;

	Engine();

	virtual ~Engine();

	void run();

	void stop();

	void add(SystemPtr system);

	SystemPtr get(const std::string& name) const;

private:
	void initSystems();
	void shutdownSystems();

	TaskManager taskManager;
	EventChannel channel;
	SystemMap systemMap;
	platform::Logger logger;
};

#endif