#include <NeXEngine.hpp>
#include <PBR_Deferred_Renderer.hpp>
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
#include <nex/mesh/MeshManager.hpp>
#include "nex/shader_generator/ShaderSourceFileGenerator.hpp"
#include "nex/renderer/RenderBackend.hpp"
#include "nex/pbr/Pbr.hpp"
#include "nex/post_processing/HBAO.hpp"
#include "nex/post_processing/SSAO.hpp"
#include "nex/post_processing/AmbientOcclusion.hpp"
#include "nex/pbr/PbrProbe.hpp"
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/scene/Scene.hpp>
#include <glm/gtc/matrix_transform.inl>
#include "nex/mesh/MeshFactory.hpp"
#include <nex/pbr/GlobalIllumination.hpp>
#include "nex/resource/ResourceLoader.hpp"
#include <nex/pbr/PbrProbe.hpp>
#include <memory>
#include <gui/NodeEditor.hpp>
#include <gui/VobLoader.hpp>
#include <gui/TextureViewer.hpp>
#include <gui/ProbeGeneratorView.hpp>
#include <nex/pbr/ProbeGenerator.hpp>
#include <nex/pbr/Cluster.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/post_processing/PostProcessor.hpp>
#include <nex/post_processing/TAA.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include <nex/shadow/ShadowMap.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <nex\material\PbrMaterialLoader.hpp>
#include <nex/pbr/Pbr.hpp>
#include <nex/pbr/PbrDeferred.hpp>
#include <nex/pbr/PbrForward.hpp>
#include <nex/pbr/PbrPass.hpp>

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
	ResourceLoader::shutdown();
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
	mCamera = std::make_unique<FPCamera>(FPCamera(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight()));
	mBaseTitle = mWindow->getTitle();


	// init ImageFactory
	ImageFactory::init(true);

	// init shader file system
	mShaderFileSystem = std::make_unique<FileSystem>(std::vector<std::filesystem::path>{ mGlobals.getOpenGLShaderDirectory(),
		mGlobals.getInterfaceShaderDirectory()}, "", "");

	ShaderSourceFileGenerator::get()->init(mShaderFileSystem.get());

	//init render backend
	initRenderBackend();

	// init texture manager
	TextureManager::get()->init(mGlobals.getTextureDirectory(), mGlobals.getCompiledTextureDirectory(), mGlobals.getCompiledTextureFileExtension());

	ResourceLoader::init(secondWindow, *this);
	ResourceLoader::get()->resetJobCounter();

	//init pbr 
	initPbr();

	AnimationManager::init(
		mGlobals.getAnimationDirectory(),
		mGlobals.getCompiledAnimationDirectory(), 
		mGlobals.getCompiledAnimationFileExtension(),
		mGlobals.getCompiledRiggedMeshFileExtension(),
		mGlobals.getCompiledRigFileExtension());

	// init static mesh manager
	MeshManager::init(mGlobals.getMeshDirectory(),
		mGlobals.getCompiledMeshDirectory(),
		mGlobals.getCompiledMeshFileExtension());
}

void nex::NeXEngine::initScene()
{
	// init effect libary
	RenderBackend::get()->initEffectLibrary();

	mGlobalIllumination = std::make_unique<GlobalIllumination>(mGlobals.getCompiledPbrDirectory(), 1024, 10, true);

	PCFFilter pcf;
	pcf.sampleCountX = 2;
	pcf.sampleCountY = 2;
	pcf.useLerpFiltering = true;

	const auto width = 2048;
	const auto height = 2048;
	const auto cascades = 4;
	const auto biasMultiplier = 6.0f;
	const auto antiFlicker = true;

	mCascadedShadow = std::make_unique<CascadedShadow>(width, height, cascades, pcf, biasMultiplier, antiFlicker);
	mGiShadowMap = std::make_unique<ShadowMap>(2048, 2048, pcf, 0.0f);


	mPbrTechnique->setGI(mGlobalIllumination.get());
	mPbrTechnique->setShadow(mCascadedShadow.get());
	mPbrTechnique->updateShaders();

	mRenderer = std::make_unique<PBR_Deferred_Renderer>(RenderBackend::get(),
		mPbrTechnique.get(),
		mCascadedShadow.get(),
		mWindow->getInputDevice());


	mGui = mWindowSystem->createGUI(mWindow);

	mWindow->activate();

	mCursor = std::make_unique<Cursor>(StandardCursorType::Hand);
	mWindow->setCursor(mCursor.get());


	mRenderer->init(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());



	initLights();

	mRenderer->getOcean()->simulate(0.0f);

	mControllerSM = std::make_unique<gui::EngineController>(mWindow,
		mInput,
		mRenderer.get(),
		mCamera.get(),
		&mScene,
		mGui.get());

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


	mBoneTrafoBuffer = std::make_unique<ShaderStorageBuffer>(Shader::DEFAULT_BONE_BUFFER_BINDING_POINT, 
		sizeof(glm::mat4) * 100, nullptr, GpuBuffer::UsageHint::DYNAMIC_COPY);

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
		throw_with_trace(*exception);
	}

	//mRenderer->getPbrTechnique()->getActive()->getCascadedShadow()->enable(false);

	bool bakeProbes = false;
	if (bakeProbes)
		mGlobalIllumination->bakeProbes(mScene, mRenderer.get());
	mRenderer->updateRenderTargets(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
	mProbeClusterView->setDepth(mRenderer->getGbuffer()->getDepthAttachment()->texture.get());
	//mRenderer->getPbrTechnique()->getActive()->getCascadedShadow()->enable(true);
}

bool NeXEngine::isRunning() const
{
	return mIsRunning;
}

void NeXEngine::run()
{
	mIsRunning = mWindow->hasFocus();
	mWindow->activate();


	ResourceLoader::get()->waitTillAllJobsFinished();

	auto& exceptionQueue = ResourceLoader::get()->getExceptionQueue();

	mRenderCommandQueue.useCameraCulling(mCamera.get());

	{
		mScene.acquireLock();
		mScene.updateWorldTrafoHierarchyUnsafe(true);
		mScene.calcSceneBoundingBoxUnsafe();
		auto box = mScene.getSceneBoundingBox();
		auto middlePoint = (box.max + box.min) / 2.0f;

		auto originalPosition = mCamera->getPosition();
		mCamera->setPosition(middlePoint, true);
		mCamera->update();

		mScene.collectRenderCommands(mRenderCommandQueue, false, mBoneTrafoBuffer.get());
		auto collection = mRenderCommandQueue.getCommands(RenderCommandQueue::Deferrable | RenderCommandQueue::Forward
			| RenderCommandQueue::Transparent);

		nex::Shader::Constants constants;
		constants.camera = mCamera.get();
		mGiShadowMap->update(mSun, box);
		mGiShadowMap->render(mRenderCommandQueue.getShadowCommands());
		//mRenderer->renderShadows(mRenderCommandQueue.getShadowCommands(), constants, mSun, nullptr);

		mGlobalIllumination->deferVoxelizationLighting(true);

		if (mGlobalIllumination->isVoxelLightingDeferred()) 
		{
			mGlobalIllumination->voxelize(collection, box, nullptr, nullptr);
			mGlobalIllumination->updateVoxelTexture(&mSun, mGiShadowMap.get());
		}
		else {
			mGlobalIllumination->voxelize(collection, box, &mSun, mGiShadowMap.get());
			mGlobalIllumination->updateVoxelTexture(nullptr, nullptr);
		}

		mCamera->setPosition(originalPosition, true);
		mCamera->update();
	}

	mRenderCommandQueue.clear();
	mScene.collectRenderCommands(mRenderCommandQueue, false, mBoneTrafoBuffer.get());
	mRenderCommandQueue.sort();

	auto* backend = RenderBackend::get();
	auto* screenSprite = backend->getScreenSprite();
	auto* lib = backend->getEffectLibrary();
	auto* postProcessor = lib->getPostProcessor();
	auto* taa = postProcessor->getTAA();

	auto currentSunDir = mSun.directionWorld;

	mTimer.reset();
	mTimer.pause(!isRunning());

	while (mWindow->isOpen())
	{
		// Poll input events before checking if the app is running, otherwise 
		// the window is likely to hang or crash (at least on windows platform)
		mWindowSystem->pollEvents();

		mTimer.update();
		

		if (!isRunning()) {
			LOG(mLogger, Info) << "Counted time = " << mTimer.getCountedTimeInSeconds();
		}

		float frameTime = mTimer.getTimeDiffInSeconds();
		
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

		

		if (isRunning())
		{

			if (true || mScene.hasChangedUnsafe()) {
				mScene.acquireLock();
				mScene.updateWorldTrafoHierarchyUnsafe(false);
				mScene.calcSceneBoundingBoxUnsafe();

				mRenderCommandQueue.clear();
				mScene.collectRenderCommands(mRenderCommandQueue, false, mBoneTrafoBuffer.get());
				mRenderCommandQueue.sort();
				mScene.setHasChangedUnsafe(false);
			}

			mGui->newFrame(frameTime);

			//update jitter for next frame
			//taa->advanceJitter();
			//mCamera->setJitter(taa->getJitterMatrix());
			//mCamera->setJitterVec(taa->getJitterVec());
			mControllerSM->frameUpdate(frameTime);
			mCamera->update();

			static float simulationTime = 0.0f;
			static int animate = 0;
			simulationTime += frameTime;

			//if (animate == 0) {
				mRenderer->getOcean()->simulate(simulationTime);
				mRenderer->getOcean()->updateAnimationTime(simulationTime);
			//	++animate;
			//}
			//else {
			//	++animate;
			//	animate = animate % 10;
			//}

			//commandQueue->useSphereCulling(mCamera->getPosition(), 10.0f);
	

			{
				//mScene.acquireLock();
				//mGlobalIllumination->update(mScene.getActiveProbeVobsUnsafe());
			}

			
			auto* screenRT = backend->getDefaultRenderTarget();
			Texture* texture = nullptr;
			SpritePass* spritePass = nullptr;

			
			const auto width = mWindow->getFrameBufferWidth();
			const auto height = mWindow->getFrameBufferHeight();
			const auto widenedWidth = width;
			const auto widenedHeight = height;
			const auto offsetX = 0;// (widenedWidth - width) / 2;
			const auto offsetY = 0;// (widenedHeight - height) / 2;

			screenSprite->setWidth(width);
			screenSprite->setHeight(height);
			screenSprite->setPosition({ offsetX, offsetY });

			

			if (mGlobalIllumination->getVisualize()) {

				static auto* depthTest = RenderBackend::get()->getDepthBuffer();
				depthTest->enableDepthBufferWriting(true);
				auto* tempRT = mRenderer->getOutRendertTarget();

				tempRT->bind();
				backend->setViewPort(0, 0, widenedWidth, widenedHeight);
				backend->setBackgroundColor(glm::vec3(1.0f));
				tempRT->clear(Color | Stencil | Depth);
				
				mGlobalIllumination->renderVoxels(mCamera->getProjectionMatrix(), mCamera->getView());

				texture = tempRT->getColorAttachmentTexture(0);
			}
			else
			{
				
				
				auto& updateables = mScene.getActiveFrameUpdateables();
				for (auto* updateable : updateables) {
					updateable->frameUpdate(frameTime);
				}
				

				Shader::Constants constants;
				constants.camera = mCamera.get();
				constants.time = mTimer.getCountedTimeInSeconds();
				constants.windowWidth = widenedWidth;
				constants.windowHeight = widenedHeight;
				constants.sun = &mSun;

				mRenderer->render(mRenderCommandQueue, constants, true);

				const auto& renderLayer = mRenderer->getRenderLayers()[mRenderer->getActiveRenderLayer()];
				texture = renderLayer.textureProvider();
				spritePass = renderLayer.pass;
			}
			
			//texture = mRenderer->getGbuffer()->getNormal();

			if (texture != nullptr) {
				screenRT->bind();
				backend->setViewPort(offsetX, offsetY, mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
				//backend->setBackgroundColor(glm::vec3(1.0f));
				//screenRT->clear(Color | Stencil | Depth);

				screenSprite->setTexture(texture);
				screenSprite->render(spritePass);
			}

			
			if (mGlobalIllumination->isVoxelLightingDeferred())
			{
				auto diffSun = currentSunDir - mSun.directionWorld;
				if (mSun._pad[0] != 0.0) {
					mSun._pad[0] = 0.0;

					mGiShadowMap->update(mSun, mScene.getSceneBoundingBox());
					mGiShadowMap->render(mRenderCommandQueue.getShadowCommands());

					mGlobalIllumination->updateVoxelTexture(&mSun, mGiShadowMap.get());
				}
					
			}

			currentSunDir = mSun.directionWorld;

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


void NeXEngine::createScene(nex::RenderEngine::CommandQueue* commandQueue)
{
	LOG(mLogger, Info) << "create scene...";
	mScene.acquireLock();
	mScene.clearUnsafe();

	mMeshes.clear();


	auto* deferred = mPbrTechnique->getDeferred();
	auto* forward = mPbrTechnique->getForward();

	//TODO
	PbrMaterialLoader solidMaterialLoader(deferred->getGeometryShader(), TextureManager::get());
	PbrMaterialLoader solidBoneAlphaStencilMaterialLoader(deferred->getGeometryBonesShader(), TextureManager::get(), 
		PbrMaterialLoader::LoadMode::SOLID_ALPHA_STENCIL);

	PbrMaterialLoader alphaTransparencyMaterialLoader(forward->getPass(), TextureManager::get(),
		PbrMaterialLoader::LoadMode::ALPHA_TRANSPARENCY);



	
	// scene nodes (sponza, transparent)
	auto group = MeshManager::get()->loadModel("sponza/sponzaSimple7.obj", solidMaterialLoader);

	commandQueue->push([groupPtr = group.get()]() {
		groupPtr->finalize();
		});

	//meshContainer->getIsLoadedStatus().get()->finalize();
	auto* sponzaNode = group->createNodeHierarchyUnsafe();
	auto* sponzaVob = mScene.createVobUnsafe(sponzaNode);
	sponzaVob->mDebugName = "sponzaSimple1";
	sponzaVob->setPosition(glm::vec3(0.0f, -2.0f, 0.0f));

	mMeshes.emplace_back(std::move(group));

	//meshContainer = MeshManager::get()->getModel("transparent/transparent.obj");
	group = MeshManager::get()->loadModel("transparent/transparent_intersected_resolved.obj",
													alphaTransparencyMaterialLoader);
	commandQueue->push([groupPtr = group.get()]() {
		groupPtr->finalize();
	});
	
	auto* transparentVob3 = mScene.createVobUnsafe(group->createNodeHierarchyUnsafe());
	transparentVob3->mDebugName = "transparent - 3";

	auto& childs = transparentVob3->getMeshRootNode()->getChildren();

	/*for (int i = 0; i < childs.size(); ++i) {
		auto* batch = childs[i]->getBatch();

		for (auto& pair : batch->getMeshes()) {
			auto* mesh = pair.first;
			auto* material = pair.second;

			mesh->mDebugName = "Intersected " + std::to_string(i);
			material->getRenderState().doCullFaces = false;
			material->getRenderState().doDepthTest = true;
			material->getRenderState().doDepthWrite = true;
			material->getRenderState().doShadowCast = false;
		}
	}*/

	transparentVob3->setPosition(glm::vec3(-4.0f, 2.0f, 0.0f));
	mMeshes.emplace_back(std::move(group));


	//bone animations
	nex::SkinnedMeshLoader meshLoader;
	auto* fileSystem = nex::AnimationManager::get()->getRiggedMeshFileSystem();
	group = nex::MeshManager::get()->loadModel("bob/boblampclean.md5mesh",
		solidBoneAlphaStencilMaterialLoader,
		&meshLoader, fileSystem);


	commandQueue->push([groupPtr = group.get()]() {
		groupPtr->finalize();
		});

	//auto* rig4 = nex::AnimationManager::get()->getRig(*bobModel);

	auto* ani = nex::AnimationManager::get()->loadBoneAnimation("bob/boblampclean.md5anim");

	auto bobVob = std::make_unique<RiggedVob>(group->createNodeHierarchyUnsafe());
	bobVob->setActiveAnimation(ani);
	bobVob->setPosition(glm::vec3(0, 0.0f, 0.0f));
	//bobVob->setPosition(glm::vec3(-5.5f, 6.0f, 0.0f));
	bobVob->setScale(glm::vec3(0.03));
	bobVob->setOrientation(glm::vec3(glm::radians(-90.0f), glm::radians(90.0f), 0.0f));
	mScene.addVobUnsafe(std::move(bobVob));
	mMeshes.emplace_back(std::move(group));


	 //probes
	const int rows = 1;
	const int columns = 1;
	const int depths = 2;
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

				position += glm::vec3(-15.0f, 1.0f, 0.0f);

				//(i * rows + j)*columns + k
				auto* probeVob = mGlobalIllumination->addUninitProbeUnsafe(position, mGlobalIllumination->getNextStoreID());
				mScene.addActiveVobUnsafe(probeVob);
			}
		}
	}

	const glm::mat4 unit(1.0f);
	//auto translate = unit;
	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2.0, 5.0f, 0.0f));
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));
	//auto scale = glm::mat4();
	glm::mat4 scale = glm::scale(unit, glm::vec3(10, 10, 10));

	glm::mat4 trafo = translateMatrix * rotation * scale;
	//cerberus->setLocalTrafo(trafo);

	mScene.updateWorldTrafoHierarchyUnsafe(true);

	LOG(mLogger, Info) << "scene created.";
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
	mSun.color = glm::vec3(1.0f, 1.0f, 1.0f);
	mSun.power = 3.0f;
	mSun.directionWorld = SphericalCoordinate::cartesian({ 2.9f,0.515f, 1.0f}); //1.1f
	//mSun.directionWorld = normalize(glm::vec3( -0.5, -1,-0.5 ));
}

void NeXEngine::initPbr()
{
	mPbrTechnique = std::make_unique<PbrTechnique>(nullptr, nullptr, &mSun);
}
void NeXEngine::initRenderBackend()
{
	mWindow->activate();
	auto* backend = RenderBackend::get();
	Viewport viewport = { 0,0, int(mVideo.width), int(mVideo.height) };
	backend->init(viewport, mVideo.msaaSamples);
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
			// 
			mTimer.pause(false);
		}
		else
		{
			LOG(mLogger, nex::Debug) << "lost focus!";
			//isRunning = false;
			if (window_s->isInFullscreenMode())
			{
				window_s->minimize();
			}

			mTimer.pause(true);
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

		unsigned widenedWidth = width;
		unsigned widenedHeight = height;

		mCamera->setDimension(widenedWidth, widenedHeight);

		mRenderer->updateRenderTargets(widenedWidth, widenedHeight);
		auto* depth = mRenderer->getGbuffer()->getDepthAttachment()->texture.get();
		mProbeClusterView->setDepth(depth);

		auto* taa = RenderBackend::get()->getEffectLibrary()->getPostProcessor()->getTAA();
		taa->updateJitterVectors(glm::vec2(1.0f / (float)widenedWidth, 1.0f / (float)widenedHeight));
	});
}

void NeXEngine::setupGUI()
{
	using namespace nex::gui;

	nex::gui::AppStyle style;
	style.apply();

	auto root = mControllerSM->getSceneGUI();
	std::unique_ptr<nex::gui::ConfigurationWindow> configurationWindow =  std::make_unique<nex::gui::ConfigurationWindow>(
		"Graphics and Video Settings", 
			root->getMainMenuBar(), 
			root->getOptionMenu());

	gui::Tab* graphicsTechniques = configurationWindow->getGraphicsTechniquesTab();
	gui::Tab* cameraTab = configurationWindow->getCameraTab();
	gui::Tab* videoTab = configurationWindow->getVideoTab();
	gui::Tab* generalTab = configurationWindow->getGeneralTab();


	auto csmView = std::make_unique<nex::CascadedShadow_ConfigurationView>(mCascadedShadow.get());
	graphicsTechniques->addChild(std::move(csmView));

	auto pbr_deferred_rendererView = std::make_unique<PBR_Deferred_Renderer_ConfigurationView>(mRenderer.get());
	graphicsTechniques->addChild(move(pbr_deferred_rendererView));

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

	configurationWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(configurationWindow));

	mProbeClusterView = std::make_unique<nex::gui::ProbeClusterView>(
		"Probe Cluster",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		mGlobalIllumination->getProbeCluster(),
		mCamera.get(),
		mWindow,
		mRenderer.get(),
		&mScene);
	mProbeClusterView->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(mProbeClusterView.get());

	mProbeGenerator = std::make_unique<ProbeGenerator>(&mScene, mGlobalIllumination.get(), mRenderer.get());

	auto probeGeneratorView = std::make_unique<nex::gui::ProbeGeneratorView>(
		"Probe Generator",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		mProbeGenerator.get(),
		mCamera.get());
	probeGeneratorView->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(probeGeneratorView));


	auto nodeEditorWindow = std::make_unique<nex::gui::MenuWindow>(
		"Scene Node Editor",
		root->getMainMenuBar(),
		root->getToolsMenu());
	nodeEditorWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	auto sceneNodeProperty = std::make_unique<NodeEditor>(mWindow);
	sceneNodeProperty->setPicker(mControllerSM->getEditMode()->getPicker());
	sceneNodeProperty->setScene(&mScene);
	nodeEditorWindow->addChild(std::move(sceneNodeProperty));
	root->addChild(move(nodeEditorWindow));


	auto textureViewerWindow = std::make_unique<nex::gui::TextureViewer>(
		"Texture Loader",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		mWindow);
	textureViewerWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(textureViewerWindow));


	auto vobLoaderWindow = std::make_unique<nex::gui::VobLoader>(
		"Vob Loader",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		&mScene,
		&mMeshes,
		mPbrTechnique.get(),
		mWindow);
	vobLoaderWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(vobLoaderWindow));

	auto globalIlluminationWindow = std::make_unique<nex::gui::GlobalIlluminationView>(
		"Global Illumination",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		mGlobalIllumination.get(),
		&mSun,
		mGiShadowMap.get(),
		&mRenderCommandQueue,
		&mScene);

	globalIlluminationWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(globalIlluminationWindow));

}

void NeXEngine::setupCamera()
{
	int windowWidth = mWindow->getFrameBufferWidth();
	int windowHeight = mWindow->getFrameBufferHeight();

	//mCamera->setPosition(glm::vec3(-22.0f, 13.0f, 22.0f), true);
	//mCamera->setPosition(glm::vec3(0.267f, 3.077, 1.306), true);
	//auto look = glm::vec3(-3.888f, 2.112, 0.094f) - glm::vec3(-0.267f, 3.077, 1.306);

	mCamera->setPosition(glm::vec3(3.242, 0.728, 0.320), true);
	auto look = glm::vec3(0.0f, 0.0f, 0.0f) - glm::vec3(3.242, 0.728, 0.320);

	
	
	//mCamera->setPosition(glm::vec3(-31.912f, 25.110f, 52.563), true);
	//look = glm::vec3(-3.888f, 2.112, 0.094f) - glm::vec3(-31.912f, 25.110f, 52.563);

	mCamera->setLook(normalize(look));
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