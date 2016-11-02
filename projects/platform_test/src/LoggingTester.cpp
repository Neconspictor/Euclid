#include <iostream>
#include <platform/logging/Logger.hpp>
#include <platform/event/Channel.hpp>
#include <boost/current_function.hpp>
#include <platform/util/concurrent/ConcurrentQueue.hpp>

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

struct MyMessage
{
	string msg;
	int value;

	MyMessage(string msg, int value)
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
		logDetailed(logger, Warning) << "MyMessageHandler: msg=" 
			<< msg.msg << endl << "value= " << msg.value << endl;
	}
};

class IntHandler
{
public:
	void operator() (const IntEvent& event)
	{
		logDetailed(logger, Warning) << "IntHandler: value= " << event.value << endl;
	}
};

int main()
{
	logger.add(makeConsoleSink());
	logger.add(makeFileSink("./log.log"));
	logger.setLogLevel(Debug);
	TestClass testClass(1);
	TestClass testClass2(1);
	testClass.foo(Debug);
	testClass2.foo(Info);
	testClass2.foo(Error);

	MyMessageHandler handler1;
	MyMessageHandler handler2;
	IntHandler intHandler1;

	cout << "Start" << endl;

	for (int i = 0; i < 10; ++i)
	{
		testClass2.foo(Fault);
	}

	Channel::add<MyMessage>(&handler1);
	Channel::add<MyMessage>(&handler2);
	Channel::add<IntEvent>(&intHandler1);

	Channel::broadcast(MyMessage("Hello World!", 34));
	Channel::broadcast(IntEvent(200));

	Channel::remove<MyMessage>(&handler1);
	Channel::broadcast(MyMessage("Test!", -1));

	Channel::remove<MyMessage>(&handler2);
	Channel::remove<IntEvent>(&intHandler1);
	Channel::broadcast(MyMessage("I should be never visible!", 666));
	Channel::broadcast(IntEvent(666));


	ConcurrentQueue<int> queue;
	queue.push(1);
	queue.wait_pop();

	cout << "End" << endl;
	//logger.terminate();

	return EXIT_SUCCESS;
}