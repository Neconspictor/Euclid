#include <Euclid.hpp>
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
#include <gui/ParticleSystemGenerator.hpp>
#include <gui/VobEditor.hpp>
#include <gui/VobLoader.hpp>
#include <nex/gui/TextureViewer.hpp>
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
#include <nex/effects/Flame.hpp>
#include <nex/particle/Particle.hpp>
#include <nex/math/BoundingBox.hpp>
#include <memory>
#include <nex/gui/VisualizationSphere.hpp>

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
	mConfig.addOption("General", "rootDirectory", &mSystemLogLevelStr, std::string("./"));
}

Euclid::~Euclid()
{
	mWindowSystem = nullptr;

	mRenderer.reset();

	ResourceLoader::shutdown();
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

	bool bakeProbes = false;
	if (bakeProbes)
		mGlobalIllumination->bakeProbes(mScene, mRenderer.get());
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

		nex::Constants constants;
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
	auto* commandQueue = RenderEngine::getCommandQueue();

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


			Constants constants;
			constants.camera = mCamera.get();
			constants.time = mTimer.getCountedTimeInSeconds();
			constants.frameTime = frameTime;
			constants.windowWidth = widenedWidth;
			constants.windowHeight = widenedHeight;
			constants.sun = &mSun;

			{
				mScene.acquireLock();

				mScene.frameUpdate(constants);
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



	
	// scene nodes (sponza, transparent)
	auto group = MeshManager::get()->loadModel("sponza/sponzaSimple7.obj", solidMaterialLoader);

	commandQueue->push([groupPtr = group.get()]() {
		groupPtr->finalize();
		});

	//meshContainer->getIsLoadedStatus().get()->finalize();
	auto* sponzaVob = mScene.createVobUnsafe(group->getBatches());
	sponzaVob->mDebugName = "sponzaSimple1";
	sponzaVob->setPosition(glm::vec3(0.0f, -2.0f, 0.0f));

	mMeshes.emplace_back(std::move(group));

	//meshContainer = MeshManager::get()->getModel("transparent/transparent.obj");
	group = MeshManager::get()->loadModel("transparent/transparent_intersected_resolved.obj",
													alphaTransparencyMaterialLoader);
	commandQueue->push([groupPtr = group.get()]() {
		groupPtr->finalize();
	});
	
	auto* transparentVob3 = mScene.createVobUnsafe(group->getBatches());
	transparentVob3->mDebugName = "transparent - 3";

	/*for (int i = 0; i < childs.size(); ++i) {
		auto* batch = childs[i]->getBatch();

		for (auto& pair : batch->getEntries()) {
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

	auto bobVob = std::make_unique<RiggedVob>(nullptr);
	bobVob->setBatches(group->getBatches());
	bobVob->setActiveAnimation(ani);
	bobVob->setPosition(glm::vec3(0, 0.0f, 0.0f));
	//bobVob->setPosition(glm::vec3(-5.5f, 6.0f, 0.0f));
	bobVob->setScale(glm::vec3(0.03f));
	bobVob->setOrientation(glm::vec3(glm::radians(-90.0f), glm::radians(90.0f), 0.0f));
	mScene.addVobUnsafe(std::move(bobVob));
	mMeshes.emplace_back(std::move(group));



	// flame test
	group = nex::MeshManager::get()->loadModel("misc/plane_simple.obj",
		flameMaterialLoader);

	commandQueue->push([groupPtr = group.get()]() {
		groupPtr->finalize();
	});

	auto flameVob = std::make_unique<Billboard>(nullptr);
	flameVob->setBatches(group->getBatches());
	flameVob->setPosition(glm::vec3(1.0, 0.246f, 3 + 0.056f));
	flameVob->setOrientation(glm::vec3(glm::radians(0.0f), glm::radians(-90.0f), glm::radians(0.0f)));
	mScene.addVobUnsafe(std::move(flameVob));
	mMeshes.emplace_back(std::move(group));


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

	return mWindowSystem->createWindow(desc);
}

void Euclid::initLights()
{
	mSun.color = glm::vec3(1.0f, 1.0f, 1.0f);
	mSun.power = 3.0f;
	mSun.directionWorld = SphericalCoordinate::cartesian({ 2.9f,0.515f, 1.0f}); //1.1f
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

	nex::LoggerManager::get()->setMinLogLevel(mSystemLogLevel);
	mConfig.write(mConfigFileName);
}


void Euclid::setupCallbacks()
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

		mControllerSM->onWindowsResize(width, height);
	});
}

void Euclid::setupGUI()
{
	using namespace nex::gui;


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
		mGlobalIllumination->getProbeCluster(),
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
		mCamera.get());
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
	textureView.showAllOptions(false);


	textureViewerWindow->addChild(std::move(textureViewer));
	textureViewerWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(textureViewerWindow));


	auto vobEditorWindow = std::make_unique<nex::gui::MenuWindow>(
		"Vob Editor",
		root->getMainMenuBar(),
		root->getToolsMenu());
	vobEditorWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	auto sceneNodeProperty = std::make_unique<VobEditor>(mWindow);
	sceneNodeProperty->setPicker(mControllerSM->getEditMode()->getPicker());
	sceneNodeProperty->setScene(&mScene);
	vobEditorWindow->addChild(std::move(sceneNodeProperty));
	root->addChild(move(vobEditorWindow));

	
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

}

void Euclid::setupCamera()
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