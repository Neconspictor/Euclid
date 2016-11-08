#include <Boost_OptionsTest.hpp>
#include <boost/program_options.hpp>
//#include <boost/filesystem.hpp>
#include <iostream>
#include <string>

using namespace std;


namespace
{
	const size_t ERROR_IN_COMMAND_LINE = 1;
	const size_t SUCCESS = 0;
	const size_t ERROR_UNHANDLED_EXCEPTION = 2;
};

int doProgram(int argc, char** argv);


int testBoostOptions(int argc, char** argv)
{
	if (doProgram(argc, argv) == ERROR_IN_COMMAND_LINE)
	{
		return ERROR_IN_COMMAND_LINE;
	}

	return SUCCESS;
}

int doProgram(int argc, char** argv)
{
	//string appName = boost::filesystem::basename(argv[0]);
	int add = 0;
	int like = 0;
	vector<string> sentence;

	namespace po = boost::program_options;
	po::options_description desc("Options");
	desc.add_options()
		("help,h,he", "Print help messages");

	po::variables_map vm;

	try
	{
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

		if (vm.count("help"))
		{
			cout << "Detailed help description!" << endl;
			return SUCCESS;
		}

	} catch(po::required_option& e)
	{
		cerr << "ERROR: " << e.what() << endl << endl;
		return ERROR_IN_COMMAND_LINE;
	} catch(po::error& e)
	{
		cerr << "ERROR: " << e.what() << endl << endl;
		return ERROR_IN_COMMAND_LINE;
	}

	cout << "USAGE: -h" << endl;
	return SUCCESS;
}