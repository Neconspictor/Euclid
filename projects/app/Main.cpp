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

void setupCallbacks(Window* window, Camera* camera, PBR_Deferred_MainLoopTask* task, Renderer3D* renderer, platform::LoggingClient& logClient);
void setupGUI(ControllerStateMachine* controllerSM, PBR_Deferred_MainLoopTask* task);
void setupCamera(Camera* camera, Window* window, Input* input);
void updateWindowTitle(Window* window, const char* baseTitle, float frameTime, float fps);
void readConfig(shared_ptr<Video> video, Renderer3D* renderer, Engine* engine);
void run(SubSystemProvider* windowSystem, Window* window, PBR_Deferred_MainLoopTask* task, ControllerStateMachine* controllerSM, ImGUI_Impl* gui,
	Camera* camera, const char* baseTitle, Timer& timer, FPSCounter& counter, SceneNode* scene);

SceneNode* createScene();

std::list<SceneNode> nodes;
std::list<Vob> vobs;

int main(int argc, char** argv)
{
	//BROFILER_THREAD("Main");

	platform::LoggingClient logger(platform::getLogServer());

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
		shared_ptr<Engine> engine = make_shared<Engine>();

		LOG(logger, platform::Info) << "Starting Engine...";
		readConfig(video, renderer.get(), engine.get());

		Timer timer;
		FPSCounter counter;
		Window* window = video->getWindow();
		Input* input = window->getInputDevice();
		std::shared_ptr<Camera> camera = make_shared<FPCamera>(FPCamera());
		std::string baseTitle = window->getTitle();

		std::unique_ptr<ImGUI_Impl> imGUI = windowSystem->createGUI(video->getWindow());
		shared_ptr<PBR_Deferred_MainLoopTask> mainLoop = make_shared<PBR_Deferred_MainLoopTask>(renderer.get());
		ControllerStateMachine controllerSM(std::make_unique<App::EditMode>(window, input, mainLoop.get(), camera.get(), imGUI.get(), std::unique_ptr<nex::engine::gui::Drawable>()));

		window->activate();
		mainLoop->init(window->getWidth(), window->getHeight());
		controllerSM.init();
		setupCamera(camera.get(), window, input);
		setupCallbacks(window, camera.get(), mainLoop.get(), renderer.get(), logger);
		setupGUI(&controllerSM, mainLoop.get());

		SceneNode* scene = createScene();

		run(windowSystem, window, mainLoop.get(), &controllerSM, imGUI.get(), camera.get(), baseTitle.c_str(), timer, counter, scene);

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

void readConfig(shared_ptr<Video> video, Renderer3D* renderer, Engine* engine)
{
	video->useRenderer(renderer);
	//video->useUISystem(ui);
	engine->add(video);
	engine->setConfigFileName("config.ini");
	engine->init();
}


void run(SubSystemProvider* windowSystem, Window* window, PBR_Deferred_MainLoopTask* task, ControllerStateMachine* controllerSM, ImGUI_Impl* gui,
	Camera* camera, const char* baseTitle, Timer& timer, FPSCounter& counter, SceneNode* scene)
{
	window->activate();

	while (window->isOpen())
	{
		// Poll input events before checking if the app is running, otherwise 
		// the window is likely to hang or crash (at least on windows platform)
		windowSystem->pollEvents();

		float frameTime = timer.update();
		float fps = counter.update(frameTime);
		updateWindowTitle(window, baseTitle, frameTime, fps);

		if (task->isRunning())
		{
			controllerSM->frameUpdate(frameTime);
			task->run(scene, camera, frameTime, window->getWidth(), window->getHeight());

			gui->newFrame();
			controllerSM->getCurrentController()->getDrawable()->drawGUI();
			ImGui::Render();
			gui->renderDrawData(ImGui::GetDrawData());

			// present rendered frame
			window->swapBuffers();
		}
		else
		{
			this_thread::sleep_for(chrono::milliseconds(500));
		}
	}
}

SceneNode* createScene()
{
	nodes.push_back(SceneNode());
	SceneNode* root = &nodes.back();

	nodes.push_back(SceneNode());
	SceneNode* ground = &nodes.back();
	root->addChild(ground);

	nodes.push_back(SceneNode());
	SceneNode* cube1 = &nodes.back();
	root->addChild(cube1);

	nodes.push_back(SceneNode());
	SceneNode* sphere = &nodes.back();
	root->addChild(sphere);

	vobs.push_back(Vob("misc/textured_plane.obj", Shaders::Pbr));
	ground->setVob(&vobs.back());
	//vobs.push_back(Vob("misc/textured_cube.obj"));
	vobs.push_back(Vob("normal_map_test/normal_map_test.obj", Shaders::Pbr));
	cube1->setVob(&vobs.back());

	vobs.push_back(Vob("normal_map_test/normal_map_sphere.obj", Shaders::Pbr));
	sphere->setVob(&vobs.back());

	ground->getVob()->setPosition({ 10, 0, 0 });
	cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });

	sphere->getVob()->setPosition({ 3.0f, 3.8f, -1.0f });

	return root;
}


void setupCallbacks(Window* window, Camera* camera, PBR_Deferred_MainLoopTask* task, Renderer3D* renderer, platform::LoggingClient& logClient)
{
	using namespace placeholders;

	Input* input = window->getInputDevice();

	//auto focusCallback = bind(&PBR_Deferred_MainLoopTask::onWindowsFocus, this, placeholders::_1, placeholders::_2);
	auto scrollCallback = bind(&Camera::onScroll, camera, placeholders::_1, placeholders::_2);
	
	input->addWindowFocusCallback([=](Window* window_s, bool receivedFocus)
	{
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
		LOG(logClient, platform::Warning) << "addRefreshCallback : called!";
		if (!window->hasFocus()) {
			LOG(logClient, platform::Warning) << "addRefreshCallback : no focus!";
		}
	});
}

void setupCamera(Camera* camera, Window* window, Input* input)
{

	if (TrackballQuatCamera* casted = dynamic_cast<TrackballQuatCamera*>(camera))
	{
		auto cameraResizeCallback = bind(&TrackballQuatCamera::updateOnResize, casted, placeholders::_1, placeholders::_2);
		casted->updateOnResize(window->getWidth(), window->getHeight());
		input->addResizeCallback(cameraResizeCallback);
	}

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
	static float runtime = 0;

	runtime += frameTime;

	// Update title every second
	if (runtime > 1)
	{
		stringstream ss;
		/*ss.precision(2);
		ss << fixed << fps;
		std::string temp = ss.str();
		ss.str("");

		ss << baseTitle << " : FPS = " << std::setw(6) << std::setfill(' ') << temp << " : frame time (ms) = " << frameTime * 1000;
		window->setTitle(ss.str());*/

		ss << baseTitle << " : FPS = " << fps;
		window->setTitle(ss.str());

		runtime = 0;
	}
}