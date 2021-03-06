#include <nex/config/Configuration.hpp>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <nex/util/ExceptionHandling.hpp>

using namespace std;
using namespace nex;
using namespace boost::program_options;

Configuration* Configuration::globalConfig = nullptr;

Configuration::Configuration() : m_logger("Configuration")
{
}


Configuration::~Configuration() {}

Configuration* Configuration::getGlobalConfiguration()
{
	return globalConfig;
}

void Configuration::setGlobalConfiguration(Configuration* config)
{
	globalConfig = config;
}

bool Configuration::load(const std::filesystem::path& fileName)
{
#ifdef WIN32
	ifstream file(fileName.generic_wstring());
#else
	ifstream file(fileName.generic_u8string());
#endif

	

	if (!file)
	{
		LOG(m_logger, Error) << "Couldn't open configuration file: " << fileName;
		return false;
	}

	try {
		store(parse_config_file(file, options, true), variables);
		notify(variables);
	}
	catch (const std::exception & e) {
		LOG(m_logger, Debug) << "Exception: " << e.what();
		LOG(m_logger, Error) << "Couldn't read configuration file: " << fileName;
		return false;
	}

	return true;
}

bool Configuration::write(const std::filesystem::path& fileName)
{
	auto optionsVec = options.options();
	boost::property_tree::ptree pt;

	for (auto item : variables)
	{

		const string& desc = item.first.c_str();
		LOG(m_logger, Debug) << desc << endl;
		auto& value = variables[desc].value();
		if (auto v = boost::any_cast<uint32_t>(&value)) {
			LOG(m_logger, Debug) << *v << endl;
			pt.put(desc, *v);
		}
		else if (auto v = boost::any_cast<string>(&value)) {
			LOG(m_logger, Debug) << *v << endl;
			pt.put(desc, *v);
		} else if (auto v = boost::any_cast<double>(&value)) {
			LOG(m_logger, Debug) << *v << endl;
			pt.put(desc, *v);
		} else if (auto v = boost::any_cast<bool>(&value)) {
			LOG(m_logger, Debug) << *v << endl;
			pt.put(desc, *v);
		} else if (auto v = boost::any_cast<unsigned int>(&value)) {
			LOG(m_logger, Debug) << *v << endl;
			pt.put(desc, *v);
		} else if (auto v = boost::any_cast<int>(&value)) {
			LOG(m_logger, Debug) << *v << endl;
			pt.put(desc, *v);
		}
		else {
			stringstream ss;
			ss << BOOST_CURRENT_FUNCTION << " couldn't convert value for program option: " << desc << endl;
			throw_with_trace(runtime_error(ss.str()));
		}

	}

	try {
		write_ini(fileName.generic_u8string(), pt);
	}
	catch (const std::exception e) {
		LOG(m_logger, Error) << "Couldn't write to configuration file: " << fileName;
	}
	
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