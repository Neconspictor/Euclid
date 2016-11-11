#ifndef NEC_ENGINE_SYSTEM_HPP
#define NEC_ENGINE_SYSTEM_HPP
#include <platform/event/Task.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/options_description.hpp>
#include <platform/logging/LoggingServer.hpp>
#include <platform/logging/LoggingClient.hpp>
#include <platform/event/GlobalEventChannel.hpp>


class Engine;
class Configuration;
class CollectOptions;

class System
{
public:

	/*struct SystemInitEvent
	{
		SystemInitEvent(System* system) : system(system){}
		System* system;
	};

	struct SystemShutdownEvent
	{
		SystemShutdownEvent(System* system) : system(system) {}
		System* system;
	};*/

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

	System(std::string name, const std::weak_ptr<platform::LoggingServer>& server);

	virtual ~System();

	/*template<typename T>
	void addSetting(Configuration* config, const std::string& nameInConfigFile, T* var, T defaultValue)
	{
		std::stringstream ss;
		ss << getName() << "." << nameInConfigFile;
		settings.add_options()
			(ss.str().c_str(), boost::program_options::value<T>()->default_value(*var))
			;

		config->addOption(getName(), nameInConfigFile, var, defaultValue);
	}*/

	virtual void init();

	virtual void handle(const CollectOptions& config) = 0;

	virtual void shutdown();

	virtual void update();

	void setLogLevel(platform::LogLevel logLevel);

	const std::string& getName() const;


protected:

	void enableUpdater(unsigned int taskFlags = Task::SINGLETHREADED_REPEATING);

	friend class Engine;
	GlobalEventChannel channel;
	std::shared_ptr<SystemUpdater> updater;
	platform::LoggingClient logClient;
	std::string name;
	boost::program_options::options_description settings;

};

#endif