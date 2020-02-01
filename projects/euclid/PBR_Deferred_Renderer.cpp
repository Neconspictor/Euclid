#include <PBR_Deferred_Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/effects/SkyBoxPass.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/effects/DepthMapPass.hpp>
#include <nex/effects/SpriteShader.hpp>
#include <nex/texture/TextureManager.hpp>
//#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/texture/GBuffer.hpp>
#include "nex/shadow/CascadedShadow.hpp"
#include <nex/renderer/Drawer.hpp>
#include "nex/renderer/RenderBackend.hpp"

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/string_cast.hpp>

#include  <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include "nex/texture/Attachment.hpp"
#include "nex/post_processing/PostProcessor.hpp"
#include "imgui/imgui.h"
#include <nex/pbr/PbrDeferred.hpp>
#include <nex/GI/Probe.hpp>
#include "nex/sky/AtmosphericScattering.hpp"
#include <nex/texture/Sampler.hpp>
#include "nex/pbr/PbrForward.hpp"
#include "nex/camera/FPCamera.hpp"
#include <nex/platform/Input.hpp>
#include "nex/gui/ImGUI_Extension.hpp"
#include <unordered_set>
#include <nex/pbr/Pbr.hpp>
#include <nex/cluster/Cluster.hpp>
#include <nex/GI/GlobalIllumination.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/post_processing/PostProcessor.hpp>
#include <nex/post_processing/SMAA.hpp>
#include <nex/post_processing/HBAO.hpp>
#include <nex/post_processing/FXAA.hpp>
#include <nex/post_processing/TAA.hpp>
#include <nex/post_processing/DownSampler.hpp>
#include <nex/effects/Blit.hpp>
#include <nex/post_processing/SSR.hpp>
#include <nex/light/Light.hpp>
#include <nex/effects/ViewSpaceZSpriteShader.hpp>

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
	mOutRT(nullptr),
	//shadowMap(nullptr),

	mShowDepthMap(false),
	mAtmosphericScattering(std::make_unique<AtmosphericScattering>()),
	mInput(input),
	mCascadedShadow(cascadedShadow),
	mRenderBackend(backend),
	mAntialiasIrradiance(true),
	mBlurIrradiance(false),
	mRenderGIinHalfRes(true),
	mUseDownSampledDepth(false),
	mActiveIrradianceRT(0),
	mOutSwitcherTAA(nullptr, 0, nullptr, nullptr)
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

bool nex::PBR_Deferred_Renderer::getIrradianceAA() const
{
	return mAntialiasIrradiance;
}

bool nex::PBR_Deferred_Renderer::getBlurIrradiance() const
{
	return mBlurIrradiance;
}

bool nex::PBR_Deferred_Renderer::getRenderGIinHalfRes() const
{
	return mRenderGIinHalfRes;
}

bool nex::PBR_Deferred_Renderer::getDownSampledDepth() const
{
	return mUseDownSampledDepth;
}

void nex::PBR_Deferred_Renderer::init(int windowWidth, int windowHeight)
{
	LOG(m_logger, LogLevel::Info) << "PBR_Deferred_Renderer::init called!";

	auto* effectLib = mRenderBackend->getEffectLibrary();

	updateRenderTargets(windowWidth, windowHeight);

	blurEffect = mRenderBackend->getEffectLibrary()->getGaussianBlur();

	mRenderBackend->getRasterizer()->enableScissorTest(false);


	auto* lib = mRenderBackend->getEffectLibrary();
	auto* mAoSelector = lib->getPostProcessor()->getAOSelector();
	mAoSelector->setUseAmbientOcclusion(true);
	mAoSelector->setAOTechniqueToUse(AOTechnique::HBAO);


	mRenderLayers.push_back({ "composited", [&]() { return mOutRT->getColorAttachmentTexture(0); },
	[lib = lib]() { return lib->getSpriteMultisampleDownsamplePass(); } });

	mRenderLayers.push_back({ "SSR UV", [&]() { return
		RenderBackend::get()->
		getEffectLibrary()->
		getPostProcessor()->
		getSSR()->
		getReflectionUV(); } });

	mRenderLayers.push_back({ "irradiance", [&]() { return mIrradianceAmbientReflectionRT[mActiveIrradianceRT]->getColorAttachmentTexture(0); },
	[lib = lib]() { return lib->getSpritePass(); } });

	mRenderLayers.push_back({ "irradiance - reflection", [&]() { return mIrradianceAmbientReflectionRT[mActiveIrradianceRT]->getColorAttachmentTexture(1); },
	[lib = lib]() { return lib->getSpritePass(); } });

	mRenderLayers.push_back({ "GBuffer: depth", [&]() { return mOutRT->getDepthAttachment()->texture.get(); },
	[lib = lib]() { return lib->getSpritePass(); } });



	mRenderLayers.push_back({ "HBAO - viewspace z", [&]() { 
		auto* hbao = getAOSelector()->getHBAO();
		return hbao->getViewSpaceZ();
		}, [&, lib = lib]() { 
			auto* shader = lib->getViewSpaceZSpritePass();
			auto* hbao = getAOSelector()->getHBAO();
			const auto& proj = hbao->getViewSpaceZProjectionInfo();
			shader->setCameraDistanceRange(proj.farplane - proj.nearplane);
			return shader;
		} });

	for (auto i = 0; i < HBAO_RANDOM_ELEMENTS; ++i) {

		mRenderLayers.push_back({ std::string("HBAO - depthview[") + std::to_string(i) + "]", 
			[=]() { return getAOSelector()->getHBAO()->getViewSpaceZ4thView(i); }, 
			[&, lib = lib]() {
			auto* shader = lib->getViewSpaceZSpritePass();
			auto* hbao = getAOSelector()->getHBAO();
			const auto& proj = hbao->getViewSpaceZProjectionInfo();
			shader->setCameraDistanceRange(proj.farplane - proj.nearplane);
			return shader;
		} });
	}

	for (auto i = 0; i < HBAO_RANDOM_ELEMENTS; ++i) {

		mRenderLayers.push_back({ std::string("HBAO - ao_result_view[") + std::to_string(i) + "]",
			[=]() { return getAOSelector()->getHBAO()->getAoResultView4th(i); }, 
			[lib = lib]() { return lib->getDepthSpritePass(); } });
	}

	mRenderLayers.push_back({ "post processed (without antialising)", [&]() { return mPingPong->getColorAttachmentTexture(0); } });
	mRenderLayers.push_back({ "GBuffer: normal - view space", [&]() { return  mPbrMrt->getNormal(); } });
	mRenderLayers.push_back({ "GBuffer: albedo", [&]() { return mPbrMrt->getAlbedo(); } });
	mRenderLayers.push_back({ "GBuffer: ambient occlusion, metalness, roughness", [&]() { return mPbrMrt->getAoMetalRoughness(); } });
	mRenderLayers.push_back({ "motion", [&]() { return mPbrMrt->getMotion(); } });
	mRenderLayers.push_back({ "luminance", [&]() { return mOutRT->getColorAttachmentTexture(1); } });
	mRenderLayers.push_back({ "ambient occlusion", [&]() { return getAOSelector()->getRenderResult(); }, 
		[lib = lib]() { return lib->getDepthSpritePass(); } });

	mRenderLayers.push_back({ "ambient occlusion - without blur", [&]() { return getAOSelector()->getHBAO()->getAO_Result(); }, 
		[lib = lib]() { return lib->getDepthSpritePass(); } });
	//mRenderLayers.push_back({ "pre-post process", [&]() { return mRenderTargetSingleSampled->getColorAttachmentTexture(0); } });
	
	mRenderLayers.push_back({ "SMAA - edge", []() { return RenderBackend::get()->
		getEffectLibrary()->
		getPostProcessor()->
		getSMAA()->
		getEdgeDetection(); } });

	mRenderLayers.push_back({ "SMAA - blend",[]() { return RenderBackend::get()->
		getEffectLibrary()->
		getPostProcessor()->
		getSMAA()->
		getBlendingWeight(); } });
	mRenderLayers.push_back({ "HBAO view space normals",[&]() { return getAOSelector()->getHBAO()->getViewSpaceNormals(); } });

	setActiveRenderLayer(getRenderLayerIndexByName("composited"));
	//setActiveRenderLayer(getRenderLayerIndexByName("ambient occlusion - without blur"));
	//setActiveRenderLayer(getRenderLayerIndexByName("HBAO - ao_result_view[0]"));
}


void nex::PBR_Deferred_Renderer::render(const RenderCommandQueue& queue, 
	const RenderContext& constants,
	bool postProcess,
	RenderTarget* out)
{
	const auto& camera = *constants.camera;
	const auto& sun = *constants.sun;
	const auto& width = constants.windowWidth;
	const auto& height = constants.windowHeight;

	auto* lib = mRenderBackend->getEffectLibrary();
	auto* postProcessor = lib->getPostProcessor();
	auto* taa = postProcessor->getTAA();


	static auto* depthTest = RenderBackend::get()->getDepthBuffer();
	depthTest->enableDepthBufferWriting(true);

	//mOutRT->enableDrawToColorAttachment(2, true);


	//mOutSwitcherTAA.switchTexture();
	//auto currentOut = mOutSwitcherTAA.getActiveTexture();
	//auto previousOut = mOutSwitcherTAA.getNonActiveTexture();

	auto invViewProj = inverse(camera.getProjectionMatrix() * camera.getView());


	static bool switcher = true;
	if (mInput->isPressed(Input::KEY_O))
	{
		switcher = !switcher;
	}

	//TODO update forward renderer!
	//if (switcher)
	//{
		//renderDeferred(queue, constants, sun);
	//}
	//else
	//{
		renderForward(queue, constants, sun);
	//}

		auto* stencilTest = mRenderBackend->getStencilTest();
		Texture2D* aoMap = nullptr;

	if (postProcess) {
		auto* depthTexture2 = static_cast<Texture2D*>(mOutRT->getDepthAttachment()->texture.get());
		aoMap = postProcessor->getAOSelector()->renderAO(camera, depthTexture2); //mPbrMrt->getNormalizedViewSpaceZ()

		

		mOutRT->bind();
		RenderBackend::get()->setViewPort(0, 0, mOutRT->getWidth(), mOutRT->getHeight());
		stencilTest->enableStencilTest(false);
		postProcessor->renderAO(aoMap);

		//stencilTest->enableStencilTest(true);

		stencilTest->setCompareFunc(CompFunc::NOT_EQUAL, 1, 1);
	}

	

	renderSky(constants, sun);//TODO
	//stencilTest->enableStencilTest(false);


	auto* globalIllumination = mPbrTechnique->getDeferred()->getGlobalIllumination();
	
	Drawer::draw(queue.getBeforeTransparentCommands(), constants, {});

	if (false) {
		// After sky we render transparent objects
		stencilTest->enableStencilTest(true);
		stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
		mOutRT->bind();
		auto* forward = mPbrTechnique->getForward();
		forward->configurePass(constants);
		forward->updateLight(sun, camera);
		Drawer::draw(queue.getTransparentCommands(), constants, {});
		stencilTest->enableStencilTest(false);
	}
	

	// At last we render tools and probes
	Drawer::draw(queue.getProbeCommands(), constants, {});
	//Drawer::draw(queue.getToolCommands(), constants, {});



	auto* colorTex = static_cast<Texture2D*>(mOutRT->getColorAttachmentTexture(0));
	auto* luminanceTexture = static_cast<Texture2D*>(mOutRT->getColorAttachmentTexture(1));
	//auto* motionTexture = static_cast<Texture2D*>(mOutRT->getColorAttachmentTexture(2));
	//auto* depthTexture = static_cast<Texture2D*>(mOutRT->getColorAttachmentTexture(3));
	auto* depthTexture = static_cast<Texture2D*>(mOutRT->getDepthAttachment()->texture.get());
	


	// instead of clearing the buffer we just disable depth and stencil tests for improved performance
	RenderBackend::get()->getDepthBuffer()->enableDepthTest(false);
	RenderBackend::get()->getStencilTest()->enableStencilTest(false);

	// finally render the offscreen buffer to a quad and do post processing stuff


	mOutRT->enableDrawToColorAttachment(1, false);
	mOutRT->enableDrawToColorAttachment(2, false);
	mOutRT->enableDrawToColorAttachment(3, false);

	Texture2D* outputTexture = colorTex;

	if (postProcess) {

		auto bloomTextures = postProcessor->computeBloom(colorTex, luminanceTexture);

		mPingPong->bind();
		RenderBackend::get()->setViewPort(0, 0, mPingPong->getWidth(), mPingPong->getHeight());

		postProcessor->doPostProcessing(colorTex,
			luminanceTexture,
			aoMap,
			colorTex,
			bloomTextures);

		outputTexture = (Texture2D*)mPingPong->getColorAttachmentTexture(0);
	}

	// At last we render tools and probes
	//mPingPong->bind();
	//auto attachment = *mPingPong->getDepthAttachment();
	//auto copyAttach = attachment;
	//copyAttach.texture = mOutRT->getDepthAttachment()->texture;
	//mPingPong->useDepthAttachment(copyAttach);
	//RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);
	//mOutRT->enableDrawToColorAttachment(0, true);
	//Drawer::draw(queue.getProbeCommands(), constants, {});
	Drawer::draw(queue.getToolCommands(), constants, {});
	//RenderBackend::get()->getDepthBuffer()->enableDepthTest(false);
	//mPingPong->useDepthAttachment(attachment);

	if (postProcess) {
		if (true) {

			postProcessor->antialias(outputTexture, mOutRT.get());
		}
		else {
			mOutRT->bind();
			postProcessor->getFXAA()->antialias(outputTexture, true);
		}
	}

	

	if (false) {

		

		mPingPong->bind();
		mRenderBackend->setViewPort(0,0, width, height);
		//taa->antialias(currentOut, previousOut, depthTexture, motionTexture, camera);

		for (int i = 0; i < 4; ++i)
			mOutRT->enableDrawToColorAttachment(i, false);
		mOutRT->enableDrawToColorAttachment(0, true);
		mPingPong->blit(mOutRT.get(), { 0,0, mOutRT->getWidth(), mOutRT->getHeight() }, RenderComponent::Color);
		for (int i = 0; i < 4; ++i)
			mOutRT->enableDrawToColorAttachment(i, true);

		/*auto testTex = mPingPong->getColorAttachments()[0].texture;//mOutSwitcherTAA.getTextures()[0];
		mOutRT->getColorAttachments()[0].texture = testTex;
		mOutRT->updateColorAttachment(0);*/

	}

	if (out) {
		mOutRT->enableDrawToColorAttachments(false);
		mOutRT->enableDrawToColorAttachment(0, true);
		mOutRT->blit(out, { 0,0, mOutRT->getWidth(), mOutRT->getHeight() }, RenderComponent::Color);
		mOutRT->enableDrawToColorAttachments(true);
	}

	mOutRT->enableDrawToColorAttachment(1, true);
	mOutRT->enableDrawToColorAttachment(2, true);
	mOutRT->enableDrawToColorAttachment(3, true);

	

	depthTest->enableDepthBufferWriting(true);

	mActiveIrradianceRT = !mActiveIrradianceRT;
}

void nex::PBR_Deferred_Renderer::setShowDepthMap(bool showDepthMap)
{
	this->mShowDepthMap = showDepthMap;
}

void nex::PBR_Deferred_Renderer::setIrradianceAA(bool antialias)
{
	mAntialiasIrradiance = antialias;
}

void nex::PBR_Deferred_Renderer::setBlurIrradiance(bool value)
{
	mBlurIrradiance = value;
}

void nex::PBR_Deferred_Renderer::setRenderGIinHalfRes(bool value)
{
	mRenderGIinHalfRes = value;
}

void nex::PBR_Deferred_Renderer::setDownsampledDepth(bool value)
{
	mUseDownSampledDepth = value;
}

void nex::PBR_Deferred_Renderer::updateRenderTargets(unsigned width, unsigned height)
{
	Renderer::updateRenderTargets(width, height);
	//update render target dimension
	//the render target dimensions are dependent from the viewport size
	// so first update the viewport and than recreate the render targets
	mRenderBackend->resize(width, height);

	mPbrMrt = mPbrTechnique->getDeferred()->createMultipleRenderTarget(width, height);
	mOutRT = createLightingTarget(width, height, mPbrMrt.get());
	//mOutRT = mRenderBackend->createRenderTarget(1);

	//auto outSwitchTexture = std::make_shared<Texture2D>(width, height, TextureDesc::createRenderTargetRGBAHDR(), nullptr);
	//mOutSwitcherTAA.setTextures(mOutRT->getColorAttachments()[0].texture,
	//	outSwitchTexture, false);
	//mOutSwitcherTAA.setTarget(mOutRT.get(), true);

	mPingPong = mRenderBackend->createRenderTarget();


	RenderAttachment luminance;
	luminance.colorAttachIndex = 1;
	luminance.target = TextureTarget::TEXTURE2D;
	luminance.type = RenderAttachmentType::COLOR;
	// TODO: use one color channel!
	luminance.texture = std::make_shared<Texture2D>(width, height, TextureDesc::createRenderTargetRGBAHDR(), nullptr);



	mPingPong->addColorAttachment(std::move(luminance));
	mPingPong->finalizeAttachments();
	mPingPong->enableDrawToColorAttachment(1, false);

	auto* pingPongDepthStencilTex = mPingPong->getDepthAttachment()->texture.get();
	auto* outStencilTex = mOutRT->getDepthAttachment()->texture.get();
	auto depthStencilDesc = pingPongDepthStencilTex->getTextureData();
	depthStencilDesc.depthStencilTextureMode = DepthStencilTexMode::STENCIL;
	//mPingPongStencilView = Texture::createView(pingPongDepthStencilTex, TextureTarget::TEXTURE2D, 0, 1, 0, 1, depthStencilDesc);
	//mOutStencilView = Texture::createView(outStencilTex, TextureTarget::TEXTURE2D, 0, 1, 0, 1, depthStencilDesc); //TODO
	
	const unsigned giWidth = mRenderGIinHalfRes ? width / 2 : width;
	const unsigned giHeight = mRenderGIinHalfRes ? height / 2 : height;

	RenderAttachment attachment;

	for (int i = 0; i < 2; ++i) {
		mIrradianceAmbientReflectionRT[i] = std::make_unique<RenderTarget>(giWidth, giHeight);


		TextureDesc desc;
		desc.internalFormat = InternalFormat::RGBA16;

		mPingPongHalf = std::make_unique<RenderTarget2D>(giWidth, giHeight, desc);

		attachment.colorAttachIndex = 0;
		attachment.texture = std::make_shared<Texture2D>(giWidth, giHeight, desc, nullptr);
		mIrradianceAmbientReflectionRT[i]->addColorAttachment(attachment);

		attachment.colorAttachIndex = 1;
		attachment.texture = std::make_shared<Texture2D>(giWidth, giHeight, desc, nullptr);
		mIrradianceAmbientReflectionRT[i]->addColorAttachment(attachment);
		mIrradianceAmbientReflectionRT[i]->finalizeAttachments();
	}

	


	mDepthHalf = std::make_unique<RenderTarget>(width / 2, height / 2);
	TextureDesc depthHalfDesc;
	depthHalfDesc.internalFormat = InternalFormat::R32F;

	attachment.texture = std::make_shared<Texture2D>(width / 2, height / 2, depthHalfDesc, nullptr);
	attachment.colorAttachIndex = 3;
	mDepthHalf->addColorAttachment(attachment);
	mDepthHalf->finalizeAttachments();	
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

nex::CascadedShadow* nex::PBR_Deferred_Renderer::getCascadedShadow()
{
	return mCascadedShadow;
}

void nex::PBR_Deferred_Renderer::pushDepthFunc(std::function<void()> func)
{
	mDepthFuncs.emplace_back(std::move(func));
}

nex::RenderTarget* nex::PBR_Deferred_Renderer::getActiveIrradianceAmbientReflectionRT()
{
	return mIrradianceAmbientReflectionRT[mActiveIrradianceRT].get();
}

nex::RenderTarget* nex::PBR_Deferred_Renderer::getOutRT()
{
	return mOutRT.get();
}

const nex::Texture* nex::PBR_Deferred_Renderer::getOutStencilView()
{
	return mOutStencilView.get();
}

nex::RenderTarget* nex::PBR_Deferred_Renderer::getPingPongRT()
{
	return mPingPong.get();
}

const nex::Texture* nex::PBR_Deferred_Renderer::getPingPongStencilView()
{
	return mPingPongStencilView.get();
}

void nex::PBR_Deferred_Renderer::renderShadows(const nex::RenderCommandQueue::Buffer& shadowCommands, 
	const RenderContext& constants, const DirLight& sun, Texture2D* depth)
{
	if (mCascadedShadow->isEnabled())
	{
		mCascadedShadow->useTightNearFarPlane(false);
		mCascadedShadow->frameUpdate(*constants.camera, sun.directionWorld, depth);
		mCascadedShadow->render(shadowCommands, constants);
		mCascadedShadow->frameReset();
	}
}

void nex::PBR_Deferred_Renderer::renderDeferred(const RenderCommandQueue& queue, 
	const RenderContext& constants, const DirLight& sun)
{
	static auto* stencilTest = RenderBackend::get()->getStencilTest();
	static auto* depthTest = RenderBackend::get()->getDepthBuffer();

	auto* lib = mRenderBackend->getEffectLibrary();
	auto* postProcessor = lib->getPostProcessor();
	auto* activeIrradiance = mIrradianceAmbientReflectionRT[mActiveIrradianceRT].get();
	auto* historyIrradiance = mIrradianceAmbientReflectionRT[!mActiveIrradianceRT].get();
	auto* ssr = lib->getPostProcessor()->getSSR();
	

	//using namespace std::chrono;

	const auto& camera = *constants.camera;
	const auto& proj = camera.getProjectionMatrix();
	const auto invProj = inverse(proj);

	// update and render into cascades
	mPbrMrt->bind();


	mRenderBackend->setViewPort(0, 0, constants.windowWidth * ssaaSamples, constants.windowHeight * ssaaSamples);

	mPbrMrt->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil); //RenderComponent::Color |


	//stencilTest->enableStencilTest(true);
	//stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
	//stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

	//mPbrTechnique->getDeferred()->configureGeometryPass(camera);
	const auto& state = RenderState::getDefault();
	//state.doCullFaces = false;
	//state.fillMode = FillMode::LINE;


	Drawer::draw(queue.getDeferrablePbrCommands(), constants, {}, &state);

	for (auto& func : mDepthFuncs) {
		func();
	}
	mDepthFuncs.clear();

	//stencilTest->enableStencilTest(false);
	//glm::vec2 minMaxPositiveZ(0.0f, 1.0f);

	//minMaxPositiveZ.x = camera->getFrustum(Perspective).nearPlane;
	//minMaxPositiveZ.y = camera->getFrustum(Perspective).farPlane;

	auto* gBufferDepth = (Texture2D*)mPbrMrt->getDepthAttachment()->texture.get();
	renderShadows(queue.getShadowCommands(), constants, sun, gBufferDepth); //mPbrMrt->getNormalizedViewSpaceZ()


	auto* globalIllumination = mPbrTechnique->getDeferred()->getGlobalIllumination(); 

	if (globalIllumination && false) {

		auto* probeManager = globalIllumination->getProbeManager();

		auto* probeCluster = probeManager->getProbeCluster();

		probeCluster->generateCluster(glm::uvec4(16, 8, 4, 6), gBufferDepth, &camera, nullptr);

		auto* envLightCuller = probeCluster->getEnvLightCuller();
		auto* envLightsBuffer = probeManager->getEnvironmentLightShaderBuffer();
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
	
	auto* deferred = mPbrTechnique->getDeferred();
	
	if (true && globalIllumination) {

		

		Texture* depth = mPbrMrt->getDepth();
		
		if (mRenderGIinHalfRes && mUseDownSampledDepth) {
			depth = lib->getDownSampler()->downsampleDepthHalf(mPbrMrt->getDepth(), mDepthHalf.get());
		}

		activeIrradiance->bind();
		mRenderBackend->setViewPort(0, 0, activeIrradiance->getWidth(), activeIrradiance->getHeight());
		activeIrradiance->clear(RenderComponent::Color);
		deferred->drawAmbientLighting(mPbrMrt.get(), depth, constants);

		


		if (mAntialiasIrradiance) {
			mPingPongHalf->bind();
			postProcessor->getFXAA()->antialias(activeIrradiance->getColorAttachmentTexture(0), false);
			activeIrradiance->enableDrawToColorAttachment(1, false);
			mPingPongHalf->blit(activeIrradiance,
				{ 0,0,activeIrradiance->getWidth(), activeIrradiance->getHeight() }, RenderComponent::Color);

			activeIrradiance->enableDrawToColorAttachment(1, true);
			activeIrradiance->enableDrawToColorAttachment(0, false);
			postProcessor->getFXAA()->antialias(activeIrradiance->getColorAttachmentTexture(1), false);
			mPingPongHalf->blit(activeIrradiance,
				{ 0,0,activeIrradiance->getWidth(), activeIrradiance->getHeight() }, RenderComponent::Color);
			activeIrradiance->enableDrawToColorAttachment(0, true);


			/*mPingPongHalf->bind();
			//mPingPongHalf->clear(RenderComponent::Color);
			postProcessor->getTAA()->antialias(	activeIrradiance->getColorAttachmentTexture(0), 
												historyIrradiance->getColorAttachmentTexture(0), 
												depth, camera);

			activeIrradiance->enableDrawToColorAttachment(1, false);
			activeIrradiance->enableDrawToColorAttachment(0, true);

			
			mPingPongHalf->blit(activeIrradiance,
				{ 0,0,activeIrradiance->getWidth(), activeIrradiance->getHeight() }, RenderComponent::Color);
			activeIrradiance->enableDrawToColorAttachment(1, true);


			postProcessor->getTAA()->antialias(activeIrradiance->getColorAttachmentTexture(1),
				historyIrradiance->getColorAttachmentTexture(1),
				depth, camera);

			activeIrradiance->enableDrawToColorAttachment(0, false);
			activeIrradiance->enableDrawToColorAttachment(1, true);


			mPingPongHalf->blit(activeIrradiance,
				{ 0,0,activeIrradiance->getWidth(), activeIrradiance->getHeight() }, RenderComponent::Color);
			activeIrradiance->enableDrawToColorAttachment(0, true);*/
		}

		

		if (mBlurIrradiance) {
			auto* blurer = mRenderBackend->getEffectLibrary()->getGaussianBlur();
			blurer->blur((Texture2D*)activeIrradiance->getColorAttachmentTexture(0),
				activeIrradiance, mPingPongHalf.get());

			auto backup = activeIrradiance->getColorAttachments()[0].texture;
			activeIrradiance->getColorAttachments()[0].texture =
				activeIrradiance->getColorAttachments()[1].texture;
			activeIrradiance->updateColorAttachment(0);

			blurer->blur((Texture2D*)activeIrradiance->getColorAttachmentTexture(0),
				activeIrradiance, mPingPongHalf.get());


			activeIrradiance->getColorAttachments()[1].texture =
				activeIrradiance->getColorAttachments()[0].texture;
			activeIrradiance->updateColorAttachment(1);

			activeIrradiance->getColorAttachments()[0].texture = backup;
			activeIrradiance->updateColorAttachment(0);
		}
		


	}


	// render scene to a offscreen buffer
	mOutRT->bind();
	mRenderBackend->setViewPort(0, 0, constants.windowWidth * ssaaSamples, constants.windowHeight * ssaaSamples);

	//mOutRT->enableDrawToColorAttachments(true);
	mOutRT->enableDrawToColorAttachment(2, false); // we don't want to clear motion buffer
	mOutRT->enableDrawToColorAttachment(3, false); // we don't want to clear depth (for e.g. ambient occlusion)
	mOutRT->clear(RenderComponent::Color);//RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil
	mOutRT->enableDrawToColorAttachment(2, true);
	mOutRT->enableDrawToColorAttachment(3, true);



	
	depthTest->enableDepthTest(false);
	depthTest->enableDepthBufferWriting(false);
	//stencilTest->enableStencilTest(true);
	//stencilTest->setCompareFunc(CompFunc::EQUAL, 1, 1);

	deferred->drawLighting(mPbrMrt.get(), activeIrradiance->getColorAttachmentTexture(0),
		activeIrradiance->getColorAttachmentTexture(1), constants, sun);

	//stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 1);

	depthTest->enableDepthTest(true);
	depthTest->enableDepthBufferWriting(true);
	//stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
	//stencilTest->setOperations(StencilTest::Operation::REPLACE, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);


	//TODO!!!
	//mPbrTechnique->useForward();
	//auto* forward = mPbrTechnique->getForward();
	//forward->configurePass(constants);
	//forward->updateLight(sun, camera);

	//Drawer::draw(queue.getForwardCommands()); //TODO!!!!
	//Drawer::draw(queue.getProbeCommands());

	//stencilTest->setCompareFunc(CompFunc::EQUAL, 1, 1);


	//SSR TODO: generalize deferred - forward!
	bool renderSSR = false;
	if (renderSSR) {
		ssr->renderReflections(gBufferDepth,
			mPbrMrt->getNormal(),
			mPbrMrt->getAlbedo(), //mOutRT->getColorAttachmentTexture(0),
			proj,
			invProj,
			camera.getClipInfo());
	}
	
}

void nex::PBR_Deferred_Renderer::renderForward(const RenderCommandQueue& queue,
	const RenderContext& constants, const DirLight& sun)
{
	static auto* stencilTest = RenderBackend::get()->getStencilTest();
	static auto* depthTest = RenderBackend::get()->getDepthBuffer();

	auto* lib = mRenderBackend->getEffectLibrary();



	//using namespace std::chrono;

	const auto& camera = *constants.camera;
	const auto& proj = camera.getProjectionMatrix();
	const auto invProj = inverse(proj);

	// update and render into cascades
	mOutRT->bind();


	mRenderBackend->setViewPort(0, 0, constants.windowWidth * ssaaSamples, constants.windowHeight * ssaaSamples);

	mOutRT->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil); //RenderComponent::Color |


	//stencilTest->enableStencilTest(true);
	//stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
	//stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

	//mPbrTechnique->getDeferred()->configureGeometryPass(camera);
	auto state = RenderState::getDefault();
	//state.doCullFaces = false;
	//state.fillMode = FillMode::LINE;


	//Drawer::draw(queue.getDeferrablePbrCommands(), constants, {}, &state);

	for (auto& func : mDepthFuncs) {
		func();
	}
	mDepthFuncs.clear();

	//stencilTest->enableStencilTest(false);

	auto* gBufferDepth = (Texture2D*)mOutRT->getDepthAttachment()->texture.get();
	renderShadows(queue.getShadowCommands(), constants, sun, gBufferDepth);


	auto* forward = mPbrTechnique->getForward();
	auto* globalIllumination = forward->getGlobalIllumination();

	

	// render scene to a offscreen buffer
	mOutRT->bind();
	mRenderBackend->setViewPort(0, 0, constants.windowWidth * ssaaSamples, constants.windowHeight * ssaaSamples);

	//mOutRT->enableDrawToColorAttachments(true);
	//mOutRT->enableDrawToColorAttachment(2, false); // we don't want to clear motion buffer
	//mOutRT->enableDrawToColorAttachment(3, false); // we don't want to clear depth (for e.g. ambient occlusion)
	mOutRT->clear(RenderComponent::Color);//RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil
	//mOutRT->enableDrawToColorAttachment(2, true);
	//mOutRT->enableDrawToColorAttachment(3, true);





	state.doDepthTest = true;
	state.depthCompare = CompFunc::LESS_EQUAL;
	state.doDepthWrite = true;

	//depthTest->enableDepthTest(true);
	//depthTest->setDefaultDepthFunc(CompFunc::EQUAL); //Ensures no overdraw
	//depthTest->enableDepthBufferWriting(false); // We don't want to change the depth buffer
	//stencilTest->enableStencilTest(true);
	//stencilTest->setCompareFunc(CompFunc::EQUAL, 1, 1);

	forward->configurePass(constants);
	forward->updateLight(sun, camera);
	Shader* shader = forward->getShaderProvider()->getShader();
	ShaderOverride<Shader> overwrite;// = { shader , shader };
	overwrite.default = shader;
	overwrite.rigged = shader;

	Drawer::draw(queue.getDeferrablePbrCommands(), constants, overwrite, &state);

	//stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 1);

	//depthTest->setDefaultDepthFunc(CompFunc::LESS);
	//depthTest->enableDepthBufferWriting(true);
	//stencilTest->setCompareFunc(CompFunc::ALWAYS, 1, 0xFF);
	//stencilTest->setOperations(StencilTest::Operation::REPLACE, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);


	
	//Drawer::draw(queue.getProbeCommands());

	//stencilTest->setCompareFunc(CompFunc::EQUAL, 1, 1);
}

void nex::PBR_Deferred_Renderer::renderSky(const RenderContext& constants, const DirLight& sun)
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

	const auto prevViewProj = camera.getViewProjPrev();
	const auto viewProjInverse = inverse(camera.getViewProj());

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

std::unique_ptr<nex::RenderTarget> nex::PBR_Deferred_Renderer::createLightingTarget(unsigned width, unsigned height, const PBR_GBuffer* gBuffer)
{
	auto result = std::make_unique<RenderTarget>(width, height);

	RenderAttachment color;
	color.colorAttachIndex = 0;
	color.target = TextureTarget::TEXTURE2D_MULTISAMPLE;
	color.type = RenderAttachmentType::COLOR;
	color.texture = std::make_shared<Texture2DMultisample>(width, height, TextureDesc::createRenderTargetRGBAHDR(), 4);
	result->addColorAttachment(std::move(color));

	RenderAttachment luminance;
	luminance.colorAttachIndex = 1;
	luminance.target = TextureTarget::TEXTURE2D_MULTISAMPLE;
	luminance.type = RenderAttachmentType::COLOR;
	// TODO: use one color channel!
	luminance.texture = std::make_shared<Texture2DMultisample>(width, height, TextureDesc::createRenderTargetRGBAHDR(), 4);

	result->addColorAttachment(std::move(luminance));

	RenderAttachment motion = gBuffer->getMotionRenderTarget();
	motion.colorAttachIndex = 2;
	//result->addColorAttachment(std::move(motion));

	RenderAttachment depth = gBuffer->getDepthRenderTarget();
	//depth.colorAttachIndex = 3;
	depth.target = TextureTarget::TEXTURE2D_MULTISAMPLE;
	depth.type = RenderAttachmentType::DEPTH_STENCIL;

	depth.texture = std::make_shared<Texture2DMultisample>(width, height, TextureDesc::createDepth(CompFunc::LESS, InternalFormat::DEPTH32F_STENCIL8), 4);
	//result->addColorAttachment(std::move(depth));

	result->useDepthAttachment(std::move(depth));

	result->finalizeAttachments();

	return result;
}

nex::PBR_Deferred_Renderer_ConfigurationView::PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer) : mRenderer(renderer), mTesselationConfig(mRenderer->getTesselationTest())
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

		const char* items[] = { hbaoText.c_str()};
		nex::AOTechnique selectedTechnique = aoSelector->getActiveAOTechnique();

		ImGui::SameLine(0, 70);
		if (ImGui::Combo("AO technique", (int*)&selectedTechnique, items, IM_ARRAYSIZE(items)))
		{
			std::cout << selectedTechnique << " is selected!" << std::endl;
			aoSelector->setAOTechniqueToUse(selectedTechnique);
		}
	}

	bool irradianceAA = mRenderer->getIrradianceAA();
	if (ImGui::Checkbox("antialias GI", &irradianceAA)) {
		mRenderer->setIrradianceAA(irradianceAA);
	}

	bool blurIrradiance = mRenderer->getBlurIrradiance();
	if (ImGui::Checkbox("blur GI", &blurIrradiance)) {
		mRenderer->setBlurIrradiance(blurIrradiance);
	}

	bool renderInHalfRes = mRenderer->getRenderGIinHalfRes();
	if (ImGui::Checkbox("render GI in half resolution", &renderInHalfRes)) {
		mRenderer->setRenderGIinHalfRes(renderInHalfRes);
		auto* out = mRenderer->getOutRT();
		mRenderer->updateRenderTargets(out->getWidth(), out->getHeight());
		
	}

	bool useDownSampledDepth = mRenderer->getDownSampledDepth();
	if (ImGui::Checkbox("Downsample Depth for GI", &useDownSampledDepth)) {
		mRenderer->setDownsampledDepth(useDownSampledDepth);
	}

	const auto& layerDescs = mRenderer->getRenderLayers();

	mRenderer->getActiveRenderLayer();

	size_t size = 1;
	for (const auto& layer : layerDescs) {
		size += layer.desc.size() +1;
	}

	size_t cursor = 0;
	std::vector<char> flatDesc(size);
	for (const auto& layer : layerDescs) {
		memcpy(flatDesc.data() + cursor, layer.desc.data(), layer.desc.size());
		cursor += layer.desc.size();
		flatDesc[cursor] = '\0';
		++cursor;
	}

	flatDesc[cursor] = '\0'; // necessary to indicate that no more data follows!

	int index = static_cast<int>(mRenderer->getActiveRenderLayer());

	if (ImGui::Combo("Render layer", &index, flatDesc.data())) {
		mRenderer->setActiveRenderLayer(index);
	}

	/*nex::gui::Separator(2.0f);
	mTesselationConfig.drawGUI();
	*/

	nex::gui::Separator(2.0f);

	ImGui::PopID();
}