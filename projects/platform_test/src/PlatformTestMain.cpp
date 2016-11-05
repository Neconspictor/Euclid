#include <LoggingTester.hpp>
#include <TaskManagementTester.hpp>
#include <platform/event/Channel.hpp>
#include <platform/logging/LogSink.hpp>
#include <platform/logging/Logger.hpp>

using namespace std;
using namespace platform;

void testChannel()
{
	MyMessageHandler handler1;
	MyMessageHandler handler2;
	IntHandler intHandler1;


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
}

int main()
{
	LoggingClient logClient(Logger::getInstance());
	logClient.add(makeConsoleSink());
	logClient.add(makeFileSink("./log.log"));
	logClient.setLogLevel(Debug);

	testLogging();
	testChannel();
	testTaskManagement();

	return EXIT_SUCCESS;
}