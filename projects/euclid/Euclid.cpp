#include <Euclid.hpp>
#include <EuclidRenderer.hpp>
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
#include <glm/gtc/matrix_transform.hpp>
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
#include <nex/sky/AtmosphericScattering.hpp>
#include <nex/renderer/MaterialDataUpdater.hpp>
#include <nex/GI/ProbeSelector.hpp>
#include <gui\Renderer_ConfigurationView.hpp>
#include <nex/gui/OceanGenerator.hpp>
#include <nex/import/ImportScene.hpp>

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
		mGlobals.getMetaFileExtension(),
		mGlobals.getEmbeddedTextureFileExtension());

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
	MeshManager::init(mGlobals.getResourceDirectoy(),
		mGlobals.getCompiledResourceDirectoy(),
		mGlobals.getCompiledVobFileExtension());
}

void nex::Euclid::initScene()
{
	auto* commandQueue = RenderEngine::getCommandQueue();


	mAtmosphericScattering = std::make_unique<AtmosphericScattering>();


	// init effect libary
	RenderBackend::get()->initEffectLibrary();
	mFlameShader = std::make_unique<FlameShader>();
	mParticleShader = std::make_unique<ParticleShader>();

	mGlobalIllumination = std::make_unique<GlobalIllumination>(1024, 100, 5);
	auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();
	voxelConeTracer->activate(true);

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

	mRenderer = std::make_unique<EuclidRenderer>(RenderBackend::get(),
		mPbrTechnique.get(),
		mCascadedShadow.get(),
		mAtmosphericScattering.get(),
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


	updateRenderContext(0.0f);

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


	mContext.boneTransformBuffer = std::make_shared<ShaderStorageBuffer>(Shader::DEFAULT_BONE_BUFFER_BINDING_POINT,
		sizeof(glm::mat4) * 100, nullptr, GpuBuffer::UsageHint::STREAM_DRAW);


	mContext.constantsBuffer = std::make_shared<UniformBuffer>(SHADER_CONSTANTS_UNIFORM_BUFFER_BINDING_POINT,
		sizeof(ShaderConstants),
		nullptr,
		nex::GpuBuffer::UsageHint::STREAM_DRAW);

	mContext.perObjectDataBuffer = std::make_shared<UniformBuffer>(OBJECT_SHADER_UNIFORM_BUFFER_BINDING_POINT,
		sizeof(PerObjectData),
		nullptr,
		nex::GpuBuffer::UsageHint::STREAM_DRAW);

	mContext.materialBuffer = std::make_shared<UniformBuffer>(SHADER_CONSTANTS_MATERIAL_BUFFER_BINDING_POINT,
		sizeof(PerObjectData) * MAX_PER_OBJECT_MATERIAL_DATA,
		nullptr,
		nex::GpuBuffer::UsageHint::STREAM_DRAW);

	updateShaderConstants();


	auto future = ResourceLoader::get()->enqueue<nex::Resource*>([=]()->nex::Resource* {
		createScene(RenderEngine::getCommandQueue());
		return nullptr;
		});

	//std::this_thread::sleep_for(std::chrono::seconds(5));

	ResourceLoader::get()->waitTillAllJobsFinished();
	executeTasks();

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

	auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();
	updateRenderContext(0.0f);
	updateShaderConstants();
	ResourceLoader::get()->waitTillAllJobsFinished();
	mRenderCommandQueue.useCameraCulling(mCamera.get());

	if (voxelConeTracer->isActive()){
		createVoxels();
	}

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
		executeTasks();

		if (isRunning())
		{
			mCamera->update();
			mControllerSM->frameUpdate(frameTime);
			renderFrame(frameTime);
			updateVoxelTexture();
			mWindow->swapBuffers();
		}
		else
		{
			mWindowSystem->waitForEvents();
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

void Euclid::collectCommands() 
{
	mScene.acquireLock();

	mScene.frameUpdate(mContext);
	mScene.updateWorldTrafoHierarchyUnsafe(false);
	mScene.calcSceneBoundingBoxUnsafe();

	mRenderCommandQueue.clear();
	mScene.collectRenderCommands(mRenderCommandQueue, false, mContext);

	mRenderCommandQueue.sort();
	mScene.setHasChangedUnsafe(false);
}


void Euclid::createScene(nex::RenderEngine::CommandQueue* commandQueue)
{
	LOG(mLogger, Info) << "create scene...";
	mScene.acquireLock();
	mScene.clearUnsafe();

	mVobBluePrintCache.clear();

	auto* meshManager = MeshManager::get();
	auto& meshFileSystem = meshManager->getFileSystem();


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

	PbrMaterialLoader materialLoader(deferred->getGeometryShaderProvider(), 
		deferred->getGeometryBonesShaderProvider(), 
		forward->getShaderProvider(),
		forward->getBoneShaderProvider(),
		TextureManager::get());

	// vob hierarchy test
	if (false) {

		//C:/Development/Repositories/Euclid/_work/data/meshes/cerberus/Cerberus.obj
		//"C:/Development/Repositories/Euclid/_work/data/anims/soldier_armor/soldier_armor1.glb"

		auto scene = nex::ImportScene::read("C:/Development/Repositories/Euclid/_work/data/anims/soldier_armor/soldier_armor5.glb", true);
		nex::NodeHierarchyLoader nodeHierarchyLoader(&scene, &materialLoader);
		auto vobBaseStore = nodeHierarchyLoader.load(AnimationManager::get());
		auto vob = meshManager->createVob(vobBaseStore, materialLoader);
		vob->updateTrafo(true, true);

		auto* vobPtr = vob.get();

		vob->getName() = "vob hierarchy root";
		vob->setPositionLocalToParent(glm::vec3(-6.000f, 0.470f, -0.800f));
		//vob->setScaleLocalToParent(glm::vec3(0.01f));

		/*glm::mat4 scale(1.0f);
		scale[0][0] = 0.01f;
		scale[1][1] = 0.01f;
		scale[2][2] = 0.01f;*/

		//vob->setTrafoMeshToLocal(scale);
		vob->updateTrafo(true, true);

		mScene.addVobUnsafe(std::move(vob), true);

		commandQueue->push([vobPtr]() {

			std::vector<Vob*> queue;

			queue.push_back(vobPtr);

			while (!queue.empty()) {

				auto* vob = queue.back();
				queue.pop_back();

				if (auto* group = vob->getMeshGroup()) {
					group->finalize();
				}

				for (auto& child : vob->getChildren()) {
					queue.push_back(child.get());
				}
			}

			

		});
	}


	//cerberus
	if (true) {

		auto cerberus = loadVob("meshes/cerberus/Cerberus.obj", commandQueue, materialLoader);
		cerberus->getName() = "cerberus";
		cerberus->setPositionLocalToParent(glm::vec3(0.0f, 2.0f, 0.0f));
		mScene.addVobUnsafe(std::move(cerberus));
	}

	
	// sponza
	
	if (true) {

		auto sponzaVob = loadVob("meshes/sponza/sponzaSimple7.obj", commandQueue, materialLoader);
		sponzaVob->getName() = "sponzaSimple1";
		sponzaVob->setPositionLocalToParent(glm::vec3(0.0f, 0.0f, 0.0f));
		sponzaVob->setIsStatic(true);

		auto& materialData = sponzaVob->getPerObjectMaterialData();

		materialData.probesUsed = 0;
		materialData.coneTracingUsed = 1;

		mScene.addVobUnsafe(std::move(sponzaVob));
	}
	


	//bone animations
	RiggedVob* bobVobPtr = nullptr;
	if (false) {
		nex::SkinnedMeshLoader meshLoader;
		auto* fileSystem = nex::AnimationManager::get()->getRiggedMeshFileSystem();
		auto bobVob = loadVob("anims/bob/boblampclean.md5mesh", commandQueue, materialLoader, fileSystem);

		auto* ani = nex::AnimationManager::get()->loadBoneAnimation("bob/boblampclean.md5anim");
		bobVobPtr = (RiggedVob*)bobVob.get();
		bobVobPtr->setActiveAnimation(ani);

		//bobVob->setDefaultScale(0.03f);

		bobVob->setTrafoMeshToLocal(glm::scale(glm::mat4(1.0f), glm::vec3(0.03f)));

		bobVob->setPositionLocalToParent(glm::vec3(0, 0.0f, 0.0f));
		//bobVob->setPosition(glm::vec3(-5.5f, 6.0f, 0.0f));
		//bobVob->setScaleLocal(glm::vec3(0.03f));

		bobVob->setRotationLocalToParent(glm::vec3(glm::radians(-90.0f), glm::radians(90.0f), 0.0f));
		mScene.addVobUnsafe(std::move(bobVob));
	}
	


	if (false) {
		//meshContainer = MeshManager::get()->getModel("transparent/transparent.obj");

		auto transparentVob3 = loadVob("meshes/transparent/transparent_intersected_resolved.obj", commandQueue, materialLoader);

		transparentVob3->getName() = "transparent - 3";
		transparentVob3->setPositionLocalToParent(glm::vec3(-12.0f, 2.0f, 0.0f));

		if (bobVobPtr) bobVobPtr->addChild(nex::make_not_owning(transparentVob3.get()));
		mScene.addVobUnsafe(std::move(transparentVob3));



		// flame test
		auto flameVobBluePrint = loadVob("misc/plane_simple.obj", commandQueue, flameMaterialLoader);

		auto flameVob = std::make_unique<Billboard>();
		flameVob->setMeshGroup(nex::make_not_owning(flameVobBluePrint->getMeshGroup()));
		flameVob->setPositionLocalToParent(glm::vec3(1.0, 0.246f, 3 + 0.056f));
		flameVob->setRotationLocalToParent(glm::vec3(glm::radians(0.0f), glm::radians(-90.0f), glm::radians(0.0f)));
		mScene.addVobUnsafe(std::move(flameVob));
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
				0.1f, //murk
				0.1f, //roughness
				mCascadedShadow.get(),
				mPSSR.get()
				);

			ocean->init();

			oceanVobPtr->setOcean(std::move(ocean));
			oceanVobPtr->updateTrafo(true, true);
			oceanVobPtr->updateWorldTrafoHierarchy(true);
			oceanVobPtr->resize(mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
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

void nex::Euclid::executeTasks()
{
	auto* commandQueue = RenderEngine::getCommandQueue();
	auto& exceptionQueue = ResourceLoader::get()->getExceptionQueue();

	while (!commandQueue->empty()) {
		auto task = commandQueue->pop();
		task();
	}

	while (!exceptionQueue.empty()) {
		auto& exception = exceptionQueue.pop();
		throw_with_trace(*exception);
	}
}

void Euclid::initLights()
{
	mSun.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	mSun.power = 3.0f;
	mSun.directionWorld = glm::vec4(SphericalCoordinate::cartesian({ 2.600f,0.100f, 1.0f }), 0.0f); //1.1f

	mCurrentSunDir = mSun.directionWorld;

	//mSun.directionWorld = SphericalCoordinate::cartesian({ 1.598f, 6.555f, 1.0f }); //1.1f
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

void nex::Euclid::createVoxels()
{
	auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();
	auto lock = mScene.acquireLock();
	mScene.updateWorldTrafoHierarchyUnsafe(true);
	mScene.calcSceneBoundingBoxUnsafe();
	updateShaderConstants();

	voxelConeTracer->voxelizeVobs(mScene, mSun, mGiShadowMap.get(), &mContext);
}

void nex::Euclid::renderFrame(float frameTime)
{
	auto* gui = nex::gui::ImGUI_Impl::get();
	auto* backend = RenderBackend::get();
	auto* screenSprite = backend->getScreenSprite();
	auto* lib = backend->getEffectLibrary();
	auto* postProcessor = lib->getPostProcessor();
	auto* taa = postProcessor->getTAA();
	auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();

	auto* activeController = mControllerSM->getActiveController();
	gui->newFrame(frameTime, activeController->allowsInputForUI());



	updateRenderContext(frameTime);

	//update jitter for next frame
	//taa->advanceJitter();
	//mCamera->setJitter(taa->getJitterMatrix());
	//mCamera->setJitterVec(taa->getJitterVec());


	if (mCascadedShadow->isEnabled())
	{
		mCascadedShadow->useTightNearFarPlane(false);
		//mCascadedShadow->frameUpdate(*mCamera, mSun.directionWorld, nullptr);
		//mCascadedShadow->frameReset();
	}


	// shader constants have to be updated before collecting render commands!
	updateShaderConstants();
	collectCommands();

	auto* screenRT = backend->getDefaultRenderTarget();
	Texture* texture = nullptr;
	SpriteShader* spriteShader = nullptr;

	screenSprite->setWidth(mContext.windowWidth / mRenderScale);
	screenSprite->setHeight(mContext.windowHeight / mRenderScale);
	screenSprite->setPosition({ 0, 0 });



	if (voxelConeTracer->getVisualize()) {
		texture = visualizeVoxels();
	}
	else
	{
		mRenderer->render(mRenderCommandQueue, mContext, false);
		const auto& renderLayer = mRenderer->getRenderLayers()[mRenderer->getActiveRenderLayer()];
		texture = renderLayer.textureProvider();
		spriteShader = renderLayer.spriteShaderProvider();
	}

	//texture = mRenderer->getGbuffer()->getNormal();

	if (texture != nullptr) {
		screenRT->bind();
		//backend->setViewPort(offsetX, offsetY, mWindow->getFrameBufferWidth(), mWindow->getFrameBufferHeight());
		//backend->setBackgroundColor(glm::vec3(1.0f));
		//screenRT->clear(Color | Stencil | Depth);

		screenSprite->setTexture(texture);
		screenSprite->render(spriteShader);
	}

	mCurrentSunDir = mSun.directionWorld;
	mControllerSM->getDrawable()->drawGUI();
	ImGui::Render();
	gui->renderDrawData(ImGui::GetDrawData());
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
			width *= mRenderScale;
			height *= mRenderScale;

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

		mScene.resize(width, height);
	});
}

void Euclid::setupGUI()
{
	using namespace nex::gui;


	nex::gui::VobViewMapper::init(mGlobalIllumination->getProbeManager(), mWindow);

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

	auto rendererView = std::make_unique<Renderer_ConfigurationView>(mRenderer.get());
	graphicsTechniques->addChild(move(rendererView));

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


	auto oceanGeneratorWindow = std::make_unique<nex::gui::MenuWindow>(
		"Ocean Generator",
		root->getMainMenuBar(),
		root->getToolsMenu());
	oceanGeneratorWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	auto oceanGenerator = std::make_unique<nex::gui::OceanGenerator>(&mScene, mCascadedShadow.get(), mPSSR.get(), mCamera.get(), &mContext);
	oceanGeneratorWindow->addChild(std::move(oceanGenerator));
	root->addChild(move(oceanGeneratorWindow));


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
		&mVobBluePrintCache,
		mPbrTechnique.get(),
		mWindow,
		mCamera.get());
	vobLoaderWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(vobLoaderWindow));

	auto voxelConeTracerViewWindow = std::make_unique<nex::gui::VoxelConeTracerView>(
		"Voxel-based cone tracer",
		root->getMainMenuBar(),
		root->getToolsMenu(),
		mGlobalIllumination->getVoxelConeTracer(),
		&mSun,
		mGiShadowMap.get(),
		&mScene,
		&mContext);

	voxelConeTracerViewWindow->useStyleClass(std::make_shared<nex::gui::ConfigurationStyle>());
	root->addChild(move(voxelConeTracerViewWindow));



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
	auto cameraPos = glm::vec3(-10.346f, 1.856f, -0.659f);
	mCamera->setPosition(cameraPos, true);
	auto look = glm::vec3(-0.126f, 1.906f, -0.900f) - cameraPos;

	
	
	//mCamera->setPosition(glm::vec3(-31.912f, 25.110f, 52.563), true);
	//look = glm::vec3(-3.888f, 2.112, 0.094f) - glm::vec3(-31.912f, 25.110f, 52.563);

	mCamera->setLook(normalize(look));
	mCamera->setUp(glm::vec3(0.0f, 1.0f, 0.0f));
	mCamera->setDimension(windowWidth, windowHeight);


	mCamera->setNearDistance(0.2f);
	mCamera->setFarDistance(60.0f);

	mCamera->update();
}

void nex::Euclid::updateRenderContext(float frameTime)
{
	auto* backend = RenderBackend::get();
	auto* lib = backend->getEffectLibrary();

	mContext.camera = mCamera.get();
	mContext.proj = &mCamera->getProjectionMatrix();
	mContext.view = &mCamera->getView();
	mContext.csm = mCascadedShadow.get();
	mContext.gi = mGlobalIllumination.get();
	mContext.lib = lib;
	mContext.sun = &mSun;
	mContext.stencilTest = backend->getStencilTest();
	mContext.invViewProj = &mCamera->getViewProjInv();
	mContext.irradianceAmbientReflection = mRenderer->getActiveIrradianceAmbientReflectionRT();
	mContext.out = mRenderer->getOutRT();
	mContext.outStencilView = mRenderer->getOutStencilView();
	mContext.pingPong = mRenderer->getPingPongRT();
	mContext.pingPongStencilView = mRenderer->getPingPongStencilView();
	mContext.time = mTimer.getCountedTimeInSeconds();
	mContext.frameTime = frameTime;
	mContext.windowWidth = mRenderScale * mWindow->getFrameBufferWidth();
	mContext.windowHeight = mRenderScale * mWindow->getFrameBufferHeight();	
}

void nex::Euclid::updateShaderConstants()
{
	auto& constants = mContext.constants;
	auto& buffer = mContext.constantsBuffer;

	constants.cameraPositionWS = glm::vec4(mCamera->getPosition(), 1.0f);
	constants.viewport = glm::vec4(mRenderScale * mWindow->getFrameBufferWidth(), mRenderScale * mWindow->getFrameBufferHeight(), 0.0f, 0.0f);
	
	constants.viewGPass = mCamera->getView();
	constants.invViewGPass = mCamera->getViewInv();
	constants.invViewRotGPass = glm::mat4(inverse(glm::mat3(constants.viewGPass)));
	constants.projectionGPass = mCamera->getProjectionMatrix();
	constants.invProjectionGPass = glm::inverse(constants.projectionGPass);
	constants.invViewProjectionGPass = mCamera->getViewProjInv();
	constants.prevViewProjectionGPass = mCamera->getViewProjPrev();

	constants.nearFarPlaneGPass = glm::vec4(mCamera->getNearDistance(), mCamera->getFarDistance(), 0.0f, 0.0f);

	constants.ambientLightPower = mGlobalIllumination->getAmbientPower(); //TODO
	constants.shadowStrength = mCascadedShadow->getShadowStrength();

	constants.dirLight = mSun;
	constants.cascadeData = mCascadedShadow->getCascadeData();

	//voxel based cone tracing is updated during voxelization

	//atmospheric scattering
	constants.atms_intensity = mSun.power;//1.8f;
	constants.atms_step_count = 16;
	constants.atms_surface_height = 0.99f;
	constants.atms_scatter_strength = 0.028f;
	constants.atms_spot_brightness = std::fmax(4.0f * mSun.power, 1.0f);

	constants.atms_mie_brightness = 0.1f;
	constants.atms_mie_collection_power = 0.39f;
	constants.atms_mie_distribution = 0.63f;
	constants.atms_mie_strength = 0.264f;

	constants.atms_rayleigh_brightness = 3.3f;
	constants.atms_rayleigh_collection_power = 0.81f;
	constants.atms_rayleigh_strength = 0.139f;

	buffer->resize(sizeof(ShaderConstants), nullptr, GpuBuffer::UsageHint::STREAM_DRAW);
	buffer->update(sizeof(ShaderConstants), &constants);
	buffer->bindToTarget();


	auto* selector = ProbeSelector::getSelector(mRenderer->getProbeSelectionAlg());
	ProbeSelector::assignProbes(mScene, selector, Probe::Type::Irradiance);
	ProbeSelector::assignProbes(mScene, selector, Probe::Type::Reflection);

	nex::MaterialDataUpdater::updateMaterialData(&mScene, mContext.materialBuffer.get());

	mContext.materialBuffer->bindToTarget();

	mContext.perObjectDataBuffer->bindToTarget();
}

void nex::Euclid::updateVoxelTexture()
{
	auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();
	if (voxelConeTracer->isVoxelLightingDeferred() && voxelConeTracer->isActive())
	{
		auto diffSun = mCurrentSunDir - mSun.directionWorld;
		if (mSun._pad[0] != 0.0) {
			mSun._pad[0] = 0.0;

			voxelConeTracer->updateVoxelLighting(mGiShadowMap.get(), mScene, mSun, mContext);
		}
	}
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

nex::Texture* nex::Euclid::visualizeVoxels()
{
	auto* backend = RenderBackend::get();
	auto* voxelConeTracer = mGlobalIllumination->getVoxelConeTracer();
	static auto* depthTest = backend->getDepthBuffer();
	depthTest->enableDepthBufferWriting(true);
	auto* tempRT = mRenderer->getOutRT();

	tempRT->bind();
	backend->setViewPort(0, 0, mContext.windowWidth, mContext.windowHeight);
	backend->setBackgroundColor(glm::vec3(1.0f));
	tempRT->clear(Color | Stencil | Depth);

	voxelConeTracer->renderVoxels(mCamera->getProjectionMatrix(), mCamera->getView());

	return tempRT->getColorAttachmentTexture(0);
}

std::unique_ptr<nex::Vob> nex::Euclid::loadVob(const std::filesystem::path& p,
	nex::RenderEngine::CommandQueue* commandQueue, 
	const AbstractMaterialLoader& materialLoader,
	const nex::FileSystem* fileSystem)
{
	if (!fileSystem) fileSystem = &MeshManager::get()->getFileSystem();

	auto path = fileSystem->resolvePath(p);
	auto id = SID(path.generic_string());
	auto* vobPtr = mVobBluePrintCache.getCachedPtr(id);

	if (!vobPtr) {
		auto vob = MeshManager::get()->loadVobHierarchy(path, materialLoader, 1.0f);
		vobPtr = vob.get();

		commandQueue->push([vobPtr = vobPtr]() {
			vobPtr->finalizeMeshes();
		});

		mVobBluePrintCache.insert(id, std::move(vob));
	}

	return vobPtr->createBluePrintCopy();
}