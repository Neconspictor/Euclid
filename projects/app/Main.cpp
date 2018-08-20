#include <Main.hpp>
#include <nex/system/Engine.hpp>
#include <nex/system/Video.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <PBR_MainLoopTask.hpp>
#include <pbr_deferred/PBR_Deferred_MainLoopTask.hpp>
#include <MainLoopTask.hpp>
#include <nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp>
#include <boost/locale.hpp>
#include <thread>
#include <glm/glm.hpp>
#include <nex/camera/TrackballQuatCamera.hpp>
#include <nex/camera/FPCamera.hpp>
#include <gui/AppStyle.hpp>
#include <gui/ConfigurationWindow.hpp>
#include <gui/SceneGUI.hpp>
#include <gui/Controller.hpp>


//#include <Brofiler.h>

using namespace std;


int main(int argc, char** argv)
{
	nex::LoggingClient logger(nex::getLogServer());

	try {
		
		Main mainClass;
		mainClass.run();
		LOG(logger, nex::Info) << "Done.";

	} catch (const exception& e)
	{
		LOG(logger, nex::Fault) << "Exception: " << typeid(e).name() << ": "<< e.what();
	} catch(...)
	{
		LOG(logger, nex::Fault) << "Unknown Exception occurred.";
	}

	nex::shutdownLogServer();

	return EXIT_SUCCESS;
}

Main::Main() : m_logClient(nex::getLogServer())
{
	using namespace std;

	m_logClient.setPrefix("Main");

	m_windowSystem = SubSystemProviderGLFW::get();
	if (!m_windowSystem->init())
	{
		//LOG(m_logClient, platform::Fault) << "Couldn't initialize window system!";
		nex::getLogServer()->terminate();
		throw std::runtime_error("Couldn't initialize window system!");
	}

	m_renderer = make_unique<RendererOpenGL>();
	m_engine = make_unique<Engine>();
	m_video = std::make_shared<Video>(m_windowSystem);
	m_video->useRenderer(m_renderer.get());

	LOG(m_logClient, nex::Info) << "Starting Engine...";
	readConfig();

	m_window = m_video->getWindow();
	m_input = m_window->getInputDevice();
	m_camera = make_unique<FPCamera>(FPCamera());
	m_baseTitle = m_window->getTitle();

	m_gui = m_windowSystem->createGUI(m_window);
	m_task = make_unique<PBR_Deferred_MainLoopTask>(m_renderer.get());
	m_controllerSM = make_unique<ControllerStateMachine>(std::make_unique<App::EditMode>(m_window, 
		m_input, 
		m_task.get(), 
		m_camera.get(), 
		m_gui.get(),
		std::unique_ptr<nex::engine::gui::Drawable>()));

	m_window->activate();
	m_task->init(m_window->getWidth(), m_window->getHeight());
	m_controllerSM->init();
	setupCamera();
	setupCallbacks();
	setupGUI();
	
	m_scene = createScene();
}

Main::~Main()
{
	if (m_windowSystem)
		m_windowSystem->terminate();
	m_windowSystem = nullptr;
}

void Main::run()
{
	m_window->activate();

	while (m_window->isOpen())
	{
		// Poll input events before checking if the app is running, otherwise 
		// the window is likely to hang or crash (at least on windows platform)
		m_windowSystem->pollEvents();

		float frameTime = m_timer.update();
		float fps = m_counter.update(frameTime);
		updateWindowTitle(frameTime, fps);

		if (m_task->isRunning())
		{
			m_controllerSM->frameUpdate(frameTime);
			m_task->run(m_scene, m_camera.get(), frameTime, m_window->getWidth(), m_window->getHeight());

			m_gui->newFrame();
			m_controllerSM->getCurrentController()->getDrawable()->drawGUI();
			ImGui::Render();
			m_gui->renderDrawData(ImGui::GetDrawData());

			// present rendered frame
			m_window->swapBuffers();
		}
		else
		{
			this_thread::sleep_for(chrono::milliseconds(500));
		}
	}

	m_engine->stop();
}

SceneNode* Main::createScene()
{
	m_nodes.push_back(SceneNode());
	SceneNode* root = &m_nodes.back();

	m_nodes.push_back(SceneNode());
	SceneNode* ground = &m_nodes.back();
	root->addChild(ground);

	m_nodes.push_back(SceneNode());
	SceneNode* cube1 = &m_nodes.back();
	root->addChild(cube1);

	m_nodes.push_back(SceneNode());
	SceneNode* sphere = &m_nodes.back();
	root->addChild(sphere);

	m_vobs.push_back(Vob("misc/textured_plane.obj", Shaders::Pbr));
	ground->setVob(&m_vobs.back());
	//vobs.push_back(Vob("misc/textured_cube.obj"));
	m_vobs.push_back(Vob("normal_map_test/normal_map_test.obj", Shaders::Pbr));
	cube1->setVob(&m_vobs.back());

	m_vobs.push_back(Vob("normal_map_test/normal_map_sphere.obj", Shaders::Pbr));
	sphere->setVob(&m_vobs.back());

	ground->getVob()->setPosition({ 10, 0, 0 });
	cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });

	sphere->getVob()->setPosition({ 3.0f, 3.8f, -1.0f });

	return root;
}

void Main::readConfig()
{
	m_video->useRenderer(m_renderer.get());
	m_engine->add(m_video);
	m_engine->setConfigFileName("config.ini");
	m_engine->init();
}

void Main::setupCallbacks()
{
	using namespace placeholders;

	Input* input = m_window->getInputDevice();

	//auto focusCallback = bind(&PBR_Deferred_MainLoopTask::onWindowsFocus, this, placeholders::_1, placeholders::_2);
	auto scrollCallback = bind(&Camera::onScroll, m_camera.get(), placeholders::_1, placeholders::_2);

	input->addWindowFocusCallback([=](Window* window_s, bool receivedFocus)
	{
		m_task->setRunning(receivedFocus);
		if (receivedFocus)
		{
			LOG(m_logClient, nex::Debug) << "received focus!";
			//isRunning = true;
		}
		else
		{
			LOG(m_logClient, nex::Debug) << "lost focus!";
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
		LOG(m_logClient, nex::Debug) << "addResizeCallback : width: " << width << ", height: " << height;

		if (!m_window->hasFocus()) {
			LOG(m_logClient, nex::Debug) << "addResizeCallback : no focus!";
		}

		if (width == 0 || height == 0) {
			LOG(m_logClient, nex::Warning) << "addResizeCallback : width or height is 0!";
			return;
		}

		m_camera->setAspectRatio((float)width / (float)height);

		m_task->updateRenderTargets(width, height);
	});

	input->addRefreshCallback([=]() {
		LOG(m_logClient, nex::Warning) << "addRefreshCallback : called!";
		if (!m_window->hasFocus()) {
			LOG(m_logClient, nex::Warning) << "addRefreshCallback : no focus!";
		}
	});
}

void Main::setupGUI()
{
	using namespace nex::engine::gui;

	App::AppStyle style;
	style.apply();

	std::unique_ptr<SceneGUI> root = std::make_unique<SceneGUI>(m_controllerSM.get());
	std::unique_ptr<Drawable> configurationWindow = std::make_unique<App::ConfigurationWindow>("Graphics Configuration Window", root->getMainMenuBar());

	for (int i = 0; i < 5; ++i)
	{
		std::unique_ptr<hbao::HBAO_ConfigurationView> hbaoView = std::make_unique<hbao::HBAO_ConfigurationView>(m_task->getHBAO(),
			root->getOptionMenu(), configurationWindow.get(), "HBAO");
		//hbaoView->setVisible(true);
		configurationWindow->addChild(move(hbaoView));
	}

	configurationWindow->useStyleClass(make_shared<App::ConfigurationStyle2>());
	root->addChild(move(configurationWindow));

	m_controllerSM->getCurrentController()->setDrawable(move(root));
}

void Main::setupCamera()
{
	if (TrackballQuatCamera* casted = dynamic_cast<TrackballQuatCamera*>(m_camera.get()))
	{
		auto cameraResizeCallback = bind(&TrackballQuatCamera::updateOnResize, casted, placeholders::_1, placeholders::_2);
		casted->updateOnResize(m_window->getWidth(), m_window->getHeight());
		m_input->addResizeCallback(cameraResizeCallback);
	}

	int windowWidth = m_window->getWidth();
	int windowHeight = m_window->getHeight();

	m_camera->setPosition(glm::vec3(0.0f, 3.0f, 2.0f));
	m_camera->setLook(glm::vec3(0.0f, 0.0f, -1.0f));
	m_camera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera->setAspectRatio((float)windowWidth / (float)windowHeight);

	Frustum frustum = m_camera->getFrustum(Perspective);
	frustum.left = -10.0f;
	frustum.right = 10.0f;
	frustum.bottom = -10.0f;
	frustum.top = 10.0f;
	frustum.nearPlane = 0.1f;
	frustum.farPlane = 10.0f;
	m_camera->setOrthoFrustum(frustum);
}

void Main::updateWindowTitle(float frameTime, float fps)
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

		ss << m_baseTitle << " : FPS = " << fps;
		m_window->setTitle(ss.str());

		runtime = 0;
	}
}