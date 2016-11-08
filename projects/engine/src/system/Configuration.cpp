#include <system/Configuration.hpp>
#include <fstream>

using namespace std;
using namespace platform;
using namespace boost::program_options;

Configuration::Configuration(const weak_ptr<LoggingServer>& server) : System("Configuration", server){}


Configuration::~Configuration(){}

bool Configuration::load(const string& fileName)
{
	ifstream file(fileName.c_str());

	if (!file)
	{
		LOG(logClient, Error) << "Couldn't open configuration file: " << fileName;
		return false;
	}

	store(parse_config_file(file, options, true), variables);
	notify(variables);
	return true;
}

void Configuration::parseCmdLine(int argc, char* argv[])
{
	store(parse_command_line(argc, argv, options), variables);
	notify(variables);
}

bool Configuration::isSet(const string& settingName) const
{
	return variables.count(settingName) > 0;
}

Configuration::Options& Configuration::settings()
{
	return options;
}
