#include <NeXEngine.hpp>
#include <nex/system/Engine.hpp>
#include <nex/system/Video.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <pbr_deferred/PBR_Deferred_Renderer.hpp>
#include <nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp>
#include <glm/glm.hpp>
#include <nex/camera/TrackballQuatCamera.hpp>
#include <nex/camera/FPCamera.hpp>
#include <gui/AppStyle.hpp>
#include <gui/ConfigurationWindow.hpp>
#include <gui/SceneGUI.hpp>
#include <gui/Controller.hpp>
#include <boxer/boxer.h>

NeXEngine::NeXEngine() :
	Engine(),
	m_logClient(nex::getLogServer()), 
	m_windowSystem(nullptr), 
	m_window(nullptr), 
	m_input(nullptr), 
	m_scene(nullptr),
	m_isRunning(false)
{
	m_logClient.setPrefix("[NeX-Engine]");
}

NeXEngine::~NeXEngine()
{
	if (m_windowSystem)
		m_windowSystem->terminate();
	m_windowSystem = nullptr;
}

void NeXEngine::init()
{

	LOG(m_logClient, nex::Info) << "Initializing Engine...";

	m_windowSystem = SubSystemProviderGLFW::get();
	if (!m_windowSystem->init())
	{
		//LOG(m_logClient, platform::Fault) << "Couldn't initialize window system!";
		nex::getLogServer()->terminate();
		throw_with_trace(std::runtime_error("Couldn't initialize window system!"));
	}

	m_renderBackend = std::make_unique<RendererOpenGL>();
	m_video = std::make_shared<Video>(m_windowSystem);
	m_video->useRenderer(m_renderBackend.get());

	m_video->useRenderer(m_renderBackend.get());
	add(m_video);
	setConfigFileName("config.ini");
	Engine::init();

	m_window = m_video->getWindow();
	m_input = m_window->getInputDevice();
	m_camera = std::make_unique<FPCamera>(FPCamera());
	m_baseTitle = m_window->getTitle();

	m_gui = m_windowSystem->createGUI(m_window);
	m_renderer = std::make_unique<PBR_Deferred_Renderer>(m_renderBackend.get());
	m_controllerSM = std::make_unique<ControllerStateMachine>(std::make_unique<App::EditMode>(m_window,
		m_input,
		m_renderer.get(),
		m_camera.get(),
		m_gui.get(),
		std::unique_ptr<nex::engine::gui::Drawable>()));

	m_window->activate();
	m_renderer->init(m_window->getWidth(), m_window->getHeight());
	m_controllerSM->init();
	setupCamera();
	setupCallbacks();
	setupGUI();

	m_scene = createScene();

	m_input->addWindowCloseCallback([](Window* window)
	{
		void* nativeWindow = window->getNativeWindow();
		boxer::Selection selection = boxer::show("Do you really want to quit?", "Exit NeX", boxer::Style::Warning, boxer::Buttons::OKCancel, nativeWindow);
		if (selection == boxer::Selection::Cancel)
		{
			window->reopen();
		}
	});
}

bool NeXEngine::isRunning() const
{
	return m_isRunning;
}

void NeXEngine::run()
{
	m_isRunning = true;
	m_window->activate();

	while (m_window->isOpen())
	{
		// Poll input events before checking if the app is running, otherwise 
		// the window is likely to hang or crash (at least on windows platform)
		m_windowSystem->pollEvents();

		float frameTime = m_timer.update();
		float fps = m_counter.update(frameTime);
		updateWindowTitle(frameTime, fps);

		if (isRunning())
		{
			m_controllerSM->frameUpdate(frameTime);
			m_camera->Projectional::update(true);
			m_renderer->render(m_scene, m_camera.get(), frameTime, m_window->getWidth(), m_window->getHeight());

			m_gui->newFrame();
			m_controllerSM->getCurrentController()->getDrawable()->drawGUI();
			ImGui::Render();
			m_gui->renderDrawData(ImGui::GetDrawData());
			
			// present rendered frame
			m_window->swapBuffers();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}

	stop();
}

void NeXEngine::setRunning(bool isRunning)
{
	m_isRunning = isRunning;
}

SceneNode* NeXEngine::createScene()
{
	m_nodes.push_back(SceneNode());
	SceneNode* root = &m_nodes.back();

	m_nodes.push_back(SceneNode());
	SceneNode* ground = &m_nodes.back();
	root->addChild(ground);

	//m_nodes.push_back(SceneNode());
	//SceneNode* cube1 = &m_nodes.back();
	//root->addChild(cube1);

	//m_nodes.push_back(SceneNode());
	//SceneNode* sphere = &m_nodes.back();
	//root->addChild(sphere);

	//m_vobs.push_back(Vob("misc/textured_plane.obj", Shaders::Pbr));
	m_vobs.push_back(Vob("sponza/firstTest.obj", Shaders::Pbr));
	ground->setVob(&m_vobs.back());
	//vobs.push_back(Vob("misc/textured_cube.obj"));
	//m_vobs.push_back(Vob("normal_map_test/normal_map_test.obj", Shaders::Pbr));
	//cube1->setVob(&m_vobs.back());

	//m_vobs.push_back(Vob("normal_map_test/normal_map_sphere.obj", Shaders::Pbr));
	//sphere->setVob(&m_vobs.back());

	//ground->getVob()->setPosition({ 10, 0, 0 });
	//cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });

	//sphere->getVob()->setPosition({ 3.0f, 3.8f, -1.0f });

	return root;
}

void NeXEngine::setupCallbacks()
{
	Input* input = m_window->getInputDevice();

	//auto focusCallback = bind(&PBR_Deferred_Renderer::onWindowsFocus, this, placeholders::_1, placeholders::_2);
	//auto scrollCallback = std::bind(&Camera::onScroll, m_camera.get(), std::placeholders::_1, std::placeholders::_2);

	input->addWindowFocusCallback([=](Window* window_s, bool receivedFocus)
	{
		setRunning(receivedFocus);
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
	//input->addScrollCallback(scrollCallback);

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

		m_renderer->updateRenderTargets(width, height);
	});

	input->addRefreshCallback([=]() {
		LOG(m_logClient, nex::Warning) << "addRefreshCallback : called!";
		if (!m_window->hasFocus()) {
			LOG(m_logClient, nex::Warning) << "addRefreshCallback : no focus!";
		}
	});
}

void NeXEngine::setupGUI()
{
	using namespace nex::engine::gui;

	App::AppStyle style;
	style.apply();

	std::unique_ptr<SceneGUI> root = std::make_unique<SceneGUI>(m_controllerSM.get());
	std::unique_ptr<App::ConfigurationWindow> configurationWindow = std::make_unique<App::ConfigurationWindow>(root->getMainMenuBar(), root->getOptionMenu());

	Tab* graphicsTechniques = configurationWindow->getGraphicsTechniquesTab();
	Tab* cameraTab = configurationWindow->getCameraTab();
	Tab* videoTab = configurationWindow->getVideoTab();
	Tab* generalTab = configurationWindow->getGeneralTab();


	auto hbaoView = std::make_unique<hbao::HBAO_ConfigurationView>(m_renderer->getHBAO());
	graphicsTechniques->addChild(move(hbaoView));

	auto ssaoView = std::make_unique<SSAO_ConfigurationView>(m_renderer->getAOSelector()->getSSAO());
	graphicsTechniques->addChild(move(ssaoView));

	auto cameraView = std::make_unique<FPCamera_ConfigurationView>(static_cast<FPCamera*>(m_camera.get()));
	cameraTab->addChild(move(cameraView));


	auto windowView = std::make_unique<Window_ConfigurationView>(m_window);
	videoTab->addChild(move(windowView));

	auto textureManagerView = std::make_unique<TextureManager_Configuration>(m_renderBackend->getTextureManager());
	generalTab->addChild(move(textureManagerView));

	auto pbr_deferred_rendererView = std::make_unique<PBR_Deferred_Renderer_ConfigurationView>(m_renderer.get());
	generalTab->addChild(move(pbr_deferred_rendererView));

	configurationWindow->useStyleClass(std::make_shared<App::ConfigurationStyle>());
	root->addChild(move(configurationWindow));

	m_controllerSM->getCurrentController()->setDrawable(move(root));
}

void NeXEngine::setupCamera()
{
	if (TrackballQuatCamera* casted = dynamic_cast<TrackballQuatCamera*>(m_camera.get()))
	{
		auto cameraResizeCallback = std::bind(&TrackballQuatCamera::updateOnResize, casted, std::placeholders::_1, std::placeholders::_2);
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

void NeXEngine::updateWindowTitle(float frameTime, float fps)
{
	static float runtime = 0;

	runtime += frameTime;

	// Update title every second
	if (runtime > 1)
	{
		static std::stringstream ss;
		/*ss.precision(2);
		ss << fixed << fps;
		std::string temp = ss.str();
		ss.str("");

		ss << baseTitle << " : FPS = " << std::setw(6) << std::setfill(' ') << temp << " : frame time (ms) = " << frameTime * 1000;
		window->setTitle(ss.str());*/

		ss << m_baseTitle << " : FPS = " << fps;
		m_window->setTitle(ss.str());
		ss.str("");
		runtime = 0;
	}
}