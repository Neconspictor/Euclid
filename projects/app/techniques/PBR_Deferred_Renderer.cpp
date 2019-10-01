#include <techniques/PBR_Deferred_Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/shader/SkyBoxPass.hpp>
#include <nex/Scene.hpp>
#include <nex/shader/DepthMapPass.hpp>
#include <nex/shader/SpritePass.hpp>
#include <nex/texture/TextureManager.hpp>
//#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include "nex/mesh/StaticMeshManager.hpp"
#include <nex/texture/GBuffer.hpp>
#include "nex/shadow/CascadedShadow.hpp"
#include <nex/drawing/StaticMeshDrawer.hpp>
#include "nex/renderer/RenderBackend.hpp"

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/string_cast.hpp>

#include  <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/EffectLibrary.hpp>
#include "nex/texture/Attachment.hpp"
#include "nex/post_processing/PostProcessor.hpp"
#include "imgui/imgui.h"
#include <nex/pbr/PbrDeferred.hpp>
#include "nex/pbr/PbrProbe.hpp"
#include "nex/sky/AtmosphericScattering.hpp"
#include <nex/texture/Sampler.hpp>
#include "nex/pbr/PbrForward.hpp"
#include "nex/camera/FPCamera.hpp"
#include <nex/Input.hpp>
#include "nex/gui/Util.hpp"
#include <unordered_set>
#include <nex/pbr/Pbr.hpp>
#include <nex/pbr/Cluster.hpp>
#include <nex/pbr/GlobalIllumination.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/post_processing/PostProcessor.hpp>
#include <nex/post_processing/SMAA.hpp>

int ssaaSamples = 1;

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
nex::PBR_Deferred_Renderer::PBR_Deferred_Renderer(
	nex::RenderBackend* backend,
	PbrTechnique* pbrTechnique,
	CascadedShadow* cascadedShadow,
	nex::Input* input) :

	Renderer(pbrTechnique),
	blurEffect(nullptr),
	m_logger("PBR_Deferred_Renderer"),
	mRenderTargetSingleSampled(nullptr),
	//shadowMap(nullptr),

	mShowDepthMap(false),
	mAtmosphericScattering(std::make_unique<AtmosphericScattering>()),
	mInput(input),
	mCascadedShadow(cascadedShadow),
	mRenderBackend(backend),
	mOcean(64, //N
		64, // maxWaveLength
		1.0f, //dimension
		0.4f, //spectrumScale
		glm::vec2(0.0f, 1.0f), //windDirection
		32.0f, //windSpeed
		200.0f //periodTime
	)
{

	//*28.0 * 0.277778
	assert(mPbrTechnique != nullptr);
	assert(mCascadedShadow != nullptr);
	assert(mInput != nullptr);
}

nex::PBR_Deferred_Renderer::~PBR_Deferred_Renderer() = default;


bool nex::PBR_Deferred_Renderer::getShowDepthMap() const
{
	return mShowDepthMap;
}

void nex::PBR_Deferred_Renderer::init(int windowWidth, int windowHeight)
{
	LOG(m_logger, LogLevel::Info) << "PBR_Deferred_Renderer::init called!";


	//CubeMap* cubeMapSky = textureManager->createCubeMap("skyboxes/sky_right.jpg", "skyboxes/sky_left.jpg",
	//	"skyboxes/sky_top.jpg", "skyboxes/sky_bottom.jpg",
	//	"skyboxes/sky_back.jpg", "skyboxes/sky_front.jpg", true);

	auto* effectLib = mRenderBackend->getEffectLibrary();
	//auto* equirectangularSkyBoxShader = effectLib->getEquirectangularSkyBoxShader();
	//auto* panoramaSkyBoxShader = effectLib->getPanoramaSkyBoxShader();
	//auto* skyboxShader = effectLib->getSkyBoxShader();

	//shadowMap = m_renderBackend->createDepthMap(2048, 2048);


	mPbrMrt = mPbrTechnique->getDeferred()->createMultipleRenderTarget(windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	mRenderTargetSingleSampled = createLightingTarget(windowWidth, windowHeight, mPbrMrt.get());
	mPingPong = mRenderBackend->createRenderTarget();

	//panoramaSkyBoxShader->bind();
	//panoramaSkyBoxShader->setSkyTexture(panoramaSky);

	//equirectangularSkyBoxShader->bind();
	//equirectangularSkyBoxShader->setSkyTexture(panoramaSky);


	blurEffect = mRenderBackend->getEffectLibrary()->getGaussianBlur();

	mRenderBackend->getRasterizer()->enableScissorTest(false);

	auto* mAoSelector = mRenderBackend->getEffectLibrary()->getPostProcessor()->getAOSelector();
	mAoSelector->setUseAmbientOcclusion(true);
	mAoSelector->setAOTechniqueToUse(AOTechnique::HBAO);


	mRenderlayers["composited"] = [&]() { return mRenderTargetSingleSampled->getColorAttachmentTexture(0); };
	mRenderlayers["GBuffer: normal - view space"] = [&]() { return  mPbrMrt->getNormal(); };
	mRenderlayers["GBuffer: albedo"] = [&]() { return mPbrMrt->getAlbedo(); };
	mRenderlayers["GBuffer: ambient occlusion, metalness, roughness"] = [&]() { return mPbrMrt->getAoMetalRoughness();};
	mRenderlayers["motion"] = [&]() { return mRenderTargetSingleSampled->getColorAttachmentTexture(2); };
	mRenderlayers["luminance"] = [&]() { return mRenderTargetSingleSampled->getColorAttachmentTexture(1); };
	mRenderlayers["ambient occlusion"] = [&]() { return getAOSelector()->getRenderResult(); };
	mRenderlayers["pre-post process"] = [&]() { return mRenderTargetSingleSampled->getColorAttachmentTexture(0); };
	mRenderlayers["post processed (without antialising)"] = [&]() { return mPingPong->getColorAttachmentTexture(0); };
	mRenderlayers["SMAA - edge"] = []() { return RenderBackend::get()->
		getEffectLibrary()->
		getPostProcessor()->
		getSMAA()->
		getEdgeDetection(); };
	mRenderlayers["SMAA - blend"] = []() { return RenderBackend::get()->
		getEffectLibrary()->
		getPostProcessor()->
		getSMAA()->
		getBlendingWeight(); };


	for (const auto& it : mRenderlayers)
		mRenderLayerDescs.push_back(it.first);

	setActiveRenderLayer("composited");
}


void nex::PBR_Deferred_Renderer::render(const RenderCommandQueue& queue, 
	const Camera& camera,
	const DirLight& sun,
	unsigned windowWidth, 
	unsigned windowHeight,
	bool postProcess,
	RenderTarget* out)
{

	/*FPCamera* fp = (FPCamera*)camera;
	fp->setPosition({3.527f, 5.133f, 1.022f});
	fp->setYaw(-1.450f);
	fp->setPitch(45.052f);
	fp->recalculateLookVector();
	fp->calcView();*/

	Pass::Constants constants;
	constants.camera = &camera;
	constants.windowWidth = windowWidth;
	constants.windowHeight = windowHeight;


	static auto* depthTest = RenderBackend::get()->getDepthBuffer();
	depthTest->enableDepthBufferWriting(true);

	static bool switcher = true;
	if (mInput->isPressed(Input::KEY_O))
	{
		switcher = !switcher;
	}

	if (switcher)
	{
		renderDeferred(queue, constants, sun);
	}
	else
	{
		renderForward(queue, constants, sun);
	}

	auto* stencilTest = mRenderBackend->getStencilTest();

	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->enableStencilTest(true);

	//mTesselationTest.draw(camera, sun->getDirection());
	//mOcean.draw(camera.getProjectionMatrix(), camera.getView(), sun.directionWorld);


	stencilTest->setCompareFunc(CompareFunction::NOT_EQUAL, 1, 1);

	renderSky(constants, sun);
	stencilTest->enableStencilTest(false);


	auto* colorTex = static_cast<Texture2D*>(mRenderTargetSingleSampled->getColorAttachmentTexture(0));
	auto* luminanceTexture = static_cast<Texture2D*>(mRenderTargetSingleSampled->getColorAttachmentTexture(1));
	auto* motionTexture = static_cast<Texture2D*>(mRenderTargetSingleSampled->getColorAttachmentTexture(2));
	auto* depthTexture = static_cast<Texture2D*>(mRenderTargetSingleSampled->getColorAttachmentTexture(3));
	auto* depthTexture2 = static_cast<Texture2D*>(mRenderTargetSingleSampled->getDepthAttachment()->texture.get());


	// instead of clearing the buffer we just disable depth and stencil tests for improved performance
	RenderBackend::get()->getDepthBuffer()->enableDepthTest(false);
	RenderBackend::get()->getStencilTest()->enableStencilTest(false);

	// finally render the offscreen buffer to a quad and do post processing stuff


	static auto* postProcessor = RenderBackend::get()->getEffectLibrary()->getPostProcessor();

	auto* aoMap = postProcessor->getAOSelector()->renderAO(camera, depthTexture2); //mPbrMrt->getNormalizedViewSpaceZ()

	// After sky we render transparent objects
	mRenderTargetSingleSampled->bind();
	mPbrTechnique->useForward();
	auto* forward = mPbrTechnique->getForward();
	forward->configurePass(constants);
	forward->updateLight(sun, camera);
	StaticMeshDrawer::draw(queue.getTransparentCommands());

	// At last we render tools
	StaticMeshDrawer::draw(queue.getToolCommands());


	Texture2D* outputTexture = colorTex;

	if (postProcess) {
		outputTexture = (Texture2D*)postProcessor->doPostProcessing(colorTex, luminanceTexture, aoMap, motionTexture, mPingPong.get()); //mPbrMrt->getMotion()
	}

	postProcessor->antialias(outputTexture, out);

	depthTest->enableDepthBufferWriting(true);
}

void nex::PBR_Deferred_Renderer::setShowDepthMap(bool showDepthMap)
{
	this->mShowDepthMap = showDepthMap;
}

void nex::PBR_Deferred_Renderer::updateRenderTargets(unsigned width, unsigned height)
{
	Renderer::updateRenderTargets(width, height);
	//update render target dimension
	//the render target dimensions are dependent from the viewport size
	// so first update the viewport and than recreate the render targets
	mRenderBackend->resize(width, height);
	mPbrMrt = mPbrTechnique->getDeferred()->createMultipleRenderTarget(width, height);
	mRenderTargetSingleSampled = createLightingTarget(width, height, mPbrMrt.get());
	mPingPong = mRenderBackend->createRenderTarget();
}

nex::AmbientOcclusionSelector* nex::PBR_Deferred_Renderer::getAOSelector()
{
	return mRenderBackend->getEffectLibrary()->getPostProcessor()->getAOSelector();
}

nex::PBR_GBuffer* nex::PBR_Deferred_Renderer::getGbuffer()
{
	return mPbrMrt.get();
}

nex::TesselationTest* nex::PBR_Deferred_Renderer::getTesselationTest()
{
	return &mTesselationTest;
}

nex::Ocean* nex::PBR_Deferred_Renderer::getOcean()
{
	return &mOcean;
}

nex::CascadedShadow* nex::PBR_Deferred_Renderer::getCascadedShadow()
{
	return mCascadedShadow;
}

void nex::PBR_Deferred_Renderer::pushDepthFunc(std::function<void()> func)
{
	mDepthFuncs.emplace_back(std::move(func));
}

nex::RenderTarget* nex::PBR_Deferred_Renderer::getTempRendertTarget()
{
	return mRenderTargetSingleSampled.get();
}

void nex::PBR_Deferred_Renderer::renderShadows(const nex::RenderCommandQueue::Buffer& shadowCommands, 
	const Pass::Constants& constants, const DirLight& sun, Texture2D* depth)
{
	if (mCascadedShadow->isEnabled())
	{
		mCascadedShadow->useTightNearFarPlane(false);
		mCascadedShadow->frameUpdate(*constants.camera, sun.directionWorld, depth);
		TransformPass* depthPass = mCascadedShadow->getDepthPass();
		depthPass->bind();
		depthPass->updateConstants(constants);

		for (unsigned i = 0; i < mCascadedShadow->getCascadeData().numCascades; ++i)
		{
			mCascadedShadow->begin(i);
			for (const auto& command : shadowCommands)
			{
				depthPass->setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
				depthPass->uploadTransformMatrices();
				StaticMeshDrawer::draw(command.mesh, nullptr);
			}
		}

		mCascadedShadow->frameReset();
	}
}

void nex::PBR_Deferred_Renderer::renderDeferred(const RenderCommandQueue& queue, 
	const Pass::Constants& constants, const DirLight& sun)
{
	static auto* stencilTest = RenderBackend::get()->getStencilTest();
	static auto* depthTest = RenderBackend::get()->getDepthBuffer();

	using namespace std::chrono;

	const auto& camera = *constants.camera;

	// update and render into cascades
	mPbrMrt->bind();


	mRenderBackend->setViewPort(0, 0, constants.windowWidth * ssaaSamples, constants.windowHeight * ssaaSamples);
	//renderer->beginScene();

	mPbrMrt->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil); //RenderComponent::Color |


	stencilTest->enableStencilTest(true);
	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

	//mPbrTechnique->getDeferred()->configureGeometryPass(camera);
	RenderState state;
	//state.doCullFaces = false;
	//state.fillMode = FillMode::LINE;

	mPbrTechnique->useDeferred();
	for (nex::Technique* technique : queue.getTechniques())
	{
		//technique->configureSubMeshPass(camera);
		technique->getActiveSubMeshPass()->setViewProjectionMatrices(camera.getProjectionMatrix(), camera.getView(), camera.getPrevView());
		technique->getActiveSubMeshPass()->updateConstants(constants);
	}
	StaticMeshDrawer::draw(queue.getDeferrablePbrCommands());

	for (auto& func : mDepthFuncs) {
		func();
	}
	mDepthFuncs.clear();

	stencilTest->enableStencilTest(false);
	//glm::vec2 minMaxPositiveZ(0.0f, 1.0f);

	//minMaxPositiveZ.x = camera->getFrustum(Perspective).nearPlane;
	//minMaxPositiveZ.y = camera->getFrustum(Perspective).farPlane;

	auto* gBufferDepth = (Texture2D*)mPbrMrt->getDepthAttachment()->texture.get();
	renderShadows(queue.getShadowCommands(), constants, sun, gBufferDepth); //mPbrMrt->getNormalizedViewSpaceZ()


	auto* globalIllumination = mPbrTechnique->getDeferred()->getGlobalIllumination(); 

	if (globalIllumination && false) {

		auto* probeCluster = globalIllumination->getProbeCluster();

		probeCluster->generateCluster(glm::uvec4(16, 8, 4, 6), gBufferDepth, &camera, nullptr);

		auto* envLightCuller = probeCluster->getEnvLightCuller();
		auto* envLightsBuffer = globalIllumination->getEnvironmentLightShaderBuffer();
		envLightCuller->cullLights(camera.getView(), probeCluster->getClusterAABBBuffer(), envLightsBuffer);

		// Readback the generated clusters
		/*auto* clusterBuffer = mProbeCluster->getClusterAABBBuffer();

		auto* clusters = (cluster::AABB*)clusterBuffer->map(GpuBuffer::Access::READ_ONLY);
		auto* envLights = (EnvironmentLight*)envLightsBuffer->map(GpuBuffer::Access::READ_ONLY);

		auto* globalLightIndexCount = (EnvLightCuller::GlobalLightIndexCount*)envLightCuller->getGlobalLightIndexCount()->map(GpuBuffer::Access::READ_ONLY);
		auto* globalLightIndexList = (EnvLightCuller::GlobalLightIndexListElement*)envLightCuller->getGlobalLightIndexList()->map(GpuBuffer::Access::READ_ONLY);
		auto* lightGrids = (cluster::LightGrid*)envLightCuller->getLightGrids()->map(GpuBuffer::Access::READ_ONLY);

		clusterBuffer->unmap();
		envLightsBuffer->unmap();
		envLightCuller->getGlobalLightIndexCount()->unmap();
		envLightCuller->getGlobalLightIndexList()->unmap();
		envLightCuller->getLightGrids()->unmap();*/
	}
	
	


	// render scene to a offscreen buffer
	mRenderTargetSingleSampled->bind();
	mRenderBackend->setViewPort(0, 0, constants.windowWidth * ssaaSamples, constants.windowHeight * ssaaSamples);

	mRenderTargetSingleSampled->enableDrawToColorAttachment(2, false); // we don't want to clear motion buffer
	mRenderTargetSingleSampled->enableDrawToColorAttachment(3, false); // we don't want to clear depth (for e.g. ambient occlusion)
	mRenderTargetSingleSampled->clear(RenderComponent::Color);//RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil
	mRenderTargetSingleSampled->enableDrawToColorAttachment(2, true);
	mRenderTargetSingleSampled->enableDrawToColorAttachment(3, true);

	
	
	
	depthTest->enableDepthTest(false);
	depthTest->enableDepthBufferWriting(false);
	stencilTest->enableStencilTest(true);
	stencilTest->setCompareFunc(CompareFunction::EQUAL, 1, 1);


	auto* deferred = mPbrTechnique->getDeferred();
	deferred->drawLighting(mPbrMrt.get(), constants, sun);

	//stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 1);

	depthTest->enableDepthTest(true);
	depthTest->enableDepthBufferWriting(true);
	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->setOperations(StencilTest::Operation::REPLACE, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);


	mPbrTechnique->useForward();
	auto* forward = mPbrTechnique->getForward();
	forward->configurePass(constants);
	forward->updateLight(sun, camera);

	if (globalIllumination)
		globalIllumination->drawTest(camera.getProjectionMatrix(), camera.getView(), 
			mRenderTargetSingleSampled->getDepthAttachment()->texture.get());

	StaticMeshDrawer::draw(queue.getForwardCommands());
	StaticMeshDrawer::draw(queue.getProbeCommands());

	stencilTest->setCompareFunc(CompareFunction::EQUAL, 1, 1);
}

void nex::PBR_Deferred_Renderer::renderForward(const RenderCommandQueue& queue,
	const Pass::Constants& constants, const DirLight& sun)
{
	static auto* stencilTest = RenderBackend::get()->getStencilTest();

	const auto& camera = *constants.camera;

	RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);
	RenderBackend::get()->getDepthBuffer()->enableDepthBufferWriting(true);
	//RenderBackend::get()->getDepthBuffer()->enableDepthClamp(true);


	// update and render into cascades
	mRenderTargetSingleSampled->bind();


	mRenderBackend->setViewPort(0, 0, constants.windowWidth * ssaaSamples, constants.windowHeight * ssaaSamples);
	//renderer->beginScene();

	mRenderTargetSingleSampled->clear(RenderComponent::Depth); //RenderComponent::Color |


	stencilTest->enableStencilTest(false);
	auto* depthPass = mRenderBackend->getEffectLibrary()->getDepthMapShader();
	//mCommandQueue.getDeferredCommands()  mCommandQueue.getShadowCommands()
	depthPass->bind();
	depthPass->updateViewProjection(constants.camera->getProjectionMatrix(), constants.camera->getView());
	StaticMeshDrawer::draw(queue.getShadowCommands(), depthPass);
	renderShadows(queue.getShadowCommands(), constants, sun, (Texture2D*)mRenderTargetSingleSampled->getDepthAttachment()->texture.get());


	// render scene to a offscreen buffer
	mRenderTargetSingleSampled->bind();

	mRenderBackend->setViewPort(0, 0, constants.windowWidth * ssaaSamples, constants.windowHeight * ssaaSamples);
	mRenderTargetSingleSampled->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);//RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil


	stencilTest->enableStencilTest(true);
	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

	mPbrTechnique->useForward();
	auto* forward = mPbrTechnique->getForward();
	forward->configurePass(constants);
	forward->updateLight(sun, *constants.camera);

	//mPbrForward->configureSubMeshPass(camera);
	//mPbrForward->getActiveSubMeshPass()->updateConstants(camera);

	for (nex::Technique* technique : queue.getTechniques())
	{
		//technique->configureSubMeshPass(camera);
		technique->getActiveSubMeshPass()->setViewProjectionMatrices(camera.getProjectionMatrix(), camera.getView(), camera.getPrevView());
		technique->getActiveSubMeshPass()->updateConstants(constants);
	}

	StaticMeshDrawer::draw(queue.getDeferrablePbrCommands()); //TODO
	StaticMeshDrawer::draw(queue.getForwardCommands());
	StaticMeshDrawer::draw(queue.getProbeCommands());
}

void nex::PBR_Deferred_Renderer::renderSky(const Pass::Constants& constants, const DirLight& sun)
{

	const auto& camera = *constants.camera;

	mAtmosphericScattering->bind();
	mAtmosphericScattering->setInverseProjection(glm::inverse(camera.getProjectionMatrix()));
	mAtmosphericScattering->setInverseViewRotation(glm::inverse(camera.getView()));
	mAtmosphericScattering->setStepCount(16);
	mAtmosphericScattering->setSurfaceHeight(0.99f);
	mAtmosphericScattering->setScatterStrength(0.028f);
	mAtmosphericScattering->setSpotBrightness(10.0f);
	mAtmosphericScattering->setViewport(constants.windowWidth, constants.windowHeight);

	const auto& view = camera.getView();
	const auto& prevView = camera.getPrevView();
	const auto& proj = camera.getProjectionMatrix();
	const auto prevViewProj = proj * prevView;
	const auto viewProjInverse = inverse(proj*view);

	mAtmosphericScattering->setPrevViewProj(prevViewProj);
	mAtmosphericScattering->setInvViewProj(viewProjInverse);


	AtmosphericScattering::Light light;
	light.direction = -normalize(sun.directionWorld);
	light.intensity = 1.8f;
	mAtmosphericScattering->setLight(light);

	AtmosphericScattering::Mie mie;
	mie.brightness = 0.1f;
	mie.collectionPower = 0.39f;
	mie.distribution = 0.63f;
	mie.strength = 0.264f;
	mAtmosphericScattering->setMie(mie);

	AtmosphericScattering::Rayleigh rayleigh;
	rayleigh.brightness = 3.3f;
	rayleigh.collectionPower = 0.81f;
	rayleigh.strength = 0.139f;
	mAtmosphericScattering->setRayleigh(rayleigh);

	mAtmosphericScattering->renderSky();
}

std::unique_ptr<nex::RenderTarget2D> nex::PBR_Deferred_Renderer::createLightingTarget(unsigned width, unsigned height, const PBR_GBuffer* gBuffer)
{
	auto result = std::make_unique<RenderTarget2D>(width, height, TextureDesc::createRenderTargetRGBAHDR());

	RenderAttachment luminance;
	luminance.colorAttachIndex = 1;
	luminance.target = TextureTarget::TEXTURE2D;
	luminance.type = RenderAttachmentType::COLOR;
	// TODO: use one color channel!
	luminance.texture = std::make_shared<Texture2D>(width, height, TextureDesc::createRenderTargetRGBAHDR(), nullptr);

	result->addColorAttachment(std::move(luminance));

	RenderAttachment motion = gBuffer->getMotionRenderTarget();
	motion.colorAttachIndex = 2;
	result->addColorAttachment(std::move(motion));

	RenderAttachment depth = gBuffer->getNormalizedViewSpaceZRenderTarget();
	depth.colorAttachIndex = 3;
	result->addColorAttachment(std::move(depth));

	result->useDepthAttachment(*gBuffer->getDepthAttachment());

	result->finalizeAttachments();

	return result;
}

nex::PBR_Deferred_Renderer_ConfigurationView::PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer) : mRenderer(renderer), mTesselationConfig(mRenderer->getTesselationTest()),
mOceanConfig(mRenderer->getOcean())
{
}

void nex::PBR_Deferred_Renderer_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(mId.c_str());

	AmbientOcclusionSelector* aoSelector = mRenderer->getAOSelector();
	bool useAmbientOcclusion = aoSelector->isAmbientOcclusionActive();

	if (ImGui::Checkbox("Ambient occlusion", &useAmbientOcclusion))
	{
		aoSelector->setUseAmbientOcclusion(useAmbientOcclusion);
	}

	if (useAmbientOcclusion)
	{
		std::stringstream ss;
		ss << AOTechnique::HBAO;
		std::string hbaoText = ss.str();

		ss.str("");
		ss << AOTechnique::SSAO;
		std::string ssaoText = ss.str();

		const char* items[] = { hbaoText.c_str(), ssaoText.c_str() };
		nex::AOTechnique selectedTechnique = aoSelector->getActiveAOTechnique();

		ImGui::SameLine(0, 70);
		if (ImGui::Combo("AO technique", (int*)&selectedTechnique, items, IM_ARRAYSIZE(items)))
		{
			std::cout << selectedTechnique << " is selected!" << std::endl;
			aoSelector->setAOTechniqueToUse(selectedTechnique);
		}
	}

	const auto& layerDescs = mRenderer->getRenderLayerDescriptions();

	size_t size = 0;
	for (const auto& item : layerDescs) {
		size += item.size() + 1;
	}

	size_t cursor = 0;
	std::vector<char> flatDesc(size);
	for (const auto& item : layerDescs) {
		memcpy(flatDesc.data() + cursor, item.data(), item.size());
		cursor += item.size();
		flatDesc[cursor] = '\0';
		++cursor;
	}

	if (ImGui::Combo("Render layer", &mSelectedRenderLayer, flatDesc.data())) {
		mRenderer->setActiveRenderLayer(layerDescs[mSelectedRenderLayer]);
	}

	/*nex::gui::Separator(2.0f);
	mTesselationConfig.drawGUI();

	nex::gui::Separator(2.0f);
	ImGui::Text("Ocean:");
	mOceanConfig.drawGUI();*/

	nex::gui::Separator(2.0f);

	ImGui::PopID();
}