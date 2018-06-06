#include <system/Engine.hpp>
#include <system/Video.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <renderer/RendererOpenGL.hpp>
#include <PBR_MainLoopTask.hpp>
#include <pbr_deferred/PBR_Deferred_MainLoopTask.hpp>
#include <MainLoopTask.hpp>
#include <window_system/glfw/SubSystemProviderGLFW.hpp>
#include <boost/locale.hpp>
#include <thread>

//#include <Brofiler.h>

using namespace std;
using namespace platform;



int main(int argc, char** argv)
{
	//BROFILER_THREAD("Main");

	glm::mat4 test;
	test[0][3] = 1;

	LoggingClient logger(getLogServer());
	SubSystemProvider* windowSystem = SubSystemProviderGLFW::get();
	if (!windowSystem->init())
	{
		LOG(logger, platform::Fault) << "Couldn't initialize window system! Aborting...";
		getLogServer()->terminate();
		return EXIT_FAILURE;
	}

	try {
		shared_ptr<Video> video = make_shared<Video>(windowSystem);
		shared_ptr<Renderer3D> renderer = make_shared<RendererOpenGL>();
		shared_ptr<Engine> engine  = make_shared<Engine>();

		video->useRenderer(renderer.get());
		//video->useUISystem(ui);
		engine->add(video);
		engine->setConfigFileName("config.ini");

		LOG(logger, Info) << "Starting Engine...";
		
		engine->init();

		std::unique_ptr<ImGUI_Impl> imGUI = windowSystem->createGUI(video->getWindow());

		shared_ptr<PBR_Deferred_MainLoopTask> mainLoop = make_shared<PBR_Deferred_MainLoopTask>(engine.get(),
			video->getWindow(), video->getWindowSystem(), renderer.get(), imGUI.get());

		//shared_ptr<MainLoopTask> mainLoop = make_shared<MainLoopTask>(engine.get(),
		//		video->getWindow(), video->getWindowSystem(), renderer.get());
		
		//mainLoop->setUI(ui);
		mainLoop->init();

		engine->run(mainLoop);
		LOG(logger, Info) << "Done.";
	} catch (const exception& e)
	{
		LOG(logger, platform::Fault) << "Exception: " << typeid(e).name() << ": "<< e.what();
	} catch(...)
	{
		LOG(logger, platform::Fault) << "Unknown Exception occurred.";
	}

	windowSystem->terminate();
	getLogServer()->terminate();

	return EXIT_SUCCESS;
}