#include <system/Engine.hpp>
#include <system/Video.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <renderer/RendererOpenGL.hpp>
#include <PBR_MainLoopTask.hpp>
#include <PBR_Deferred_MainLoopTask.hpp>
#include <MainLoopTask.hpp>
#include <platform/window_system/glfw/WindowSystemGLFW.hpp>
#include <boost/locale.hpp>
#include <thread>
#include <platform/SystemUI.hpp>

//#include <Brofiler.h>

using namespace std;
using namespace platform;



int main(int argc, char** argv)
{
	//BROFILER_THREAD("Main");

	// launch console in a seperate thread
	//std::thread myThread(console_thread);
	//myThread.join();
	//console_thread();
	/*boost::locale::generator gen;

	locale loc = gen("de_DE.utf8");

	locale::global(loc);

	cout << "Test: " << 1.3f << endl;

	LoggingClient logger (getLogServer());
	logger.setPrefix("[main]");
	try
	{
		RendererOpenGL renderer;
		Window::WindowStruct desc;
		desc.width = 800;
		desc.height = 600;
		desc.colorBitDepth = 32;
		desc.fullscreen = true;
		desc.posX = 0;
		desc.posY = 0;
		desc.refreshRate = 59;
		desc.title = u8"Hello WindowGLFW!";
		desc.vSync = true;
		desc.visible = true;

		
		PlatformProvider* windowSystem = WindowSystemGLFW::get();
		windowSystem->init();
		Window* window = windowSystem->createWindow(desc, renderer);
		//WindowGLFW* glfwWindow = (WindowGLFW*)window;
		//GLFWwindow* source = glfwWindow->getSource();
		//HWND child = glfwGetWin32Window(source);
		//HWND parent = reinterpret_cast<HWND>(nana::API::root(*monty));

		//SetParent(child, parent);

		 //myThread = thread(console_thread);

		window->minimize();

		Timer timer;
		timer.update();
		// for the first frame set a fixed frame time of 62.5 fps
		float frameTime = 0.016f;

		while (window->isOpen())
		{
			float fps = 1.0f / frameTime;
			

			//window.pollEvents();
			windowSystem->pollEvents();

			if (window->getInputDevice()->isPressed(Input::KEY_A))
			{
				cout << "A is pressed!" << endl;
			}

			if (window->getInputDevice()->isPressed(Input::KEY_ESCAPE))
			{
				window->close();
			}

			if (window->getInputDevice()->isDown(Input::KEY_A))
			{
				cout << "A is down!" << endl;
			}

			if (window->getInputDevice()->isReleased(Input::KEY_A))
			{
				cout << "A is released!" << endl;
			}

			bool pressedDown = window->getInputDevice()->isPressed(Input::KEY_A) &&
				window->getInputDevice()->isDown(Input::KEY_A);

			if (pressedDown)
			{
				cout << "A is pressed and down!" << endl;
			}

			if (window->getInputDevice()->isDown(Input::LeftMouseButton))
			{
				cout << "left mouse button is down!" << endl;
			}

			if (window->getInputDevice()->isPressed(Input::RightMouseButton))
			{
				cout << "right mouse button is pressed!" << endl;
			}

			if (window->getInputDevice()->isReleased(Input::MiddleMouseButton))
			{
				cout << "middle mouse button is released!" << endl;
			}

			//MouseOffset mouse = window->getInputDevice()->getFrameMouseOffset();
			//cout << "mouseXAbsolute = " << mouse.xAbsolute <<
			//	", mouseYAbsolute = " << mouse.yAbsolute << endl <<
			//	"mouseXOffset = " << mouse.xOffset <<
			//	", mouseYOffset = " << mouse.yOffset << endl;

			if (window->hasFocus())
			{
				window->setCursorPosition(window->getWidth() / 2, window->getHeight() / 2);
				//cout << "window has focus!: " << window->getWidth() << endl;
			}

			window->swapBuffers();

			// calc frame time for the next frame
			frameTime = timer.update();
		}

		windowSystem->terminate();*/


	glm::mat4 test;
	test[0][3] = 1;

	LoggingClient logger(getLogServer());
	PlatformProvider* windowSystem = WindowSystemGLFW::get();
	if (!windowSystem->init())
	{
		LOG(logger, platform::Fault) << "Couldn't initialize window system! Aborting...";
		getLogServer()->terminate();
		return EXIT_FAILURE;
	}

	SystemUI* ui = SystemUI::get(windowSystem);

	try {
		shared_ptr<Video> video = make_shared<Video>(Video(windowSystem));
		shared_ptr<Renderer3D> renderer = make_shared<RendererOpenGL>();
		shared_ptr<Engine> engine  = make_shared<Engine>(Engine());

		video->useRenderer(renderer.get());
		video->useUISystem(ui);
		engine->add(video);
		engine->setConfigFileName("config.ini");

		LOG(logger, Info) << "Starting Engine...";
		
		engine->init();

		shared_ptr<PBR_Deferred_MainLoopTask> mainLoop = make_shared<PBR_Deferred_MainLoopTask>(PBR_Deferred_MainLoopTask(engine.get(),
			video->getWindow(), video->getWindowSystem(), renderer.get()));

		//shared_ptr<MainLoopTask> mainLoop = make_shared<MainLoopTask>(MainLoopTask(engine.get(),
		//		video->getWindow(), video->getWindowSystem(), renderer.get()));
		
		mainLoop->setUI(ui);
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

	SystemUI::shutdown();
	windowSystem->terminate();
	getLogServer()->terminate();

	return EXIT_SUCCESS;
}