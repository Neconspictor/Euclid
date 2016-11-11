#include <EngineTester.hpp>
#include <system/Engine.hpp>
#include <iostream>
#include <Boost_OptionsTest.hpp>

using namespace std;
using namespace platform;

int main(int argc, char** argv)
{
	/*shared_ptr<TestSystem> ts(new TestSystem());
	Engine engine;

	engine.add(ts);

	cout << "Running" << endl;
	engine.run();
	cout << "Done" << endl;
	testEngine();*/
	shared_ptr<LoggingServer> loggingServer(new LoggingServer());
	LoggingClient logger (loggingServer);
	logger.add(makeConsoleEndpoint());
	try
	{
		//testBoostOptions(argc, argv);
		shared_ptr<TestSystem> ts(new TestSystem(loggingServer));
		Engine engine(loggingServer);

		engine.add(ts);

		cout << "Running" << endl;
		engine.run();
		cout << "Done" << endl;
		//testEngine();
	} catch(const exception& e)
	{
		LOG(logger, platform::Fault) << "EngineTestMain, line " << __LINE__ <<": Exception occurred: " << e.what();
	} catch(...)
	{
		LOG(logger, platform::Fault) << "EngineTestMain, line " << __LINE__ << ": Unknown Exception occurred.";
	}

	//terminate running log threads


	return EXIT_SUCCESS;
}