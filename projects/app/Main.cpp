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
#include <glm/glm.hpp>
#include <camera/TrackballQuatCamera.hpp>
#include <camera/FPCamera.hpp>
#include <gui/AppStyle.hpp>
#include <gui/ConfigurationWindow.hpp>
#include <gui/SceneGUI.hpp>


//#include <Brofiler.h>

using namespace std;

platform::LoggingClient* globalLogClient = nullptr;

void setupCallbacks(Window* window, Camera* camera, PBR_Deferred_MainLoopTask* task, Renderer3D* renderer);
void setupGUI(ControllerStateMachine* controllerSM, PBR_Deferred_MainLoopTask* task);
void updateWindowTitle(Window* window, const char* baseTitle, float frameTime, float fps);

float runtime = 0;

int main(int argc, char** argv)
{
	//BROFILER_THREAD("Main");

	platform::LoggingClient logger(platform::getLogServer());
	globalLogClient = &logger;

	SubSystemProvider* windowSystem = SubSystemProviderGLFW::get();
	if (!windowSystem->init())
	{
		LOG(logger, platform::Fault) << "Couldn't initialize window system! Aborting...";
		platform::getLogServer()->terminate();
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

		LOG(logger, platform::Info) << "Starting Engine...";
		
		engine->init();

		std::unique_ptr<ImGUI_Impl> imGUI = windowSystem->createGUI(video->getWindow());

		shared_ptr<PBR_Deferred_MainLoopTask> mainLoop = make_shared<PBR_Deferred_MainLoopTask>(video->getWindow(), video->getWindowSystem(), renderer.get(), imGUI.get());

		//shared_ptr<MainLoopTask> mainLoop = make_shared<MainLoopTask>(engine.get(),
		//		video->getWindow(), video->getWindowSystem(), renderer.get());
		
		//mainLoop->setUI(ui);
		mainLoop->init();


		Window* window = video->getWindow();
		Input* input = window->getInputDevice();
		std::shared_ptr<Camera> camera = make_shared<FPCamera>(FPCamera());

		if (TrackballQuatCamera* casted = dynamic_cast<TrackballQuatCamera*>(camera.get()))
		{
			auto cameraResizeCallback = bind(&TrackballQuatCamera::updateOnResize, casted, placeholders::_1, placeholders::_2);
			casted->updateOnResize(window->getWidth(), window->getHeight());
			input->addResizeCallback(cameraResizeCallback);
		}

		window->activate();
		int windowWidth = window->getWidth();
		int windowHeight = window->getHeight();

		camera->setPosition(glm::vec3(0.0f, 3.0f, 2.0f));
		camera->setLook(glm::vec3(0.0f, 0.0f, -1.0f));
		camera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
		camera->setAspectRatio((float)windowWidth / (float)windowHeight);

		Frustum frustum = camera->getFrustum(Perspective);
		frustum.left = -10.0f;
		frustum.right = 10.0f;
		frustum.bottom = -10.0f;
		frustum.top = 10.0f;
		frustum.nearPlane = 0.1f;
		frustum.farPlane = 10.0f;
		camera->setOrthoFrustum(frustum);

		setupCallbacks(window, camera.get(), mainLoop.get(), renderer.get());





		ControllerStateMachine controllerSM(nullptr);

		controllerSM.setCurrentController(std::make_unique<App::EditMode>(window,
			input, 
			mainLoop.get(), 
			camera.get(), 
			imGUI.get(), 
			std::unique_ptr<nex::engine::gui::Drawable>()));

		controllerSM.init();

		setupGUI(&controllerSM, mainLoop.get());


		Timer timer;
		FPSCounter counter;

		std::string baseTitle = window->getTitle();

		while(window->isOpen())
		{
			// Poll input events before checking if the app is running, otherwise 
			// the window is likely to hang or crash (at least on windows platform)
			windowSystem->pollEvents();

			float frameTime = timer.update();
			float fps = counter.update(frameTime);
			updateWindowTitle(window, baseTitle.c_str(), frameTime, fps);

			if (mainLoop->isRunning())
			{
				controllerSM.frameUpdate(frameTime);

				window->activate();

				mainLoop->run(camera.get(), frameTime);

				imGUI->newFrame();
				controllerSM.getCurrentController()->getDrawable()->drawGUI();
				ImGui::Render();
				imGUI->renderDrawData(ImGui::GetDrawData());

				// present rendered frame
				window->swapBuffers();
			} else
			{
				this_thread::sleep_for(chrono::milliseconds(500));
			}
		}

		engine->stop();

		LOG(logger, platform::Info) << "Done.";
	} catch (const exception& e)
	{
		LOG(logger, platform::Fault) << "Exception: " << typeid(e).name() << ": "<< e.what();
	} catch(...)
	{
		LOG(logger, platform::Fault) << "Unknown Exception occurred.";
	}

	windowSystem->terminate();
	platform::shutdownLogServer();

	return EXIT_SUCCESS;
}


void setupCallbacks(Window* window, Camera* camera, PBR_Deferred_MainLoopTask* task, Renderer3D* renderer)
{
	using namespace placeholders;

	Input* input = window->getInputDevice();

	//auto focusCallback = bind(&PBR_Deferred_MainLoopTask::onWindowsFocus, this, placeholders::_1, placeholders::_2);
	auto scrollCallback = bind(&Camera::onScroll, camera, placeholders::_1, placeholders::_2);
	
	input->addWindowFocusCallback([=](Window* window_s, bool receivedFocus)
	{
		platform::LoggingClient& logClient = *globalLogClient;

		task->setRunning(receivedFocus);
		if (receivedFocus)
		{
			LOG(logClient, platform::Debug) << "received focus!";
			//isRunning = true;
		}
		else
		{
			LOG(logClient, platform::Debug) << "lost focus!";
			//isRunning = false;
			if (window_s->isInFullscreenMode())
			{
				window_s->minimize();
			}
		}
	});
	input->addScrollCallback(scrollCallback);

	input->addResizeCallback([=](int width, int height)
	{
		platform::LoggingClient& logClient = *globalLogClient;

		LOG(logClient, platform::Debug) << "addResizeCallback : width: " << width << ", height: " << height;

		if (!window->hasFocus()) {
			LOG(logClient, platform::Debug) << "addResizeCallback : no focus!";
		}

		if (width == 0 || height == 0) {
			LOG(logClient, platform::Warning) << "addResizeCallback : width or height is 0!";
			return;
		}

		camera->setAspectRatio((float)width / (float)height);

		task->updateRenderTargets(width, height);
	});

	input->addRefreshCallback([=]() {
		platform::LoggingClient& logClient = *globalLogClient;

		LOG(logClient, platform::Warning) << "addRefreshCallback : called!";
		if (!window->hasFocus()) {
			LOG(logClient, platform::Warning) << "addRefreshCallback : no focus!";
		}
	});
}


void setupGUI(ControllerStateMachine* controllerSM, PBR_Deferred_MainLoopTask* task)
{
	using namespace nex::engine::gui;

	App::AppStyle style;
	style.apply();

	std::unique_ptr<SceneGUI> root = std::make_unique<SceneGUI>(controllerSM);
	std::unique_ptr<Drawable> configurationWindow = std::make_unique<App::ConfigurationWindow>("Graphics Configuration Window", root->getMainMenuBar());

	for (int i = 0; i < 5; ++i)
	{
		std::unique_ptr<hbao::HBAO_ConfigurationView> hbaoView = std::make_unique<hbao::HBAO_ConfigurationView>(task->getHBAO(),
			root->getOptionMenu(), configurationWindow.get(), "HBAO");
		//hbaoView->setVisible(true);
		configurationWindow->addChild(move(hbaoView));
	}

	configurationWindow->useStyleClass(make_shared<App::ConfigurationStyle2>());
	root->addChild(move(configurationWindow));

	controllerSM->getCurrentController()->setDrawable(move(root));
}

void updateWindowTitle(Window* window, const char* baseTitle, float frameTime, float fps)
{
	runtime += frameTime;
	if (runtime > 1)
	{
		stringstream ss; ss << baseTitle << " : FPS= " << fps;
		window->setTitle(ss.str());
		runtime -= 1;
	}
}