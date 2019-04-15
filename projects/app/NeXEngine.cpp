#include <NeXEngine.hpp>
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
#include <nex/texture/TextureManager.hpp>
#include <nex/common/Log.hpp>
#include <nex/exception/EnumFormatException.hpp>
#include <Globals.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/shader_generator/ShaderSourceFileGenerator.hpp"
#include "nex/RenderBackend.hpp"
#include "nex/mesh/Vob.hpp"
#include "nex/material/Material.hpp"
#include "nex/pbr/Pbr.hpp"
#include "nex/post_processing/HBAO.hpp"
#include "nex/post_processing/SSAO.hpp"
#include "nex/post_processing/AmbientOcclusion.hpp"
#include "nex/pbr/PbrForward.hpp"
#include "nex/pbr/PbrDeferred.hpp"
#include "nex/pbr/PbrProbe.hpp"
#include <nex/shadow/CascadedShadow.hpp>

using namespace nex;


NeXEngine::NeXEngine(SubSystemProvider* provider) :
	mLogger("NeX-Engine"),
	mWindowSystem(provider),
	mWindow(nullptr),
	mInput(nullptr),
	mScene(nullptr),
	mIsRunning(false),
	mConfigFileName("config.ini"),
	mSystemLogLevel(nex::Debug),
	panoramaSky(nullptr)
{
	mConfig.addOption("Logging", "logLevel", &mSystemLogLevelStr, std::string(""));
	mConfig.addOption("General", "rootDirectory", &mSystemLogLevelStr, std::string("./"));
}

NeXEngine::~NeXEngine()
{
	mWindowSystem = nullptr;
}

nex::LogLevel NeXEngine::getLogLevel() const
{
	return mSystemLogLevel;
}

void NeXEngine::init()
{

	LOG(mLogger, nex::Info) << "Initializing Engine...";


	mVideo.handle(mConfig);
	Configuration::setGlobalConfiguration(&mConfig);
	readConfig();
	
	nex::util::Globals::initGlobals();
	LOG(mLogger, nex::Info) << "root Directory = " << ::util::Globals::getRootDirectory();


	mWindow = createWindow();
	mInput = mWindow->getInputDevice();
	mCamera = std::make_unique<FPCamera>(FPCamera());
	mBaseTitle = mWindow->getTitle();


	// init texture manager (filesystem)
	mTextureFileSystem.addIncludeDirectory(util::Globals::getTexturePath());
	TextureManager::get()->init(&mTextureFileSystem);

	// init shader file system
	mShaderFileSystem.addIncludeDirectory(util::Globals::getOpenGLShaderPath());
	ShaderSourceFileGenerator::get()->init(&mShaderFileSystem);

	//init render backend
	initRenderBackend();

	initLights();

	//init pbr 
	initPbr();

	// init mesh manager (filesystem)
	mMeshFileSystem.addIncludeDirectory(util::Globals::getMeshesPath());
	StaticMeshManager::get()->init(&mMeshFileSystem, 
		std::make_unique<PbrMaterialLoader>(mPbrDeferred.get(), mPbrForward.get(), TextureManager::get()));
	//StaticMeshManager::get()->setPbrMaterialLoader(std::make_unique<PbrMaterialLoader>(mPbrDeferred.get(), mPbrForward.get(), TextureManager::get()));


	initProbes();


	mRenderer = std::make_unique<PBR_Deferred_Renderer>(RenderBackend::get(), 
		mPbrDeferred.get(), 
		mPbrForward.get(), 
		mCascadedShadow.get(), 
		mWindow->getInputDevice());


	mGui = mWindowSystem->createGUI(mWindow);
	
	

	mControllerSM = std::make_unique<gui::ControllerStateMachine>(std::make_unique<nex::gui::EditMode>(mWindow,
		mInput,
		mRenderer.get(),
		mCamera.get(),
		mGui.get(),
		std::unique_ptr<nex::gui::Drawable>()));

	mWindow->activate();

	mCursor = std::make_unique<Cursor>(StandardCursorType::Hand);
	mWindow->setCursor(mCursor.get());


	mRenderer->init(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
	mControllerSM->init();
	setupCamera();
	setupCallbacks();
	setupGUI();

	mScene = createScene();

	mScene->init();

	mInput->addWindowCloseCallback([](Window* window)
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
	return mIsRunning;
}

void NeXEngine::run()
{
	mIsRunning = true;
	mWindow->activate();

	SimpleTimer timer;

	while (mWindow->isOpen())
	{
		// Poll input events before checking if the app is running, otherwise 
		// the window is likely to hang or crash (at least on windows platform)
		mWindowSystem->pollEvents();

		mTimer.update();
		float frameTime = mTimer.getTimeInSeconds();
		//timer.update(ImGui::GetTime());
		//float frameTime = timer.diff;
		
		float fps = mCounter.update(frameTime);

		updateWindowTitle(frameTime, fps);

		if (isRunning())
		{

			mGui->newFrame(frameTime);
			mScene->update(frameTime);
			mControllerSM->frameUpdate(frameTime);
			mCamera->Projectional::update(true);

			mRenderer->render(mScene, mCamera.get(), &mSun, frameTime, mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
			mControllerSM->getCurrentController()->getDrawable()->drawGUI();
			
			ImGui::Render();
			mGui->renderDrawData(ImGui::GetDrawData());
			
			// present rendered frame
			mWindow->swapBuffers();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
}


void NeXEngine::setConfigFileName(const char*  fileName)
{
	mConfigFileName = fileName;
}

void NeXEngine::setRunning(bool isRunning)
{
	mIsRunning = isRunning;
}

SceneNode* NeXEngine::createScene()
{
	mNodes.emplace_back(SceneNode());
	SceneNode* root = &mNodes.back();

	mNodes.emplace_back(SceneNode());
	SceneNode* ground = &mNodes.back();
	mVobs.emplace_back(Vob("misc/textured_plane.obj", MaterialType::Pbr));
	ground->vob = &mVobs.back();
	ground->vob->setPosition({ 10, 0, 0 });
	root->addChild(ground);

	/*m_nodes.emplace_back(SceneNode());
	SceneNode* cerberus = &m_nodes.back();
	m_vobs.emplace_back(Vob("cerberus/cerberus.obj", MaterialType::Pbr));
	cerberus->vob = &m_vobs.back();
	root->addChild(cerberus);

	m_nodes.emplace_back(SceneNode());
	SceneNode* cube1 = &m_nodes.back();
	m_vobs.emplace_back(Vob("normal_map_test/normal_map_test.obj", MaterialType::Pbr));
	cube1->vob = &m_vobs.back();
	cube1->vob->setPosition({ 0.0f, 1.3f, 0.0f });
	root->addChild(cube1);

	m_nodes.emplace_back(SceneNode());
	SceneNode* sphere = &m_nodes.back();
	m_vobs.emplace_back(Vob("normal_map_test/normal_map_sphere.obj", MaterialType::Pbr));
	sphere->vob = &m_vobs.back();
	sphere->vob->setPosition({ 3.0f, 3.8f, -1.0f });
	root->addChild(sphere);


	auto* textureManager = TextureManager::get();
	TextureData data = {
			TextureFilter::Linear_Mipmap_Linear,
			TextureFilter::Linear,
			TextureUVTechnique::Repeat,
			TextureUVTechnique::Repeat,
			TextureUVTechnique::Repeat,
			ColorSpace::SRGBA,
			PixelDataType::UBYTE,
			InternFormat::SRGBA8,
			true
	};

	m_nodes.emplace_back(SceneNode());
	SceneNode* sphere2 = &m_nodes.back();
	auto material = std::make_unique<PbrMaterial>();
	material->setAlbedoMap(textureManager->getImage("pbr/albedo.png", data));
	

	data.colorspace = ColorSpace::RGBA;
	data.internalFormat = InternFormat::RGBA8;
	material->setAoMap(textureManager->getImage("pbr/ao.png", data));
	material->setNormalMap(textureManager->getImage("pbr/normal.png", data));
	material->setEmissionMap(textureManager->getDefaultBlackTexture());
	material->setMetallicMap(textureManager->getDefaultBlackTexture());
	material->setRoughnessMap(textureManager->getImage("pbr/roughness.png", data));
	mModels.emplace_back(StaticMeshManager::createSphere(16, 16, std::move(material)));


	m_vobs.emplace_back(Vob(mModels.back().get()));
	sphere2->vob = &m_vobs.back();
	sphere2->vob->setPosition({ 4.0f, 4.8f, -1.0f });
	root->addChild(sphere2);*/

	//m_nodes.push_back(SceneNode());
	//SceneNode* cube1 = &m_nodes.back();
	//root->addChild(cube1);

	//m_nodes.push_back(SceneNode());
	//SceneNode* sphere = &m_nodes.back();
	//root->addChild(sphere);

	//m_vobs.push_back(Vob("sponza/firstTest.obj", Shaders::Pbr));
	//m_vobs.push_back(Vob("misc/textured_cube.obj"));
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
	desc.title = mVideo.windowTitle;
	desc.fullscreen = mVideo.fullscreen;
	desc.colorBitDepth = mVideo.colorBitDepth;
	desc.refreshRate = mVideo.refreshRate;
	desc.posX = 0;
	desc.posY = 0;

	// Note that framebuffer width and height are inferred!
	desc.virtualScreenWidth = mVideo.width;
	desc.virtualScreenHeight = mVideo.height;
	desc.visible = true;
	desc.vSync = mVideo.vSync;

	return mWindowSystem->createWindow(desc);
}

void NeXEngine::initLights()
{
	mSun.setColor(glm::vec3(1.0f, 1.0f, 1.0f));
	mSun.setPower(3.0f);
	mSun.setDirection({ -1,-1,-1 });
}

void NeXEngine::initPbr()
{
	CascadedShadow::PCFFilter pcf;
	pcf.sampleCountX = 2;
	pcf.sampleCountY = 2;
	pcf.useLerpFiltering = true;
	mCascadedShadow = std::make_unique<CascadedShadow>(2048, 2048, 4, pcf, 6.0f, true);


	mPbrDeferred = std::make_unique<PbrDeferred>(&mAmbientLight, mCascadedShadow.get(), &mSun, nullptr);
	mPbrForward = std::make_unique<PbrForward>(&mAmbientLight, mCascadedShadow.get(), &mSun, nullptr);
}

void NeXEngine::initProbes()
{
	TextureManager* textureManager = TextureManager::get();

	panoramaSky = textureManager->getHDRImage("hdr/HDR_040_Field.hdr",
		{
			TextureFilter::Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			false }
	);

	mPbrProbe = std::make_unique<PbrProbe>(panoramaSky);

	mPbrDeferred->setProbe(mPbrProbe.get());
	mPbrForward->setProbe(mPbrProbe.get());
}

void NeXEngine::initRenderBackend()
{
	mWindow->activate();
	auto* backend = RenderBackend::get();
	backend->setViewPort(0, 0, mVideo.width, mVideo.height);
	backend->setMSAASamples(mVideo.msaaSamples);
	backend->init();
}


void NeXEngine::readConfig()
{
	LOG(mLogger, nex::Info) << "Loading configuration file...";
	if (!mConfig.load(mConfigFileName))
	{
		LOG(mLogger, nex::Warning) << "Configuration file couldn't be read. Default values are used.";
	}
	else
	{
		LOG(mLogger, nex::Info) << "Configuration file loaded.";
	}

	try
	{
		mSystemLogLevel = nex::stringToLogLevel(mSystemLogLevelStr);
	}
	catch (const EnumFormatException& e)
	{

		//log error and set default log level
		LOG(mLogger, nex::Error) << e.what();

		LOG(mLogger, nex::Warning) << "Couldn't get log level from " << mSystemLogLevelStr << std::endl
			<< "Log level is set now to 'Warning'" << std::endl;

		mSystemLogLevel = nex::Warning;
		mSystemLogLevelStr = "Warning";
	}

	nex::LoggerManager::get()->setMinLogLevel(mSystemLogLevel);
	mConfig.write(mConfigFileName);
}


void NeXEngine::setupCallbacks()
{
	Input* input = mWindow->getInputDevice();

	//auto focusCallback = bind(&PBR_Deferred_Renderer::onWindowsFocus, this, placeholders::_1, placeholders::_2);
	//auto scrollCallback = std::bind(&Camera::onScroll, m_camera.get(), std::placeholders::_1, std::placeholders::_2);

	input->addWindowFocusCallback([=](Window* window_s, bool receivedFocus)
	{
		setRunning(receivedFocus);
		if (receivedFocus)
		{
			LOG(mLogger, nex::Debug) << "received focus!";
			//isRunning = true;
		}
		else
		{
			LOG(mLogger, nex::Debug) << "lost focus!";
			//isRunning = false;
			if (window_s->isInFullscreenMode())
			{
				window_s->minimize();
			}
		}
	});
	//input->addScrollCallback(scrollCallback);

	input->addFrameBufferResizeCallback([=](unsigned width, unsigned height)
	{
		LOG(mLogger, nex::Debug) << "addFrameBufferResizeCallback : width: " << width << ", height: " << height;

		if (!mWindow->hasFocus()) {
			LOG(mLogger, nex::Debug) << "addFrameBufferResizeCallback : no focus!";
		}

		if (width == 0 || height == 0) {
			LOG(mLogger, nex::Warning) << "addFrameBufferResizeCallback : width or height is 0!";
			return;
		}

		mCamera->setAspectRatio((float)width / (float)height);

		mRenderer->updateRenderTargets(width, height);
	});
}

void NeXEngine::setupGUI()
{
	using namespace nex::gui;

	nex::gui::AppStyle style;
	style.apply();

	std::unique_ptr<SceneGUI> root = std::make_unique<SceneGUI>(mControllerSM.get());
	std::unique_ptr<nex::gui::ConfigurationWindow> configurationWindow = std::make_unique<nex::gui::ConfigurationWindow>(root->getMainMenuBar(), root->getOptionMenu());

	gui::Tab* graphicsTechniques = configurationWindow->getGraphicsTechniquesTab();
	gui::Tab* cameraTab = configurationWindow->getCameraTab();
	gui::Tab* videoTab = configurationWindow->getVideoTab();
	gui::Tab* generalTab = configurationWindow->getGeneralTab();


	auto csmView = std::make_unique<nex::CascadedShadow_ConfigurationView>(mCascadedShadow.get());
	graphicsTechniques->addChild(std::move(csmView));

	auto hbaoView = std::make_unique<nex::HbaoConfigurationView>(mRenderer->getAOSelector()->getHBAO());
	graphicsTechniques->addChild(std::move(hbaoView));

	auto ssaoView = std::make_unique<SSAO_ConfigurationView>(mRenderer->getAOSelector()->getSSAO());
	graphicsTechniques->addChild(std::move(ssaoView));

	auto pbrView = std::make_unique<Pbr_ConfigurationView>(mPbrDeferred.get());
	graphicsTechniques->addChild(std::move(pbrView));

	auto cameraView = std::make_unique<FPCamera_ConfigurationView>(static_cast<FPCamera*>(mCamera.get()));
	cameraTab->addChild(move(cameraView));


	auto windowView = std::make_unique<Window_ConfigurationView>(mWindow);
	videoTab->addChild(move(windowView));

	auto textureManagerView = std::make_unique<TextureManager_Configuration>(TextureManager::get());
	generalTab->addChild(move(textureManagerView));

	auto pbr_deferred_rendererView = std::make_unique<PBR_Deferred_Renderer_ConfigurationView>(mRenderer.get());
	generalTab->addChild(move(pbr_deferred_rendererView));

	configurationWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(configurationWindow));

	mControllerSM->getCurrentController()->setDrawable(move(root));
}

void NeXEngine::setupCamera()
{
	if (TrackballQuatCamera* casted = dynamic_cast<TrackballQuatCamera*>(mCamera.get()))
	{
		auto cameraResizeCallback = std::bind(&TrackballQuatCamera::updateOnResize, casted, std::placeholders::_1, std::placeholders::_2);
		casted->updateOnResize(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
		mInput->addFrameBufferResizeCallback(cameraResizeCallback);
	}

	int windowWidth = mWindow->getFrameBufferWidth();
	int windowHeight = mWindow->getFrameBufferHeight();

	mCamera->setPosition(glm::vec3(0.0f, 3.0f, 2.0f));
	mCamera->setLook(glm::vec3(0.0f, 0.0f, -1.0f));
	mCamera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
	mCamera->setAspectRatio((float)windowWidth / (float)windowHeight);


	mCamera->setNearPlane(0.1f);
	mCamera->setFarPlane(150.0f);
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

		ss << mBaseTitle << " : FPS = " << fps;
		mWindow->setTitle(ss.str());
		ss.str("");
		runtime = 0;
	}
}