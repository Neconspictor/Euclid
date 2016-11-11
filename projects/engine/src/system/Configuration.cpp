#include <system/Configuration.hpp>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace std;
using namespace platform;
using namespace boost::program_options;

Configuration::Configuration(const weak_ptr<LoggingServer>& server) : 
	System("Configuration", server) {}


Configuration::~Configuration() {}

bool Configuration::load(const string& fileName)
{
	ifstream file(fileName.c_str());

	if (!file)
	{
		LOG(logClient, Error) << "Couldn't open configuration file: " << fileName;
		//parse_config_file(file, options, true);
		//return false;
	}

	store(parse_config_file(file, options, true), variables);
	notify(variables);
	return true;
}

bool Configuration::write(const string& fileName)
{
	auto optionsVec = options.options();
	boost::property_tree::ptree pt;

	for (auto item : variables)
	{

		const string& desc = item.first.c_str();
		LOG(logClient, Debug) << desc << endl;
		auto& value = variables[desc].value();
		if (auto v = boost::any_cast<uint32_t>(&value)) {
			LOG(logClient, Debug) << *v << endl;
			pt.put(desc, *v);
		}
		else if (auto v = boost::any_cast<string>(&value)) {
			LOG(logClient, Debug) << *v << endl;
			pt.put(desc, *v);
		} else if (auto v = boost::any_cast<double>(&value)) {
			LOG(logClient, Debug) << *v << endl;
			pt.put(desc, *v);
		} else
		{
			stringstream ss;
			ss << BOOST_CURRENT_FUNCTION << " couldn't convert value for program option: " << desc << endl;
			throw runtime_error(ss.str());
		}

	}

	write_ini(fileName, pt);
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
