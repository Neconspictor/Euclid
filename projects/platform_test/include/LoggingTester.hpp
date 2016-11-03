#ifndef PLATFORM_TEST_LOGGING_TESTER_HPP
#define PLATFORM_TEST_LOGGING_TESTER_HPP
#include <ostream>
#include <platform/logging/LogLevel.hpp>
#include <platform/logging/Logger.hpp>

extern platform::Logger logger;

class TestClass
{
public:

	TestClass(int test) : test(test)
	{}

	void foo(platform::LogLevel level)
	{
		LOG(logger, level) << "Hello World! " << test << std::endl;
		++test;
	}

private:
	int test;
};

struct MyMessage
{
	std::string msg;
	int value;

	MyMessage(std::string msg, int value)
	{
		this->msg = msg;
		this->value = value;
	}
};

struct IntEvent
{
	int value;
	IntEvent(int value)
	{
		this->value = value;
	}
};

class MyMessageHandler
{
public:
	void operator() (const MyMessage& msg)
	{
		LOG(logger, platform::Warning) << "MyMessageHandler: msg="
			<< msg.msg << std::endl << "value= " << msg.value << std::endl;
	}
};

class IntHandler
{
public:
	void operator() (const IntEvent& event)
	{
		LOG(logger, platform::Warning) << "IntHandler: value= " << event.value << std::endl;
	}
};

void testLogging();

#endif