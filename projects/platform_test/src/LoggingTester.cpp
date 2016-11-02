#include <iostream>
#include <platform/logging/Logger.hpp>
#include <boost/current_function.hpp>

using namespace std;
using namespace platform;

Logger logger;

class TestClass
{
public:

	TestClass(int test) : test(test)
	{}

	void foo(LogLevel level)
	{
		logDetailed(logger, level) << "Hello World! " << test << endl;
		++test;
	}

private:
	int test;
};


int main()
{
	logger.add(makeConsoleSink());
	logger.add(makeFileSink("./log.log"));
	logger.setLogLevel(Warning);
	TestClass testClass(1);
	TestClass testClass2(1);
	testClass.foo(Debug);
	testClass2.foo(Info);
	testClass2.foo(Error);

	cout << "Start" << endl;

	for (int i = 0; i < 10000; ++i)
	{
		testClass2.foo(Info);
	}

	for (int i = 0; i < 10000; ++i)
	{
		testClass2.foo(Fault);
	}
	cout << "End" << endl;
	logger.terminate();

	return EXIT_SUCCESS;
}