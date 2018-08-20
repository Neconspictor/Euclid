#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <NeXEngine.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>


#ifdef WIN32
	#include <vld.h>
#endif


void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}


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