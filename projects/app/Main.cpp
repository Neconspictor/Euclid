#include <NeXEngine.hpp>
#include "nex/logging/GlobalLoggingServer.hpp"


int main(int argc, char** argv)
{
	nex::LoggingClient logger(nex::getLogServer());

	try {
		
		NeXEngine neXEngine;
		neXEngine.init();
		neXEngine.run();
		LOG(logger, nex::Info) << "Done.";

	} catch (const std::exception& e)
	{
		LOG(logger, nex::Fault) << "Exception: " << typeid(e).name() << ": "<< e.what();
	} catch(...)
	{
		LOG(logger, nex::Fault) << "Unknown Exception occurred.";
	}

	nex::shutdownLogServer();

	return EXIT_SUCCESS;
}