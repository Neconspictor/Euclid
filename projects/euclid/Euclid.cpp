#include <Euclid.hpp>
#include <PBR_Deferred_Renderer.hpp>
#include <nex/platform/glfw/SubSystemProviderGLFW.hpp>
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
#include "nex/post_processing/AmbientOcclusion.hpp"
#include <nex/GI/Probe.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/scene/Scene.hpp>
#include <glm/gtc/matrix_transform.inl>
#include "nex/mesh/MeshFactory.hpp"
#include <nex/GI/GlobalIllumination.hpp>
#include "nex/resource/ResourceLoader.hpp"
#include <memory>
#include <nex/gui/ParticleSystemGenerator.hpp>
#include <nex/gui/vob/VobEditor.hpp>
#include <nex/gui/vob/VobLoader.hpp>
#include <nex/gui/TextureViewer.hpp>
#include <nex/gui/ProbeGeneratorView.hpp>
#include <nex/GI/ProbeGenerator.hpp>
#include <nex/cluster/Cluster.hpp>
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
#include <nex/effects/Flame.hpp>
#include <nex/particle/Particle.hpp>
#include <nex/math/BoundingBox.hpp>
#include <memory>
#include <nex/gui/VisualizationSphere.hpp>
#include <nex/water/PSSR.hpp>
#include <nex/gui/Picker.hpp>
#include <nex/gui/ImGUI.hpp>
#include <gui/FontManager.hpp>
#include <nex/gui/vob/VobViewMapper.hpp>

using namespace nex;


Euclid::Euclid(SubSystemProvider* provider) :
	mLogger("NeX-Engine"),
	mWindowSystem(provider),
	mWindow(nullptr),
	mInput(nullptr),
	mIsRunning(false),
	mConfigFileName("config.ini"),
	mSystemLogLevel(nex::Debug)
{
	mConfig.addOption("Logging", "logLevel", &mSystemLogLevelStr, std::string(""));
	mConfig.addOption("General", "rootDirectory", (std::string*)nullptr, std::string("./"));
	mConfig.addOption("Input", "language", &mKeyMapLanguageStr, std::string("US"));
}

Euclid::~Euclid()
{
	mWindowSystem = nullptr;

	mRenderer.reset();

	ResourceLoader::shutdown();

	mScene.acquireLock();
	mScene.clearUnsafe();
}

nex::LogLevel Euclid::getLogLevel() const
{
	return mSystemLogLevel;
}

void Euclid::init()
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
	desc.language = mKeyMapLanguage;
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
	TextureManager::get()->init(mGlobals.getTextureDirectory(), 
		mGlobals.getCompiledTextureDirectory(), 
		mGlobals.getCompiledTextureFileExtension(),
		mGlobals.getMetaFileExtension());

	ResourceLoader::init(secondWindow, *this);
	ResourceLoader::get()->resetJobCounter();

	//init pbr 
	initPbr();

	AnimationManager::init(
		mGlobals.getAnimationDirectory(),
		mGlobals.getCompiledAnimationDirectory(), 
		mGlobals.getCompiledAnimationFileExtension(),
		mGlobals.getCompiledRiggedMeshFileExtension(),
		mGlobals.getCompiledRigFileExtension(),
		mGlobals.getMetaFileExtension());

	// init static mesh manager
	MeshManager::init(mGlobals.getMeshDirectory(),
		mGlobals.getCompiledMeshDirectory(),
		mGlobals.getCompiledMeshFileExtension());
}

void nex::Euclid::initScene()
{
	auto* commandQueue = RenderEngine::getCommandQueue();

	// init effect libary
	RenderBackend::get()->initEffectLibrary();
	mFlameShader = std::make_unique<FlameShader>();
	mParticleShader = std::make_unique<ParticleShader>();

	mGlobalIllumination = std::make_unique<GlobalIllumination>(1024, 100, 5, true);
	auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();
	voxelConeTracer->setUseConetracing(false);

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


	auto* gui = nex::gui::ImGUI_Impl::get();
	gui->init(mWindow, mGlobals.getFontDirectory());

	mWindow->activate();

	mCursor = std::make_unique<Cursor>(StandardCursorType::Hand);
	mWindow->setCursor(mCursor.get());


	mRenderer->init(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());

	mPSSR = std::make_unique<PSSR>(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());

	initLights();

	mPicker = std::make_unique<nex::gui::Picker>();
	mFontManager = std::make_unique<nex::gui::FontManager>(nex::gui::ImGUI_Impl::get());

	mControllerSM = std::make_unique<gui::EngineController>(mWindow,
		mInput,
		mRenderer.get(),
		mCamera.get(),
		mPicker.get(),
		&mScene,
		gui,
		mFontManager.get());

	mControllerSM->activate();

	setupCamera();
	setupCallbacks();
	setupGUI();

	mInput->addWindowCloseCallback([](Window* window)
		{
			void* nativeWindow = window->getNativeWindow();
			boxer::Selection selection = boxer::show("Do you really want to quit?", "Exit Euclid", boxer::Style::Warning, boxer::Buttons::OKCancel, nativeWindow);
			if (selection == boxer::Selection::Cancel)
			{
				window->reopen();
			}
		});

	ProbeFactory::init(mGlobals.getCompiledPbrDirectory(), mGlobals.getCompiledPbrFileExtension());


	mBoneTrafoBuffer = std::make_unique<ShaderStorageBuffer>(Shader::DEFAULT_BONE_BUFFER_BINDING_POINT, 
		sizeof(glm::mat4) * 100, nullptr, GpuBuffer::UsageHint::DYNAMIC_COPY);

	auto future = ResourceLoader::get()->enqueue([=]()->nex::Resource* {
		createScene(RenderEngine::getCommandQueue());
		return nullptr;
		});

	//std::this_thread::sleep_for(std::chrono::seconds(5));

	ResourceLoader::get()->waitTillAllJobsFinished();

	auto& exceptionQueue = ResourceLoader::get()->getExceptionQueue();

	while (!commandQueue->empty()) {
		auto task = commandQueue->pop();
		task();
	}

	while (!exceptionQueue.empty()) {
		auto& exception = exceptionQueue.pop();
		throw_with_trace(*exception);
	}

	//mRenderer->getPbrTechnique()->getActive()->getCascadedShadow()->enable(false);

	auto* probeManager = mGlobalIllumination->getProbeManager();
	bool bakeProbes = true;

	{
		auto* factory = probeManager->getFactory();

		TextureDesc backgroundHDRData;
		backgroundHDRData.internalFormat = InternalFormat::RGB32F;
		auto* backgroundHDR = TextureManager::get()->getImage("hdr/HDR_040_Field.hdr", true, backgroundHDRData, true);
		auto* defaultIrradianceProbe = probeManager->createUninitializedProbeVob(Probe::Type::Irradiance, glm::vec3(0, 1, 1), backgroundHDR, 0);
		auto* defaultReflectionProbe = probeManager->createUninitializedProbeVob(Probe::Type::Reflection, glm::vec3(1, 1, 1), backgroundHDR, 0);
		auto lock = mScene.acquireLock();
		mScene.addActiveVobUnsafe(defaultIrradianceProbe);
		mScene.addActiveVobUnsafe(defaultReflectionProbe);
	}

	{
		auto* factory = probeManager->getFactory();

		TextureDesc backgroundHDRData;
		backgroundHDRData.internalFormat = InternalFormat::RGB32F;
		auto* backgroundHDR = TextureManager::get()->getImage("hdr/HDR_Free_City_Night_Lights_Ref.hdr", true, backgroundHDRData, true);
		auto* defaultIrradianceProbe = probeManager->createUninitializedProbeVob(Probe::Type::Irradiance, glm::vec3(0, 2, 1), backgroundHDR, 1);
		auto* defaultReflectionProbe = probeManager->createUninitializedProbeVob(Probe::Type::Reflection, glm::vec3(1, 2, 1), backgroundHDR, 1);
		auto lock = mScene.acquireLock();
		mScene.addActiveVobUnsafe(defaultIrradianceProbe);
		mScene.addActiveVobUnsafe(defaultReflectionProbe);
	}

	

	{
		auto* factory = probeManager->getFactory();

		TextureDesc backgroundHDRData;
		backgroundHDRData.internalFormat = InternalFormat::RGB32F;
		auto* backgroundHDR = TextureManager::get()->getImage("hdr/newport_loft.hdr", true, backgroundHDRData, true);
		auto* defaultIrradianceProbe = probeManager->createUninitializedProbeVob(Probe::Type::Irradiance, glm::vec3(0, 3, 1), backgroundHDR, 2);
		auto* defaultReflectionProbe = probeManager->createUninitializedProbeVob(Probe::Type::Reflection, glm::vec3(1, 3, 1), backgroundHDR, 2);
		auto lock = mScene.acquireLock();
		mScene.addActiveVobUnsafe(defaultIrradianceProbe);
		mScene.addActiveVobUnsafe(defaultReflectionProbe);
	}

	{
		auto* factory = probeManager->getFactory();

		TextureDesc backgroundHDRData;
		backgroundHDRData.internalFormat = InternalFormat::RGB32F;
		auto* backgroundHDR = TextureManager::get()->getImage("hdr/grace_cathedral.hdr", true, backgroundHDRData, true);
		auto* defaultIrradianceProbe = probeManager->createUninitializedProbeVob(Probe::Type::Irradiance, glm::vec3(0, 4, 1), backgroundHDR, 3);
		auto* defaultReflectionProbe = probeManager->createUninitializedProbeVob(Probe::Type::Reflection, glm::vec3(1, 4, 1), backgroundHDR, 3);
		auto lock = mScene.acquireLock();
		mScene.addActiveVobUnsafe(defaultIrradianceProbe);
		mScene.addActiveVobUnsafe(defaultReflectionProbe);
	}


	if (bakeProbes) {
		auto* probeBaker = mGlobalIllumination->getProbeBaker();
		auto* factory = probeManager->getFactory();
		probeBaker->bakeProbes(mScene, mSun, *factory, mRenderer.get());
	}

	mRenderer->updateRenderTargets(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
	mProbeClusterView->setDepth(mRenderer->getGbuffer()->getDepthAttachment()->texture.get());
	//mRenderer->getPbrTechnique()->getActive()->getCascadedShadow()->enable(true);
}

bool Euclid::isRunning() const
{
	return mIsRunning;
}

void Euclid::run()
{
	mIsRunning = mWindow->hasFocus();
	mWindow->activate();

	auto* backend = RenderBackend::get();
	auto* screenSprite = backend->getScreenSprite();
	auto* lib = backend->getEffectLibrary();
	auto* postProcessor = lib->getPostProcessor();
	auto* taa = postProcessor->getTAA();
	auto* commandQueue = RenderEngine::getCommandQueue();
	auto* gui = nex::gui::ImGUI_Impl::get();
	auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();

	const auto invViewProj = inverse(mCamera->getProjectionMatrix() * mCamera->getView());

	RenderContext context;
	context.camera = mCamera.get();
	context.proj = &mCamera->getProjectionMatrix();
	context.view = &mCamera->getView();
	context.csm = mCascadedShadow.get();
	context.gi = mGlobalIllumination.get();
	context.lib = lib;
	context.sun = &mSun;
	context.stencilTest = backend->getStencilTest();
	context.invViewProj = &invViewProj;
	context.irradianceAmbientReflection = mRenderer->getActiveIrradianceAmbientReflectionRT();
	context.out = mRenderer->getOutRT();
	context.outStencilView = mRenderer->getOutStencilView();
	context.pingPong = mRenderer->getPingPongRT();
	context.pingPongStencilView = mRenderer->getPingPongStencilView();
	context.time = 0.0f;
	context.frameTime = 0.0f;
	context.windowWidth = mWindow->getFrameBufferWidth();
	context.windowHeight = mWindow->getFrameBufferHeight();

	ResourceLoader::get()->waitTillAllJobsFinished();

	auto& exceptionQueue = ResourceLoader::get()->getExceptionQueue();

	mRenderCommandQueue.useCameraCulling(mCamera.get());

	voxelConeTracer->activate(false);

	if (voxelConeTracer->isActive()){
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

		mGiShadowMap->update(mSun, box);
		mGiShadowMap->render(mRenderCommandQueue.getShadowCommands());
		//mRenderer->renderShadows(mRenderCommandQueue.getShadowCommands(), context, mSun, nullptr);

		voxelConeTracer->deferVoxelizationLighting(true);

		if (voxelConeTracer->isVoxelLightingDeferred())
		{
			voxelConeTracer->voxelize(collection, box, nullptr, nullptr);
			voxelConeTracer->updateVoxelTexture(&mSun, mGiShadowMap.get());
		}
		else {
			voxelConeTracer->voxelize(collection, box, &mSun, mGiShadowMap.get());
			voxelConeTracer->updateVoxelTexture(nullptr, nullptr);
		}

		mCamera->setPosition(originalPosition, true);
		mCamera->update();
	}

	mRenderCommandQueue.clear();
	mScene.collectRenderCommands(mRenderCommandQueue, false, mBoneTrafoBuffer.get());
	mRenderCommandQueue.sort();

	auto currentSunDir = mSun.directionWorld;

	mTimer.reset();
	mTimer.pause(!isRunning());



	while (mWindow->isOpen())
	{
		// Poll input events before checking if the app is running, otherwise 
		// the window is likely to hang or crash (at least on windows platform)
		mWindowSystem->pollEvents();

		mTimer.update();

		float frameTime = mTimer.getTimeDiffInSeconds();
		
		float fps = mCounter.update(frameTime);

		updateWindowTitle(frameTime, fps);

		while (!commandQueue->empty()) {
			auto task = commandQueue->pop();
			task();
		}

		while (!exceptionQueue.empty()) {
			auto& exception = exceptionQueue.pop();
			throw *exception;
		}

		

		if (isRunning())
		{
			const auto width = mWindow->getFrameBufferWidth();
			const auto height = mWindow->getFrameBufferHeight();
			const auto widenedWidth = width;
			const auto widenedHeight = height;
			const auto offsetX = 0;// (widenedWidth - width) / 2;
			const auto offsetY = 0;// (widenedHeight - height) / 2;
			const auto invViewProj = inverse(mCamera->getProjectionMatrix() * mCamera->getView());


			context.invViewProj = &invViewProj;
			context.irradianceAmbientReflection = mRenderer->getActiveIrradianceAmbientReflectionRT();

			context.out = mRenderer->getOutRT();
			context.outStencilView = mRenderer->getOutStencilView();

			context.pingPong = mRenderer->getPingPongRT();
			context.pingPongStencilView = mRenderer->getPingPongStencilView();

			context.time = mTimer.getCountedTimeInSeconds();
			context.frameTime = frameTime;

			context.windowWidth = widenedWidth;
			context.windowHeight = widenedHeight;
			
			{
				mScene.acquireLock();

				mScene.frameUpdate(context);
				mScene.updateWorldTrafoHierarchyUnsafe(false);
				mScene.calcSceneBoundingBoxUnsafe();

				mRenderCommandQueue.clear();
				mScene.collectRenderCommands(mRenderCommandQueue, false, mBoneTrafoBuffer.get());

				mRenderCommandQueue.sort();
				mScene.setHasChangedUnsafe(false);
			}

			mControllerSM->frameUpdate(frameTime);

			auto* activeController = mControllerSM->getActiveController();
			gui->newFrame(frameTime, activeController->allowsInputForUI());

			//update jitter for next frame
			//taa->advanceJitter();
			//mCamera->setJitter(taa->getJitterMatrix());
			//mCamera->setJitterVec(taa->getJitterVec());
			
			mCamera->update();

			//commandQueue->useSphereCulling(mCamera->getPosition(), 10.0f);
	

			{
				//mScene.acquireLock();
				//mGlobalIllumination->update(mScene.getActiveProbeVobsUnsafe());
			}

			
			auto* screenRT = backend->getDefaultRenderTarget();
			Texture* texture = nullptr;
			SpriteShader* spriteShader = nullptr;

			screenSprite->setWidth(width);
			screenSprite->setHeight(height);
			screenSprite->setPosition({ offsetX, offsetY });

			

			if (voxelConeTracer->getVisualize()) {

				static auto* depthTest = RenderBackend::get()->getDepthBuffer();
				depthTest->enableDepthBufferWriting(true);
				auto* tempRT = mRenderer->getOutRT();

				tempRT->bind();
				backend->setViewPort(0, 0, widenedWidth, widenedHeight);
				backend->setBackgroundColor(glm::vec3(1.0f));
				tempRT->clear(Color | Stencil | Depth);
				
				voxelConeTracer->renderVoxels(mCamera->getProjectionMatrix(), mCamera->getView());

				texture = tempRT->getColorAttachmentTexture(0);
			}
			else
			{
				mRenderer->render(mRenderCommandQueue, context, true);

				const auto& renderLayer = mRenderer->getRenderLayers()[mRenderer->getActiveRenderLayer()];
				texture = renderLayer.textureProvider();
				spriteShader = renderLayer.spriteShaderProvider();
			}
			
			//texture = mRenderer->getGbuffer()->getNormal();

			if (texture != nullptr) {
				screenRT->bind();
				backend->setViewPort(offsetX, offsetY, mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
				//backend->setBackgroundColor(glm::vec3(1.0f));
				//screenRT->clear(Color | Stencil | Depth);

				screenSprite->setTexture(texture);
				screenSprite->render(spriteShader);
			}

			
			if (voxelConeTracer->isVoxelLightingDeferred() && voxelConeTracer->isActive())
			{
				auto diffSun = currentSunDir - mSun.directionWorld;
				if (mSun._pad[0] != 0.0) {
					mSun._pad[0] = 0.0;

					mGiShadowMap->update(mSun, mScene.getSceneBoundingBox());
					mGiShadowMap->render(mRenderCommandQueue.getShadowCommands());

					voxelConeTracer->updateVoxelTexture(&mSun, mGiShadowMap.get());
				}
					
			}

			currentSunDir = mSun.directionWorld;

			mControllerSM->getDrawable()->drawGUI();

			ImGui::Render();

			


			gui->renderDrawData(ImGui::GetDrawData());
			
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


void Euclid::setConfigFileName(const char*  fileName)
{
	mConfigFileName = fileName;
}

void Euclid::setRunning(bool isRunning)
{
	mIsRunning = isRunning;
}


void Euclid::createScene(nex::RenderEngine::CommandQueue* commandQueue)
{
	LOG(mLogger, Info) << "create scene...";
	mScene.acquireLock();
	mScene.clearUnsafe();

	mMeshes.clear();


	nex::SamplerDesc flameStructureTexDesc;
	flameStructureTexDesc.wrapS = flameStructureTexDesc.wrapT = flameStructureTexDesc.wrapR
		= UVTechnique::Repeat;
	FlameMaterialLoader flameMaterialLoader(mFlameShader.get(),
		TextureManager::get()->getImage("misc/Flame4.psd"),
		flameStructureTexDesc,
		1.0f * glm::vec4(1.0f, 0.5f, 0.1f, 1.0f));


	auto* deferred = mPbrTechnique->getDeferred();
	auto* forward = mPbrTechnique->getForward();

	//TODO

	PbrMaterialLoader solidMaterialLoader(deferred->getGeometryShaderProvider(), TextureManager::get());
	PbrMaterialLoader solidBoneAlphaStencilMaterialLoader(deferred->getGeometryBonesShaderProvider(), TextureManager::get(),
		PbrMaterialLoader::LoadMode::SOLID_ALPHA_STENCIL);

	PbrMaterialLoader alphaTransparencyMaterialLoader(forward->getShaderProvider(), TextureManager::get(),
		PbrMaterialLoader::LoadMode::ALPHA_TRANSPARENCY);


	//cerberus
	if (true) {
		auto group = MeshManager::get()->loadModel("cerberus/Cerberus.obj", solidMaterialLoader);

		commandQueue->push([groupPtr = group.get()]() {
			groupPtr->finalize();
		});

		//meshContainer->getIsLoadedStatus().get()->finalize();
		auto* cerberus = mScene.createVobUnsafe(group->getBatches());
		cerberus->getName() = "cerberus";
		cerberus->setPositionLocalToParent(glm::vec3(0.0f, 0.0f, 0.0f));

		mMeshes.emplace_back(std::move(group));
	}


	
	// sponza
	
	if (false) {
		auto group = MeshManager::get()->loadModel("sponza/sponzaSimple7.obj", solidMaterialLoader);

		commandQueue->push([groupPtr = group.get()]() {
			groupPtr->finalize();
		});

		//meshContainer->getIsLoadedStatus().get()->finalize();
		auto* sponzaVob = mScene.createVobUnsafe(group->getBatches());
		sponzaVob->getName() = "sponzaSimple1";
		sponzaVob->setPositionLocalToParent(glm::vec3(0.0f, -2.0f, 0.0f));

		mMeshes.emplace_back(std::move(group));
	}
	



	//bone animations
	Vob* bobVobPtr = nullptr;
	if (false) {
		nex::SkinnedMeshLoader meshLoader;
		auto* fileSystem = nex::AnimationManager::get()->getRiggedMeshFileSystem();
		auto group = nex::MeshManager::get()->loadModel("bob/boblampclean.md5mesh",
			solidBoneAlphaStencilMaterialLoader,
			1.0f,
			&meshLoader, fileSystem);


		commandQueue->push([groupPtr = group.get()]() {
			groupPtr->finalize();
		});

		//auto* rig4 = nex::AnimationManager::get()->getRig(*bobModel);

		auto* ani = nex::AnimationManager::get()->loadBoneAnimation("bob/boblampclean.md5anim");

		auto bobVob = std::make_unique<RiggedVob>(nullptr);
		bobVobPtr = bobVob.get();
		bobVob->setBatches(group->getBatches());
		bobVob->setActiveAnimation(ani);

		//bobVob->setDefaultScale(0.03f);

		bobVob->setTrafoMeshToLocal(glm::scale(glm::mat4(1.0f), glm::vec3(0.03f)));

		bobVob->setPositionLocalToParent(glm::vec3(0, 0.0f, 0.0f));
		//bobVob->setPosition(glm::vec3(-5.5f, 6.0f, 0.0f));
		//bobVob->setScaleLocal(glm::vec3(0.03f));

		bobVob->setRotationLocalToParent(glm::vec3(glm::radians(-90.0f), glm::radians(90.0f), 0.0f));
		mScene.addVobUnsafe(std::move(bobVob));
		mMeshes.emplace_back(std::move(group));
	}
	





	if (false) {
		//meshContainer = MeshManager::get()->getModel("transparent/transparent.obj");
		auto group = MeshManager::get()->loadModel("transparent/transparent_intersected_resolved.obj",
			alphaTransparencyMaterialLoader);
		commandQueue->push([groupPtr = group.get()]() {
			groupPtr->finalize();
		});

		auto transparentVob3 = std::make_unique<Vob>();
		transparentVob3->setBatches(group->getBatches());
		transparentVob3->getName() = "transparent - 3";
		transparentVob3->setPositionLocalToParent(glm::vec3(-12.0f, 2.0f, 0.0f));
		mMeshes.emplace_back(std::move(group));

		if (bobVobPtr) bobVobPtr->addChild(transparentVob3.get());

		mScene.addVobUnsafe(std::move(transparentVob3));



		// flame test
		group = nex::MeshManager::get()->loadModel("misc/plane_simple.obj",
			flameMaterialLoader);

		commandQueue->push([groupPtr = group.get()]() {
			groupPtr->finalize();
		});

		auto flameVob = std::make_unique<Billboard>(nullptr);
		flameVob->setBatches(group->getBatches());
		flameVob->setPositionLocalToParent(glm::vec3(1.0, 0.246f, 3 + 0.056f));
		flameVob->setRotationLocalToParent(glm::vec3(glm::radians(0.0f), glm::radians(-90.0f), glm::radians(0.0f)));
		mScene.addVobUnsafe(std::move(flameVob));
		mMeshes.emplace_back(std::move(group));
	}

	


	if (false) {
		// particle system
		AABB boundingBox = { glm::vec3(-0.3f, 0.0f, -0.3f), glm::vec3(0.3f, 1.0f, 0.3f) };

		auto shaderProvider = std::make_shared<ShaderProvider>(mParticleShader.get());
		auto particleMaterial = std::make_unique<ParticleShader::Material>(shaderProvider);
		ParticleRenderer::createParticleMaterial(particleMaterial.get());
		particleMaterial->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		particleMaterial->texture = TextureManager::get()->getImage("particle/fire.png");

		auto particleSystem = std::make_unique<VarianceParticleSystem>(
			4.0f, //averageLifeTime
			1.0f, //averageScale
			0.4f, //averageSpeed
			boundingBox, //boundingBox
			0.0f, //gravityInfluence
			std::move(particleMaterial), //material
			20000, //maxParticles
			glm::vec3(1.0f, 0.0f, 0.0f), //position
			280.0f, //pps
			0.0f, //rotation
			false, //randomizeRotation
			false //doSorting
			);

		particleSystem->setDirection(glm::vec3(0, 1, 0), PI / 16.0f);

		//particleSystem.setScaleVariance(0.015f);
		//particleSystem.setSpeedVariance(0.025f);
		//particleSystem.setLifeVariance(0.0125f);
		mScene.addVobUnsafe(std::move(particleSystem));
	}


	//ocean
	if (false) {
		auto oceanVob = std::make_unique<OceanVob>();
		oceanVob->setPositionLocalToParent(glm::vec3(-10.0f, 3.0f, -10.0f));

		commandQueue->push([oceanVobPtr = oceanVob.get(), this]() {
			//ocean
			auto ocean = std::make_unique<OceanGPU>(
				128, //N
				128, // maxWaveLength
				5.0f, //dimension
				0.4f, //spectrumScale
				glm::vec2(0.0f, 1.0f), //windDirection
				12, //windSpeed
				1000.0f, //periodTime
				glm::uvec2(12, 6), // tileCount
				mCascadedShadow.get(),
				mPSSR.get()
				);

			ocean->init();

			oceanVobPtr->setOcean(std::move(ocean));
			oceanVobPtr->updateTrafo(true, true);
			oceanVobPtr->updateWorldTrafoHierarchy(true);
		});


		mScene.addVobUnsafe(std::move(oceanVob));
	}
	

	 //probes
	const int rows = 0;
	const int columns = 0;
	const int depths = 2;
	const float rowMultiplicator = 11.0f;
	const float columnMultiplicator = 11.0f;
	const float depthMultiplicator = 7.0f;
	const float depthOffset = 7.0f;

	auto* probeManager = mGlobalIllumination->getProbeManager();

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			for (int k = 0; k < depths; ++k) {
				//if (i == 0 && j == 0 && k == 0) continue;
				auto position = glm::vec3((i - rows / 2) * rowMultiplicator,
					(k - depths / 2) * depthMultiplicator + depthOffset,
					(j - columns / 2) * columnMultiplicator);

				position += glm::vec3(-15.0f, 1.0f, 0.0f);

				//(i * rows + j)*columns + k
				auto* probeVob = probeManager->addUninitProbeUnsafe(Probe::Type::Irradiance, 
					position, 
					std::nullopt, 
					probeManager->getNextStoreID());
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

Window* Euclid::createWindow()
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
	desc.language = mKeyMapLanguage;

	return mWindowSystem->createWindow(desc);
}

void Euclid::initLights()
{
	mSun.color = glm::vec3(1.0f, 1.0f, 1.0f);
	mSun.power = 3.0f;
	mSun.directionWorld = SphericalCoordinate::cartesian({ 1.598f, 6.555f, 1.0f }); //1.1f
	//mSun.directionWorld = SphericalCoordinate::cartesian({ 2.9f,0.515f, 1.0f}); //1.1f
	//mSun.directionWorld = normalize(glm::vec3( -0.5, -1,-0.5 ));
}

void Euclid::initPbr()
{
	mPbrTechnique = std::make_unique<PbrTechnique>(nullptr, nullptr, &mSun);
}
void Euclid::initRenderBackend()
{
	mWindow->activate();
	auto* backend = RenderBackend::get();
	Rectangle viewport = { 0,0, int(mVideo.width), int(mVideo.height) };
	backend->init(viewport, mVideo.msaaSamples);
}


void Euclid::readConfig()
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


	try
	{
		mKeyMapLanguage = nex::toKeyMapLanguage(mKeyMapLanguageStr);
	}
	catch (const EnumFormatException & e)
	{
		LOG(mLogger, nex::Warning) << "No valid keymap language : " << mKeyMapLanguageStr << std::endl
			<< "US keymap is used" << std::endl;

		mKeyMapLanguage = KeyMapLanguage::US;
		mKeyMapLanguageStr = "US";
	}


	nex::LoggerManager::get()->setMinLogLevel(mSystemLogLevel);
	mConfig.write(mConfigFileName);
}


void Euclid::setupCallbacks()
{
	Input* input = mWindow->getInputDevice();

	//auto focusCallback = bind(&PBR_Deferred_Renderer::onWindowsFocus, this, placeholders::_1, placeholders::_2);
	//auto scrollCallback = std::bind(&Camera::onScroll, mCamera.get(), std::placeholders::_1, std::placeholders::_2);

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

		mControllerSM->onWindowsResize(width, height);


		mPSSR->resize(width, height);
	});
}

void Euclid::setupGUI()
{
	using namespace nex::gui;


	nex::gui::VobViewMapper::init(mGlobalIllumination->getProbeManager());

	mFontManager->setGlobalFontScale(1.0f);

	/*
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\Ubuntu\\Ubuntu-Regular.ttf", 14);
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\Segoe UI\\segoeui.ttf", 16);

	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\Roboto\\Roboto-Regular.ttf", 13);
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\ProggyClean\\ProggyClean.ttf", 24);
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\Source_Sans_Pro\\SourceSansPro-Regular.ttf", 16);
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\Open_Sans\\OpenSans-SemiBold.ttf", 16);
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\soloist\\soloistacad.ttf", 16);
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\conthrax\\conthrax-sb.ttf", 13);
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\nasalization\\nasalization-rg.ttf", 16);
	io.Fonts->AddFontFromFileTTF("C:\\Development\\Repositories\\Euclid\\_work\\data\\fonts\\ethnocentric\\ethnocentric rg.ttf", 13);
	*/




	mVisualizationSphere = std::make_unique<VisualizationSphere>(&mScene);

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

	auto pbrView = std::make_unique<Pbr_ConfigurationView>(mPbrTechnique.get(), &mSun);
	graphicsTechniques->addChild(std::move(pbrView));

	auto cameraView = std::make_unique<FPCamera_ConfigurationView>(static_cast<FPCamera*>(mCamera.get()));
	cameraTab->addChild(std::move(cameraView));


	auto windowView = std::make_unique<Window_ConfigurationView>(mWindow);
	videoTab->addChild(move(windowView));

	auto textureManagerView = std::make_unique<TextureManager_Configuration>(TextureManager::get());
	generalTab->addChild(move(textureManagerView));

	auto fontManagerView = std::make_unique<FontManager_View>(mFontManager.get());
	generalTab->addChild(move(fontManagerView));

	

	configurationWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(configurationWindow));

	auto voxelConeTracerViewWindow = std::make_unique<nex::gui::VoxelConeTracerView>(
		"Voxel based cone tracing",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		mGlobalIllumination->getVoxelConeTracer(),
		&mSun,
		mGiShadowMap.get(),
		&mRenderCommandQueue,
		&mScene);

	voxelConeTracerViewWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(voxelConeTracerViewWindow));


	auto particleSystemGeneratorWindow = std::make_unique<nex::gui::MenuWindow>(
		"Particle System Generator",
		root->getMainMenuBar(),
		root->getToolsMenu());
	particleSystemGeneratorWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	auto particleSystemGenerator = std::make_unique<nex::gui::ParticleSystemGenerator>(&mScene, 
		mVisualizationSphere.get(), 
		mCamera.get(),
		mWindow,
		mParticleShader.get());

	particleSystemGeneratorWindow->addChild(std::move(particleSystemGenerator));
	root->addChild(move(particleSystemGeneratorWindow));


	mProbeClusterView = std::make_unique<nex::gui::ProbeClusterView>(
		"Probe Cluster",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		mGlobalIllumination->getProbeManager()->getProbeCluster(),
		mCamera.get(),
		mWindow,
		mRenderer.get(),
		&mScene);
	mProbeClusterView->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(mProbeClusterView.get());

	mProbeGenerator = std::make_unique<ProbeGenerator>(&mScene, mVisualizationSphere.get(), mGlobalIllumination.get(), mRenderer.get());

	auto probeGeneratorView = std::make_unique<nex::gui::ProbeGeneratorView>(
		"Probe Generator",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		mProbeGenerator.get(),
		mCamera.get(),
		&mSun);
	probeGeneratorView->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(probeGeneratorView));


	auto textureViewerWindow = std::make_unique<nex::gui::MenuWindow>(
		"Texture Viewer",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		nex::gui::MenuWindow::COMMON_FLAGS);

	auto textureViewer = std::make_unique<TextureViewer>(glm::vec2(256), "Select Texture", mWindow);
	auto& textureView = textureViewer->getTextureView();
	textureView.useNearestNeighborFiltering();
	textureView.showAllOptions(true);


	textureViewerWindow->addChild(std::move(textureViewer));
	textureViewerWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(textureViewerWindow));


	auto vobEditorWindow = std::make_unique<nex::gui::MenuWindow>(
		"Vob Editor",
		root->getMainMenuBar(),
		root->getToolsMenu(), nex::gui::MenuWindow::DEFAULT_FLAGS); //ImGuiWindowFlags_NoCollapse

	//vobEditorWindow->setExplicitContentSize(ImVec2(300,300));

	vobEditorWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	auto* vobEditor = mControllerSM->getSceneGUI()->getVobEditor();

	vobEditorWindow->addChild(vobEditor);
	root->addChild(move(vobEditorWindow));

	
	auto vobLoaderWindow = std::make_unique<nex::gui::VobLoader>(
		"Vob Loader",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		&mScene,
		&mMeshes,
		mPbrTechnique.get(),
		mWindow,
		mCamera.get());
	vobLoaderWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(vobLoaderWindow));


	auto imguiDemoWindow = std::make_unique<nex::gui::MenuDrawable>(
		"ImGui Demo",
		root->getMainMenuBar(),
		root->getToolsMenu(), 
		true,
		[](const ImVec2& menuBarPos, float menuBarHeight, bool& isVisible) {
			ImGui::ShowDemoWindow(&isVisible);
		});

	root->addChild(move(imguiDemoWindow));

	auto imguiStyleEditorWindow = std::make_unique<nex::gui::MenuDrawable>(
		"ImGui Style Editor",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		true,
		[](const ImVec2& menuBarPos, float menuBarHeight, bool& isVisible) {
			ImGui::Begin("Style Editor", &isVisible); 
				ImGui::ShowStyleEditor(); 
			ImGui::End();
		});

	root->addChild(move(imguiStyleEditorWindow));

}

void Euclid::setupCamera()
{
	int windowWidth = mWindow->getFrameBufferWidth();
	int windowHeight = mWindow->getFrameBufferHeight();

	//mCamera->setPosition(glm::vec3(-22.0f, 13.0f, 22.0f), true);
	//mCamera->setPosition(glm::vec3(0.267f, 3.077, 1.306), true);
	//auto look = glm::vec3(-3.888f, 2.112, 0.094f) - glm::vec3(-0.267f, 3.077, 1.306);

	//auto cameraPos = glm::vec3(10.556f, 4.409f, 1.651f);
	auto cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
	mCamera->setPosition(cameraPos, true);
	auto look = glm::vec3(-0.0f, 0.0f, 0.0f) - cameraPos;

	
	
	//mCamera->setPosition(glm::vec3(-31.912f, 25.110f, 52.563), true);
	//look = glm::vec3(-3.888f, 2.112, 0.094f) - glm::vec3(-31.912f, 25.110f, 52.563);

	mCamera->setLook(normalize(look));
	mCamera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
	mCamera->setDimension(windowWidth, windowHeight);


	mCamera->setNearDistance(0.1f);
	mCamera->setFarDistance(150.0f);
}

void Euclid::updateWindowTitle(float frameTime, float fps)
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