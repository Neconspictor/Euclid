#include <NeXEngine.hpp>
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
#include <nex/util/ExceptionHandling.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/common/Log.hpp>
#include "nex/exception/EnumFormatException.hpp"
#include "nex/util/Globals.hpp"

NeXEngine::NeXEngine(SubSystemProvider* provider) :
	m_logger("NeX-Engine"),
	m_windowSystem(provider),
	m_window(nullptr), 
	m_input(nullptr), 
	m_scene(nullptr),
	m_isRunning(false),
	m_systemLogLevel(nex::Debug),
	m_configFileName("config.ini")
{
	m_config.addOption("Logging", "logLevel", &m_systemLogLevelStr, std::string(""));
	m_config.addOption("General", "rootDirectory", &m_systemLogLevelStr, std::string("./"));
}

NeXEngine::~NeXEngine()
{
	m_windowSystem = nullptr;
}

nex::LogLevel NeXEngine::getLogLevel() const
{
	return m_systemLogLevel;
}

void NeXEngine::init()
{

	LOG(m_logger, nex::Info) << "Initializing Engine...";

	m_renderBackend = std::make_unique<RendererOpenGL>();


	m_video.handle(m_config);
	Configuration::setGlobalConfiguration(&m_config);
	readConfig();
	
	util::Globals::initGlobals();
	LOG(m_logger, nex::Info) << "root Directory = " << ::util::Globals::getRootDirectory();


	m_window = createWindow();
	m_input = m_window->getInputDevice();
	m_camera = std::make_unique<FPCamera>(FPCamera());
	m_baseTitle = m_window->getTitle();


	//init render backend
	initRenderBackend();

	// init shader file system
	ShaderProgramGL::getShaderFileSystem()->addIncludeDirectory(util::Globals::getOpenGLShaderPath());

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

	m_scene->init(m_renderBackend->getModelManager());

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
			m_scene->update(frameTime);
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
}


void NeXEngine::setConfigFileName(const char*  fileName)
{
	m_configFileName = fileName;
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
	m_vobs.push_back(Vob("misc/textured_plane.obj", ShaderType::Pbr));
	ground->vob = &m_vobs.back();
	ground->vob->setPosition({ 10, 0, 0 });
	root->addChild(ground);

	/*m_nodes.push_back(SceneNode());
	SceneNode* cerberus = &m_nodes.back();
	m_vobs.push_back(Vob("cerberus/cerberus.obj", Shaders::Pbr));
	cerberus->setVob(&m_vobs.back());
	root->addChild(cerberus);*/

	m_nodes.push_back(SceneNode());
	SceneNode* cube1 = &m_nodes.back();
	m_vobs.push_back(Vob("normal_map_test/normal_map_test.obj", ShaderType::Pbr));
	cube1->vob = &m_vobs.back();
	cube1->vob->setPosition({ 0.0f, 1.3f, 0.0f });
	root->addChild(cube1);

	m_nodes.push_back(SceneNode());
	SceneNode* sphere = &m_nodes.back();
	m_vobs.push_back(Vob("normal_map_test/normal_map_sphere.obj", ShaderType::Pbr));
	sphere->vob = &m_vobs.back();
	sphere->vob->setPosition({ 3.0f, 3.8f, -1.0f });
	root->addChild(sphere);

	//m_nodes.push_back(SceneNode());
	//SceneNode* cube1 = &m_nodes.back();
	//root->addChild(cube1);

	//m_nodes.push_back(SceneNode());
	//SceneNode* sphere = &m_nodes.back();
	//root->addChild(sphere);

	//m_vobs.push_back(Vob("sponza/firstTest.obj", Shaders::Pbr));
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

Window* NeXEngine::createWindow()
{
	Window::WindowStruct desc;
	desc.title = m_video.windowTitle;
	desc.fullscreen = m_video.fullscreen;
	desc.colorBitDepth = m_video.colorBitDepth;
	desc.refreshRate = m_video.refreshRate;
	desc.posX = 0;
	desc.posY = 0;
	desc.width = m_video.width;
	desc.height = m_video.height;
	desc.visible = true;
	desc.vSync = m_video.vSync;

	return m_windowSystem->createWindow(desc);
}

void NeXEngine::initRenderBackend()
{
	m_window->activate();
	m_renderBackend->setViewPort(0, 0, m_video.width, m_video.height);
	m_renderBackend->setMSAASamples(m_video.msaaSamples);
	m_renderBackend->init();
}


void NeXEngine::readConfig()
{
	LOG(m_logger, nex::Info) << "Loading configuration file...";
	if (!m_config.load(m_configFileName))
	{
		LOG(m_logger, nex::Warning) << "Configuration file couldn't be read. Default values are used.";
	}
	else
	{
		LOG(m_logger, nex::Info) << "Configuration file loaded.";
	}

	try
	{
		m_systemLogLevel = nex::stringToLogLevel(m_systemLogLevelStr);
	}
	catch (const EnumFormatException& e)
	{

		//log error and set default log level
		LOG(m_logger, nex::Error) << e.what();

		LOG(m_logger, nex::Warning) << "Couldn't get log level from " << m_systemLogLevelStr << std::endl
			<< "Log level is set now to 'Warning'" << std::endl;

		m_systemLogLevel = nex::Warning;
		m_systemLogLevelStr = "Warning";
	}

	nex::LoggerManager::get()->setMinLogLevel(m_systemLogLevel);
	m_config.write(m_configFileName);
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
			LOG(m_logger, nex::Debug) << "received focus!";
			//isRunning = true;
		}
		else
		{
			LOG(m_logger, nex::Debug) << "lost focus!";
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
		LOG(m_logger, nex::Debug) << "addResizeCallback : width: " << width << ", height: " << height;

		if (!m_window->hasFocus()) {
			LOG(m_logger, nex::Debug) << "addResizeCallback : no focus!";
		}

		if (width == 0 || height == 0) {
			LOG(m_logger, nex::Warning) << "addResizeCallback : width or height is 0!";
			return;
		}

		m_camera->setAspectRatio((float)width / (float)height);

		m_renderer->updateRenderTargets(width, height);
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
	graphicsTechniques->addChild(std::move(ssaoView));

	auto pbrDeferredView = std::make_unique<PBR_Deferred_ConfigurationView>(m_renderer->getPBR());
	graphicsTechniques->addChild(std::move(pbrDeferredView));

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
	frustum.farPlane = 100.0f;
	m_camera->setOrthoFrustum(frustum);
	m_camera->setNearPlane(0.1f);
	m_camera->setFarPlane(150.0f);
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