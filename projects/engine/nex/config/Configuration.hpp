#pragma once

#include <boost/program_options.hpp>
#include <string>
#include <nex/common/Log.hpp>

namespace nex
{
	class Configuration
	{
	public:

		using Options = boost::program_options::options_description;
		using Variables = boost::program_options::variables_map;

		Configuration();

		virtual ~Configuration();

		static Configuration* getGlobalConfiguration();
		static void setGlobalConfiguration(Configuration* config);

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
				LOG(m_logger, nex::Warning) << "Failed to find variable: " << settingName;
				return T();
			}

			return variables[settingName.c_str()].as<T>();
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
		nex::Logger m_logger;

		static Configuration* globalConfig;
	};

}