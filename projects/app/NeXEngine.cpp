#include <NeXEngine.hpp>
#include <pbr_deferred/PBR_Deferred_Renderer.hpp>
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
#include <nex/shadow/SceneNearFarComputePass.hpp>
#include <nex/sky/AtmosphericScattering.hpp>
#include <queue>
#include <nex/Scene.hpp>
#include <glm/gtc/matrix_transform.inl>
#include "nex/math/Ray.hpp"
#include "nex/mesh/MeshFactory.hpp"
#include "nex/shader/SimpleColorPass.hpp"
#include "nex/gui/Picker.hpp"

using namespace nex;


NeXEngine::NeXEngine(SubSystemProvider* provider) :
	mLogger("NeX-Engine"),
	mWindowSystem(provider),
	mWindow(nullptr),
	mInput(nullptr),
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
	
	mGlobals.init(Configuration::getGlobalConfiguration());
	LOG(mLogger, nex::Info) << "root Directory = " << mGlobals.getRootDirectory();


	mWindow = createWindow();
	mInput = mWindow->getInputDevice();
	mCamera = std::make_unique<FPCamera>(FPCamera(mWindow->getVirtualScreenWidth(), mWindow->getVirtualScreenHeight()));
	mBaseTitle = mWindow->getTitle();

	// init shader file system
	mShaderFileSystem = std::make_unique<FileSystem>(std::vector<std::filesystem::path>{ mGlobals.getOpenGLShaderDirectory()});
	ShaderSourceFileGenerator::get()->init(mShaderFileSystem.get());

	//init render backend
	initRenderBackend();

	// init texture manager
	TextureManager::get()->init(mGlobals.getTextureDirectory(), mGlobals.getCompiledTextureDirectory(), mGlobals.getCompiledTextureFileExtension());

	// init effect libary
	RenderBackend::get()->initEffectLibrary();

	initLights();

	//init pbr 
	initPbr();

	// init static mesh manager
	StaticMeshManager::get()->init(mGlobals.getMeshDirectory(),
		mGlobals.getCompiledMeshDirectory(),
		mGlobals.getCompiledMeshFileExtension(),
		std::make_unique<PbrMaterialLoader>(mPbrTechnique.get(), TextureManager::get()));

	initProbes();


	mRenderer = std::make_unique<PBR_Deferred_Renderer>(RenderBackend::get(), 
		mPbrTechnique.get(),
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

	createScene();

	mPicker = std::make_unique<gui::Picker>();

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

		auto* commandQueue = mRenderer->getCommandQueue();
		commandQueue->setCamera(mCamera.get());

		if (isRunning())
		{
			mGui->newFrame(frameTime);

			if (!mGui->isActive() || mControllerSM->getCurrentController()->isNotInterruptibleActionActive())
			{
				mControllerSM->frameUpdate(frameTime);

				if (mInput->isPressed(Input::Button::LeftMouseButton))
				{
					const auto& mouseData = mInput->getFrameMouseOffset();
					const glm::ivec2 position(mouseData.xAbsolute, mouseData.yAbsolute);
					mPicker->pick(mScene, mCamera->calcScreenRay(position));
					mPickedSceneNodeProperty->setPicker(mPicker.get());
				}

				if (mInput->isPressed(Input::KEY_L))
				{
					const auto& mouseData = mInput->getFrameMouseOffset();
					std::cout << "mouse position = " << mouseData.xAbsolute << ", " << mouseData.yAbsolute << std::endl;
				}
			}

			mCamera->update();

			commandQueue->clear();
			collectRenderCommands(commandQueue, mScene);
			commandQueue->sort();

			mRenderer->render(mCamera.get(), &mSun, frameTime, mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
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

void NeXEngine::collectRenderCommands(RenderCommandQueue* commandQueue, const Scene& scene)
{
	std::queue<SceneNode*> queue;
	RenderCommand command;


	for (const auto& root : scene.getRoots())
	{
		queue.emplace(root);

		while (!queue.empty())
		{
			auto* node = queue.back();
			queue.pop();

			auto range = node->getChildren();

			for (auto it = range.begin; it != range.end; ++it)
			{
				queue.emplace(*it);
			}

			auto* mesh = node->getMesh();
			if (mesh != nullptr)
			{
				command.mesh = mesh;
				command.material = node->getMaterial();
				command.worldTrafo = node->getWorldTrafo();
				command.prevWorldTrafo = node->getPrevWorldTrafo();
				command.boundingBox = (mCamera->getView() * command.worldTrafo) * mesh->getAABB();
				commandQueue->push(command, true);
			}
		}
	}
}


void NeXEngine::createScene()
{
	mScene.clear();

	auto* meshContainer = StaticMeshManager::get()->getModel("misc/textured_plane.obj");
	auto* ground = meshContainer->createNodeHierarchy(&mScene);

	//ground->setPositionLocal({ 10, 0, 0 });

	//meshContainer = StaticMeshManager::get()->getModel("cerberus/cerberus.obj");
	//auto* cerberus = meshContainer->createNodeHierarchy(&mScene);
	//cerberus->setPositionLocal({0, 2, 0});

	const glm::mat4 unit(1.0f);
	//auto translate = unit;
	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0, 5.0f, 0.0f));
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));
	//auto scale = glm::mat4();
	glm::mat4 scale = glm::scale(unit, glm::vec3(10, 10, 10));

	glm::mat4 trafo = translateMatrix * rotation * scale;
	//cerberus->setLocalTrafo(trafo);

	mScene.updateWorldTrafoHierarchy(true);

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


	mPbrTechnique = std::make_unique<PbrTechnique>(&mAmbientLight, mCascadedShadow.get(), &mSun, nullptr);
}

void NeXEngine::initProbes()
{
	TextureManager* textureManager = TextureManager::get();

	panoramaSky = textureManager->getImage("hdr/HDR_040_Field.hdr",
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

	PbrProbeFactory factory(mGlobals.getCompiledPbrDirectory());

	mPbrProbe = factory.create(panoramaSky, 0);


	mPbrTechnique->setProbe(mPbrProbe.get());
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

	auto sceneNodeProperty = std::make_unique<SceneNodeProperty>();
	mPickedSceneNodeProperty = sceneNodeProperty.get();
	generalTab->addChild(std::move(sceneNodeProperty));

	configurationWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(configurationWindow));

	mControllerSM->getCurrentController()->setDrawable(move(root));
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