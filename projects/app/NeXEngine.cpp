#include <NeXEngine.hpp>
#include <techniques/PBR_Deferred_Renderer.hpp>
#include <nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp>
#include <glm/glm.hpp>
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
#include "nex/renderer/RenderBackend.hpp"
#include "nex/pbr/Pbr.hpp"
#include "nex/post_processing/HBAO.hpp"
#include "nex/post_processing/SSAO.hpp"
#include "nex/post_processing/AmbientOcclusion.hpp"
#include "nex/pbr/PbrProbe.hpp"
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/Scene.hpp>
#include <glm/gtc/matrix_transform.inl>
#include "nex/mesh/MeshFactory.hpp"
#include <nex/pbr/GlobalIllumination.hpp>
#include "nex/resource/ResourceLoader.hpp"
#include <nex/pbr/PbrProbe.hpp>
#include <memory>

using namespace nex;


NeXEngine::NeXEngine(SubSystemProvider* provider) :
	mLogger("NeX-Engine"),
	mWindowSystem(provider),
	mWindow(nullptr),
	mInput(nullptr),
	mIsRunning(false),
	mConfigFileName("config.ini"),
	mSystemLogLevel(nex::Debug)
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
	
	mGlobals.init(Configuration::getGlobalConfiguration());
	LOG(mLogger, nex::Info) << "root Directory = " << mGlobals.getRootDirectory();


	mWindow = createWindow();

	Window::WindowStruct desc;
	desc.shared = mWindow;
	desc.title = mVideo.windowTitle;
	desc.fullscreen = mVideo.fullscreen;
	desc.colorBitDepth = mVideo.colorBitDepth;
	desc.refreshRate = mVideo.refreshRate;
	desc.posX = 0;
	desc.posY = 0;

	// Note that framebuffer width and height are inferred!
	desc.virtualScreenWidth = mVideo.width;
	desc.virtualScreenHeight = mVideo.height;
	desc.visible = false;
	desc.vSync = mVideo.vSync;
	auto* secondWindow = mWindowSystem->createWindow(desc);

	mWindow->activate();


	mWindow->activate();
	mWindow->setVisible(true);
	mWindow->setVsync(mVideo.vSync);


	mInput = mWindow->getInputDevice();
	mCamera = std::make_unique<FPCamera>(FPCamera(mWindow->getVirtualScreenWidth(), mWindow->getVirtualScreenHeight()));
	mBaseTitle = mWindow->getTitle();


	// init ImageFactory
	ImageFactory::init(true);

	// init shader file system
	mShaderFileSystem = std::make_unique<FileSystem>(std::vector<std::filesystem::path>{ mGlobals.getOpenGLShaderDirectory()}, "", "");
	ShaderSourceFileGenerator::get()->init(mShaderFileSystem.get());

	//init render backend
	initRenderBackend();

	// init texture manager
	TextureManager::get()->init(mGlobals.getTextureDirectory(), mGlobals.getCompiledTextureDirectory(), mGlobals.getCompiledTextureFileExtension());


	// init effect libary
	RenderBackend::get()->initEffectLibrary();

	ResourceLoader::init(secondWindow, *this);
	ResourceLoader::get()->resetJobCounter();

	initLights();

	//init pbr 
	initPbr();

	// init static mesh manager
	StaticMeshManager::get()->init(mGlobals.getMeshDirectory(),
		mGlobals.getCompiledMeshDirectory(),
		mGlobals.getCompiledMeshFileExtension(),
		std::make_unique<PbrMaterialLoader>(mPbrTechnique.get(), TextureManager::get()));


	mRenderer = std::make_unique<PBR_Deferred_Renderer>(RenderBackend::get(), 
		mPbrTechnique.get(),
		mCascadedShadow.get(), 
		mWindow->getInputDevice());


	mGui = mWindowSystem->createGUI(mWindow);
	
	

	mControllerSM = std::make_unique<gui::EngineController>(mWindow,
		mInput,
		mRenderer.get(),
		mCamera.get(),
		&mScene,
		mGui.get());

	mWindow->activate();

	mCursor = std::make_unique<Cursor>(StandardCursorType::Hand);
	mWindow->setCursor(mCursor.get());


	mRenderer->init(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
	mControllerSM->activate();
	setupCamera();
	setupCallbacks();
	setupGUI();

	mInput->addWindowCloseCallback([](Window* window)
	{
		void* nativeWindow = window->getNativeWindow();
		boxer::Selection selection = boxer::show("Do you really want to quit?", "Exit NeX", boxer::Style::Warning, boxer::Buttons::OKCancel, nativeWindow);
		if (selection == boxer::Selection::Cancel)
		{
			window->reopen();
		}
	});

	PbrProbeFactory::init(mGlobals.getCompiledPbrDirectory(), mGlobals.getCompiledPbrFileExtension());
	
	auto future = ResourceLoader::get()->enqueue([=](nex::RenderEngine::CommandQueue* commandQueue)->nex::Resource* {
		createScene(commandQueue);
		return nullptr;
	});

	//std::this_thread::sleep_for(std::chrono::seconds(5));

	ResourceLoader::get()->waitTillAllJobsFinished();

	auto& exceptionQueue = ResourceLoader::get()->getExceptionQueue();

	while (!mCommandQueue->empty()) {
		auto task = mCommandQueue->pop();
		task();
	}

	while (!exceptionQueue.empty()) {
		auto& exception = exceptionQueue.pop();
		throw *exception;
	}

	//mRenderer->getPbrTechnique()->getActive()->getCascadedShadow()->enable(false);
	mGlobalIllumination->bakeProbes(mScene, mRenderer.get());
	mRenderer->updateRenderTargets(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
	//mRenderer->getPbrTechnique()->getActive()->getCascadedShadow()->enable(true);
}

bool NeXEngine::isRunning() const
{
	return mIsRunning;
}

void NeXEngine::run()
{
	mIsRunning = true;
	mWindow->activate();


	ResourceLoader::get()->waitTillAllJobsFinished();

	auto& exceptionQueue = ResourceLoader::get()->getExceptionQueue();

	SimpleTimer timer;

	RenderCommandQueue commandQueue;
	commandQueue.useCameraCulling(mCamera.get());

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

		while (!mCommandQueue->empty()) {
			auto task = mCommandQueue->pop();
			task();
		}

		while (!exceptionQueue.empty()) {
			auto& exception = exceptionQueue.pop();
			throw *exception;
		}

		{
			mScene.acquireLock();
			mScene.updateWorldTrafoHierarchyUnsafe(true);
		}

		if (isRunning())
		{
			mGui->newFrame(frameTime);
			mCamera->update();
			mControllerSM->frameUpdate(frameTime);

			//commandQueue->useSphereCulling(mCamera->getPosition(), 10.0f);
			commandQueue.clear();
			collectRenderCommands(commandQueue, mScene);
			commandQueue.sort();

			{
				mScene.acquireLock();
				mGlobalIllumination->update(mScene.getActiveProbeVobsUnsafe());
			}

			
			RenderTarget2D* screenRenderTarget = RenderBackend::get()->getDefaultRenderTarget();
			mRenderer->render(commandQueue, *mCamera, mSun, mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight(), true, screenRenderTarget);
			mControllerSM->getDrawable()->drawGUI();
			
			ImGui::Render();
			mGui->renderDrawData(ImGui::GetDrawData());
			
			// present rendered frame
			mWindow->swapBuffers();
		}
		else
		{
			mWindowSystem->waitForEvents();
			//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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

void NeXEngine::collectRenderCommands(RenderCommandQueue& commandQueue, const Scene& scene)
{
	//std::queue<SceneNode*> queue;
	RenderCommand command;
	std::list<SceneNode*> queue;


	scene.acquireLock();
	for (const auto& root : scene.getActiveVobsUnsafe())
	{
		queue.push_back(root->getMeshRootNode());

		while (!queue.empty())
		{
			auto* node = queue.front();
			queue.pop_front();

			auto range = node->getChildren();

			for (auto* node : range)
			{
				queue.push_back(node);
			}

			auto* mesh = node->getMesh();
			if (mesh != nullptr)
			{
				command.mesh = mesh;
				command.material = node->getMaterial();
				command.worldTrafo = node->getWorldTrafo();
				command.prevWorldTrafo = node->getPrevWorldTrafo();
				command.boundingBox = node->getMeshBoundingBoxWorld();
				commandQueue.push(command, true);
			}
		}
	}
}


void NeXEngine::createScene(nex::RenderEngine::CommandQueue* commandQueue)
{
	mScene.acquireLock();
	mScene.clearUnsafe();

	auto* meshContainer = StaticMeshManager::get()->getModel("misc/textured_plane.obj");
	commandQueue->push([=]() {
		meshContainer->finalize();
	});


	//meshContainer->getIsLoadedStatus().get()->finalize();
	
	auto* ground = meshContainer->createNodeHierarchyUnsafe(&mScene);
	auto* groundVob = mScene.createVobUnsafe(ground);
	groundVob->setSelectable(true);
	groundVob->mDebugName = "ground";

	meshContainer = StaticMeshManager::get()->getModel("sponza/sponzaTest5.obj");

	commandQueue->push([=]() {
		meshContainer->finalize();
	});

	//meshContainer->getIsLoadedStatus().get()->finalize();
	auto* sponzaNode = meshContainer->createNodeHierarchyUnsafe(&mScene);
	auto* sponzaVob = mScene.createVobUnsafe(sponzaNode);
	sponzaVob->mDebugName = "sponza";

	//meshContainer = StaticMeshManager::get()->getModel("transparent/transparent.obj");
	meshContainer = StaticMeshManager::get()->getModel("transparent/transparent_intersected_resolved.obj");
	commandQueue->push([=]() {
		meshContainer->finalize();
	});
	
	//meshContainer->getIsLoadedStatus().get()->finalize();
	//auto* transparent = meshContainer->createNodeHierarchyUnsafe(&mScene);
	//auto* transparentVob = mScene.createVobUnsafe(transparent);
	//transparentVob->mDebugName = "transparent - 1";
	//auto* transparentVob2 = mScene.createVobUnsafe(meshContainer->createNodeHierarchyUnsafe(&mScene));
	//transparentVob2->mDebugName = "transparent - 2";
	auto* transparentVob3 = mScene.createVobUnsafe(meshContainer->createNodeHierarchyUnsafe(&mScene));
	transparentVob3->mDebugName = "transparent - 3";

	//(*(transparentVob->getMeshRootNode()->getChildren().begin()))->getMaterial()->getRenderState().doCullFaces = false;
	//(*(transparentVob->getMeshRootNode()->getChildren().begin()))->getMaterial()->getRenderState().doShadowCast = false;
	//(*(transparentVob2->getMeshRootNode()->getChildren().begin()))->getMaterial()->getRenderState().doCullFaces = false;
	//(*(transparentVob2->getMeshRootNode()->getChildren().begin()))->getMaterial()->getRenderState().doShadowCast = false;

	auto& childs = transparentVob3->getMeshRootNode()->getChildren();

	for (int i = 0; i < childs.size(); ++i) {
		childs[i]->getMesh()->mDebugName = "Intersected " + std::to_string(i);
		childs[i]->getMaterial()->getRenderState().doCullFaces = false;
		childs[i]->getMaterial()->getRenderState().doShadowCast = false;
	}




	//transparentVob->setPosition(glm::vec3(-2.0f, 2.0f, 0.0f));
	//transparentVob2->setPosition(glm::vec3(-3.0f, 2.0f, 0.0f));
	transparentVob3->setPosition(glm::vec3(-4.0f, 2.0f, 0.0f));

	const int rows = 7;
	const int columns = 7;
	const int depths = 4;
	const float rowMultiplicator = 11.0f;
	const float columnMultiplicator = 11.0f;
	const float depthMultiplicator = 7.0f;
	const float depthOffset = 7.0f;

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			for (int k = 0; k < depths; ++k) {
				//if (i == 0 && j == 0 && k == 0) continue;
				auto position = glm::vec3((i - rows / 2) * rowMultiplicator,
					(k - depths / 2) * depthMultiplicator + depthOffset,
					(j - columns / 2) * columnMultiplicator);

				auto* probeVob = mGlobalIllumination->addUninitProbeUnsafe(mScene, position, (i * rows + j)*columns + k);
				mScene.addActiveVobUnsafe(probeVob);
			}
		}
	}

	/*meshContainer = StaticMeshManager::get()->getModel("cerberus/cerberus.obj");
	ResourceLoader::get()->enqueue([=] {
		return meshContainer;
	});
	auto* cerberus = mScene.createVobUnsafe(meshContainer->createNodeHierarchyUnsafe(&mScene));
	cerberus->setPosition(glm::vec3(-9.0f, 2.0f, 0.0f));*/

	//ground->setPositionLocal({ 10, 0, 0 });

	const glm::mat4 unit(1.0f);
	//auto translate = unit;
	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0, 5.0f, 0.0f));
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));
	//auto scale = glm::mat4();
	glm::mat4 scale = glm::scale(unit, glm::vec3(10, 10, 10));

	glm::mat4 trafo = translateMatrix * rotation * scale;
	//cerberus->setLocalTrafo(trafo);

	mScene.updateWorldTrafoHierarchyUnsafe(true);
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
	desc.visible = false;
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
	mGlobalIllumination = std::make_unique<GlobalIllumination>(mGlobals.getCompiledPbrDirectory(), 128, 200);
	
	CascadedShadow::PCFFilter pcf;
	pcf.sampleCountX = 2;
	pcf.sampleCountY = 2;
	pcf.useLerpFiltering = true;
	mCascadedShadow = std::make_unique<CascadedShadow>(2048, 2048, 4, pcf, 6.0f, true);


	mPbrTechnique = std::make_unique<PbrTechnique>(mGlobalIllumination.get(), mCascadedShadow.get(), &mSun);
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

		mCamera->setDimension(width, height);

		mRenderer->updateRenderTargets(width, height);
	});
}

void NeXEngine::setupGUI()
{
	using namespace nex::gui;

	nex::gui::AppStyle style;
	style.apply();

	auto root = mControllerSM->getSceneGUI();
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

	auto pbrView = std::make_unique<Pbr_ConfigurationView>(mPbrTechnique.get());
	graphicsTechniques->addChild(std::move(pbrView));

	auto cameraView = std::make_unique<FPCamera_ConfigurationView>(static_cast<FPCamera*>(mCamera.get()));
	cameraTab->addChild(std::move(cameraView));


	auto windowView = std::make_unique<Window_ConfigurationView>(mWindow);
	videoTab->addChild(move(windowView));

	auto textureManagerView = std::make_unique<TextureManager_Configuration>(TextureManager::get());
	generalTab->addChild(move(textureManagerView));

	auto pbr_deferred_rendererView = std::make_unique<PBR_Deferred_Renderer_ConfigurationView>(mRenderer.get());
	generalTab->addChild(move(pbr_deferred_rendererView));

	auto sceneNodeProperty = std::make_unique<SceneNodeProperty>(mWindow);
	sceneNodeProperty->setPicker(mControllerSM->getEditMode()->getPicker());
	sceneNodeProperty->setScene(&mScene);

	generalTab->addChild(std::move(sceneNodeProperty));

	configurationWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(configurationWindow));
}

void NeXEngine::setupCamera()
{
	int windowWidth = mWindow->getFrameBufferWidth();
	int windowHeight = mWindow->getFrameBufferHeight();

	mCamera->setPosition(glm::vec3(-22.0f, 13.0f, 22.0f), true);
	mCamera->setLook(glm::vec3(0.0f, 0.0f, -1.0f));
	mCamera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
	mCamera->setDimension(windowWidth, windowHeight);


	mCamera->setNearDistance(0.1f);
	mCamera->setFarDistance(150.0f);
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