#ifndef NEC_ENGINE_SYSTEM_CONFIGURATION_HPP
#define NEC_ENGINE_SYSTEM_CONFIGURATION_HPP

#include <system/System.hpp>
#include <boost/program_options.hpp>
#include <string>

class Configuration : public System
{
public:

	using Options = boost::program_options::options_description;
	using Variables = boost::program_options::variables_map;

	Configuration(const std::weak_ptr<platform::LoggingServer>& server);

	virtual ~Configuration();

	bool load(const std::string& file);

	void parseCmdLine(int argc, char* argv[]);

	bool isSet(const std::string& settingName) const;

	Options& settings();

	template<typename T>
	T get(const std::string& settingName) const
	{
		if (!isSet(settingName))
		{
			LOG(logClient, platform::Warning) << "Failed to find variable: " << settingName;
			return T();
		}

		return variables[name.c_str()].as<T>();
	}

private:
	Options options;
	Variables variables;

};

#endif