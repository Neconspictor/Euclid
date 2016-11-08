#include <TaskManagementTester.hpp>
#include <LoggingTester.hpp>
#include <platform/logging/Logger.hpp>
#include <platform/logging/LoggingClient.hpp>
#include <iostream>

using namespace std;
using namespace platform;

LoggingClient logClient(LoggingServer::getInstance());

void testLogging()
{
	logClient.setPrefix("[LoggingTester]");
	TestClass testClass(1);
	TestClass testClass2(1);
	testClass.foo(Debug);
	testClass2.foo(Info);
	testClass2.foo(Error);

	cout << "Start" << endl;

	for (int i = 0; i < 10; ++i)
	{
		testClass2.foo(Fault);
	}

	cout << "End" << endl;
}