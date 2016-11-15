#include <EngineTester.hpp>
#include <system/Engine.hpp>
#include <system/Video.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <renderer/RendererOpenGL.hpp>
#include <MainLoopTask.hpp>

using namespace std;
using namespace platform;

int main(int argc, char** argv)
{
	BROFILER_THREAD("Main");
	LoggingClient logger (getLogServer());
	logger.add(makeConsoleEndpoint());
	logger.setPrefix("[main]");
	try
	{
		shared_ptr<Video> video = make_shared<Video>(Video());
		shared_ptr<Renderer> renderer = make_shared<RendererOpenGL>(RendererOpenGL());
		shared_ptr<Engine> engine  = make_shared<Engine>(Engine());

		video->useRenderer(renderer);
		engine->add(video);
		engine->setConfigFileName("AppConfig.ini");

		LOG(logger, Info) << "Starting Engine...";
		engine->init();

		shared_ptr<MainLoopTask> mainLoop = make_shared<MainLoopTask>(engine.get(),
			video->getWindow().get(), renderer.get());
		mainLoop.get()->setLogLevel(engine.get()->getLogLevel());
		engine->run(mainLoop);
		LOG(logger, Info) << "Done.";
	} catch(const exception& e)
	{
		LOG(logger, platform::Fault) << "Main.cpp, line " << __LINE__ <<": Exception occurred: " << e.what();
	} catch(...)
	{
		LOG(logger, platform::Fault) << "Main.cpp, line " << __LINE__ << ": Unknown Exception occurred.";
	}

	//terminate running log threads


	return EXIT_SUCCESS;
}