#pragma once
#include <nex/event/Task.hpp>
#include <boost/program_options/options_description.hpp>
#include <nex/event/GlobalEventChannel.hpp>
#include "nex/common/Log.hpp"

class Configuration;

class System
{
public:

	System(const std::string& name);

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

	virtual void handle(Configuration& config) = 0;

	virtual void shutdown();

	virtual void update();

	const std::string& getName() const;


protected:

	using Options = boost::program_options::options_description;

	//void enableUpdater(unsigned int taskFlags = Task::SINGLETHREADED_REPEATING);

	//friend class Engine;
	//GlobalEventChannel channel;
	//std::shared_ptr<SystemUpdater> updater;
	nex::Logger m_logger;
	std::string name;
	Options settings;

};