#include <techniques/PBR_Deferred_Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/shader/SkyBoxPass.hpp>
#include <nex/Scene.hpp>
#include <nex/shader/DepthMapPass.hpp>
#include <nex/shader/ScreenPass.hpp>
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
}


void nex::PBR_Deferred_Renderer::render(const RenderCommandQueue& queue, 
	const Camera& camera,
	const DirectionalLight& sun, 
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


	static auto* depthTest = RenderBackend::get()->getDepthBuffer();
	depthTest->enableDepthBufferWriting(true);

	static bool switcher = true;
	if (mInput->isPressed(Input::KEY_O))
	{
		switcher = !switcher;
	}

	if (switcher)
	{
		renderDeferred(queue, camera, sun, windowWidth, windowHeight);
	}
	else
	{
		renderForward(queue, camera, sun, windowWidth, windowHeight);
	}

	auto* stencilTest = mRenderBackend->getStencilTest();

	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->enableStencilTest(true);

	//mTesselationTest.draw(camera, sun->getDirection());
	//static float simulationTime = 0.0f;
	//simulationTime += frameTime;

	//mOcean.simulate(simulationTime * 0.5f);
	//mOcean.draw(camera, sun->getDirection());


	stencilTest->setCompareFunc(CompareFunction::NOT_EQUAL, 1, 1);

	renderSky(camera, sun, windowWidth, windowHeight);
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
	forward->configurePass(camera);
	forward->updateLight(sun, camera);
	StaticMeshDrawer::draw(queue.getTransparentCommands());

	// At last we render tools
	StaticMeshDrawer::draw(queue.getToolCommands());


	Texture2D* outputTexture = colorTex;

	if (postProcess) {
		outputTexture = (Texture2D*)postProcessor->doPostProcessing(colorTex, luminanceTexture, aoMap, motionTexture, mPingPong.get()); //mPbrMrt->getMotion()
	}
	//auto backup = *mPingPong->getDepthAttachment();
	//mPingPong->useDepthAttachment(*mRenderTargetSingleSampled->getDepthAttachment());
	//mPingPong->useDepthAttachment(std::move(backup));

	postProcessor->antialias(outputTexture, out);
	//postProcessor->antialias((Texture2D*)colorTex, out);


	depthTest->enableDepthBufferWriting(true);
	//ShaderStorageBuffer::syncWithGPU();
}

void nex::PBR_Deferred_Renderer::setShowDepthMap(bool showDepthMap)
{
	this->mShowDepthMap = showDepthMap;
}

void nex::PBR_Deferred_Renderer::updateRenderTargets(unsigned width, unsigned height)
{
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

nex::TesselationTest* nex::PBR_Deferred_Renderer::getTesselationTest()
{
	return &mTesselationTest;
}

nex::Ocean* nex::PBR_Deferred_Renderer::getOcean()
{
	return &mOcean;
}

void nex::PBR_Deferred_Renderer::renderShadows(const nex::RenderCommandQueue::Buffer& shadowCommands, 
	const Camera&  camera, const DirectionalLight& sun,
	Texture2D* depth)
{
	if (mCascadedShadow->isEnabled())
	{
		mCascadedShadow->useTightNearFarPlane(false);
		mCascadedShadow->frameUpdate(camera, sun.getDirection(), depth);
		TransformPass* depthPass = mCascadedShadow->getDepthPass();
		depthPass->bind();
		depthPass->updateConstants(camera);

		for (unsigned i = 0; i < mCascadedShadow->getCascadeData().numCascades; ++i)
		{
			mCascadedShadow->begin(i);
			for (const auto& command : shadowCommands)
			{
				depthPass->setModelMatrix(command.worldTrafo, command.prevWorldTrafo);
				depthPass->uploadTransformMatrices();
				StaticMeshDrawer::draw(command.mesh, nullptr, depthPass);
			}
		}

		mCascadedShadow->frameReset();
	}
}

void nex::PBR_Deferred_Renderer::renderDeferred(const RenderCommandQueue& queue, 
	const Camera&  camera, const DirectionalLight& sun,
	unsigned windowWidth,
	unsigned windowHeight)
{
	static auto* stencilTest = RenderBackend::get()->getStencilTest();
	static auto* depthTest = RenderBackend::get()->getDepthBuffer();

	using namespace std::chrono;


	// update and render into cascades
	mPbrMrt->bind();


	mRenderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
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
		technique->getActiveSubMeshPass()->updateConstants(camera);
	}
	StaticMeshDrawer::draw(queue.getDeferrablePbrCommands());

	stencilTest->enableStencilTest(false);
	//glm::vec2 minMaxPositiveZ(0.0f, 1.0f);

	//minMaxPositiveZ.x = camera->getFrustum(Perspective).nearPlane;
	//minMaxPositiveZ.y = camera->getFrustum(Perspective).farPlane;

	renderShadows(queue.getShadowCommands(), camera, sun, (Texture2D*)mPbrMrt->getDepthAttachment()->texture.get()); //mPbrMrt->getNormalizedViewSpaceZ()


	// render scene to a offscreen buffer
	mRenderTargetSingleSampled->bind();
	mRenderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);

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
	deferred->drawLighting(mPbrMrt.get(), camera, sun);

	//stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 1);

	depthTest->enableDepthTest(true);
	depthTest->enableDepthBufferWriting(true);
	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->setOperations(StencilTest::Operation::REPLACE, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);


	mPbrTechnique->useForward();
	auto* forward = mPbrTechnique->getForward();
	forward->configurePass(camera);
	forward->updateLight(sun, camera);

	StaticMeshDrawer::draw(queue.getForwardCommands());
	StaticMeshDrawer::draw(queue.getProbeCommands());

	stencilTest->setCompareFunc(CompareFunction::EQUAL, 1, 1);
}

void nex::PBR_Deferred_Renderer::renderForward(const RenderCommandQueue& queue,
	const Camera&  camera, const DirectionalLight& sun,
	unsigned windowWidth,
	unsigned windowHeight)
{
	static auto* stencilTest = RenderBackend::get()->getStencilTest();

	RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);
	RenderBackend::get()->getDepthBuffer()->enableDepthBufferWriting(true);
	//RenderBackend::get()->getDepthBuffer()->enableDepthClamp(true);


	// update and render into cascades
	mRenderTargetSingleSampled->bind();


	mRenderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	//renderer->beginScene();

	mRenderTargetSingleSampled->clear(RenderComponent::Depth); //RenderComponent::Color |


	stencilTest->enableStencilTest(false);
	auto* depthPass = mRenderBackend->getEffectLibrary()->getDepthMapShader();
	//mCommandQueue.getDeferredCommands()  mCommandQueue.getShadowCommands()
	depthPass->bind();
	depthPass->updateViewProjection(camera.getProjectionMatrix(), camera.getView());
	StaticMeshDrawer::draw(queue.getShadowCommands(), depthPass);
	renderShadows(queue.getShadowCommands(), camera, sun, (Texture2D*)mRenderTargetSingleSampled->getDepthAttachment()->texture.get());


	// render scene to a offscreen buffer
	mRenderTargetSingleSampled->bind();

	mRenderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	mRenderTargetSingleSampled->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);//RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil


	stencilTest->enableStencilTest(true);
	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

	mPbrTechnique->useForward();
	auto* forward = mPbrTechnique->getForward();
	forward->configurePass(camera);
	forward->updateLight(sun, camera);

	//mPbrForward->configureSubMeshPass(camera);
	//mPbrForward->getActiveSubMeshPass()->updateConstants(camera);

	for (nex::Technique* technique : queue.getTechniques())
	{
		//technique->configureSubMeshPass(camera);
		technique->getActiveSubMeshPass()->setViewProjectionMatrices(camera.getProjectionMatrix(), camera.getView(), camera.getPrevView());
		technique->getActiveSubMeshPass()->updateConstants(camera);
	}

	StaticMeshDrawer::draw(queue.getDeferrablePbrCommands()); //TODO
	StaticMeshDrawer::draw(queue.getForwardCommands());
	StaticMeshDrawer::draw(queue.getProbeCommands());
}

void nex::PBR_Deferred_Renderer::renderSky(const Camera& camera, const DirectionalLight& sun, unsigned width, unsigned height)
{
	mAtmosphericScattering->bind();
	mAtmosphericScattering->setInverseProjection(glm::inverse(camera.getProjectionMatrix()));
	mAtmosphericScattering->setInverseViewRotation(glm::inverse(camera.getView()));
	mAtmosphericScattering->setStepCount(16);
	mAtmosphericScattering->setSurfaceHeight(0.99f);
	mAtmosphericScattering->setScatterStrength(0.028f);
	mAtmosphericScattering->setSpotBrightness(10.0f);
	mAtmosphericScattering->setViewport(width, height);

	const auto& view = camera.getView();
	const auto& prevView = camera.getPrevView();
	const auto& proj = camera.getProjectionMatrix();
	const auto prevViewProj = proj * prevView;
	const auto viewProjInverse = inverse(proj*view);

	mAtmosphericScattering->setPrevViewProj(prevViewProj);
	mAtmosphericScattering->setInvViewProj(viewProjInverse);


	AtmosphericScattering::Light light;
	light.direction = -normalize(sun.getDirection());
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
	auto result = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR());

	RenderAttachment luminance;
	luminance.colorAttachIndex = 1;
	luminance.target = TextureTarget::TEXTURE2D;
	luminance.type = RenderAttachmentType::COLOR;
	// TODO: use one color channel!
	luminance.texture = std::make_shared<Texture2D>(width, height, TextureData::createRenderTargetRGBAHDR(), nullptr);

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
	ImGui::PushID(m_id.c_str());

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

	nex::gui::Separator(2.0f);
	mTesselationConfig.drawGUI();

	nex::gui::Separator(2.0f);
	ImGui::Text("Ocean:");
	mOceanConfig.drawGUI();

	ImGui::PopID();
}