#include <LoggingTester.hpp>
#include <TaskManagementTester.hpp>
#include <platform/logging/LogEndpoint.hpp>
#include <platform/logging/LoggingClient.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;
using namespace platform;


int main()
{
	LoggingClient logClient(getLogServer());
	logClient.add(makeConsoleEndpoint());
	logClient.add(makeFileEndpoint("./log.log"));
	logClient.setLogLevel(Debug);

	testLogging();
	testTaskManagement();

	return EXIT_SUCCESS;
}