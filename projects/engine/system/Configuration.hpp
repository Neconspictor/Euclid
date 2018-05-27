#pragma once

#include <system/System.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <typeinfo>

class Configuration;

class CollectOptions {
public:
	CollectOptions(Configuration* config)
	{
		this->config = config;
	}

	Configuration* config;
};

class Configuration : public System
{
public:

	void handle(const CollectOptions& config) override {};

	using Options = boost::program_options::options_description;
	using Variables = boost::program_options::variables_map;

	Configuration();

	virtual ~Configuration();

	bool load(const std::string& file);

	bool write(const std::string& file);

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


	template<typename T>
	void addOption(const std::string& category, const std::string& configOption, T* var, const T& defaultValue)
	{
		std::stringstream ss;
		ss << category << "." << configOption;
		const std::string optionName = ss.str();
		options.add_options()
			(optionName.c_str(), boost::program_options::value<T>(var)->default_value(defaultValue))
			;
	}

private:
	Options options;
	Variables variables;
};