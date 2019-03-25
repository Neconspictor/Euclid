#include <pbr_deferred/PBR_Deferred_Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/camera/TrackballQuatCamera.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <nex/SceneNode.hpp>
#include <nex/shader/DepthMapShader.hpp>
#include <nex/shader/ScreenShader.hpp>
#include <nex/util/Math.hpp>
#include <nex/texture/TextureManager.hpp>
//#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include "nex/mesh/StaticMeshManager.hpp"
#include <nex/texture/GBuffer.hpp>
#include "nex/shadow/CascadedShadow.hpp"
#include <nex/drawing/StaticMeshDrawer.hpp>
#include "nex/RenderBackend.hpp"
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>

#include  <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/EffectLibrary.hpp>
#include "nex/texture/Attachment.hpp"
#include "nex/post_processing/PostProcessor.hpp"
#include "nex/post_processing/SMAA.hpp"
#include "imgui/imgui.h"
#include <nex/pbr/PbrDeferred.hpp>
#include "nex/pbr/PbrProbe.hpp"
#include "nex/sky/AtmosphericScattering.hpp"
#include <nex/texture/Sampler.hpp>
#include "nex/pbr/PbrForward.hpp"
#include <nex/shadow/SceneNearFarComputeShader.hpp>

int ssaaSamples = 1;

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
nex::PBR_Deferred_Renderer::PBR_Deferred_Renderer(nex::RenderBackend* backend, nex::Input* input) :
	Renderer(backend),
	blurEffect(nullptr),
	m_logger("PBR_Deferred_Renderer"),
	panoramaSky(nullptr),
	mRenderTargetSingleSampled(nullptr),
	//shadowMap(nullptr),
	mShowDepthMap(false),
	mInput(input),
	mAtmosphericScattering(std::make_unique<AtmosphericScattering>())
{
}


bool nex::PBR_Deferred_Renderer::getShowDepthMap() const
{
	return mShowDepthMap;
}

void nex::PBR_Deferred_Renderer::init(int windowWidth, int windowHeight)
{
	using namespace std::placeholders;


	LOG(m_logger, LogLevel::Info)<< "PBR_Deferred_Renderer::init called!";

	StaticMeshManager* modelManager = StaticMeshManager::get();
	TextureManager* textureManager = TextureManager::get();

	//auto rendererResizeCallback = bind(&Renderer::setViewPort, renderer, 0, 0, _1, _2);
	//window->addResizeCallback(rendererResizeCallback);


	modelManager->loadModels();

	//panoramaSky = textureManager->getHDRImage("skyboxes/panoramas/pisa.hdr", { false, true, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32 });
	//panoramaSky = textureManager->getImage("skyboxes/panoramas/pisa.hdr", { true, true, Linear_Mipmap_Linear, Linear, ClampToEdge });
	//panoramaSky = textureManager->getHDRImage("hdr/newport_loft.hdr", { false, true, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32 });
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
			false}
	);


	nex::TextureData data = {
				nex::TextureFilter::Linear_Mipmap_Linear,
				nex::TextureFilter::Linear,
				nex::TextureUVTechnique::Repeat,
				nex::TextureUVTechnique::Repeat,
				nex::TextureUVTechnique::Repeat,
				nex::ColorSpace::RGBA,
				nex::PixelDataType::UBYTE,
				nex::InternFormat::RGBA8,
				true };
	smaaTestTex = static_cast<Texture2D*>(textureManager->getImage("trash/Unigine01.png", data));
	smaaTestSRGBTex = static_cast<Texture2D*>(textureManager->getImage("trash/Unigine01.png"));

	testTexture = textureManager->getImage("container.png");

	//HDR_Free_City_Night_Lights_Ref

	//CubeMap* cubeMapSky = textureManager->createCubeMap("skyboxes/sky_right.jpg", "skyboxes/sky_left.jpg",
	//	"skyboxes/sky_top.jpg", "skyboxes/sky_bottom.jpg",
	//	"skyboxes/sky_back.jpg", "skyboxes/sky_front.jpg", true);

	auto* effectLib = m_renderBackend->getEffectLibrary();
	auto* equirectangularSkyBoxShader = effectLib->getEquirectangularSkyBoxShader();
	auto* panoramaSkyBoxShader = effectLib->getPanoramaSkyBoxShader();
	auto* skyboxShader = effectLib->getSkyBoxShader();

	//shadowMap = m_renderBackend->createDepthMap(2048, 2048);
	mRenderTargetSingleSampled = createLightingTarget(windowWidth, windowHeight);
	mPingPong = m_renderBackend->createRenderTarget();

	panoramaSkyBoxShader->bind();
	panoramaSkyBoxShader->setSkyTexture(panoramaSky);

	equirectangularSkyBoxShader->bind();
	equirectangularSkyBoxShader->setSkyTexture(panoramaSky);


	globalLight.setColor(glm::vec3(1.0f, 1.0f, 1.0f));
	globalLight.setPower(3.0f);
	globalLight.setDirection({-1,-1,-1});

	blurEffect = m_renderBackend->getEffectLibrary()->getGaussianBlur();

	CascadedShadow::PCFFilter pcf;
	pcf.sampleCountX = 2;
	pcf.sampleCountY = 2;
	pcf.useLerpFiltering = true;
	mCascadedShadow = std::make_unique<CascadedShadow>(2048, 2048, 4, pcf, 6.0f, true);

	mPbrProbe = std::make_unique<PbrProbe>(panoramaSky);
	mPbrDeferred = std::make_unique<PbrDeferred>(mPbrProbe.get(), &globalLight, mCascadedShadow.get());
	mPbrMrt = mPbrDeferred->createMultipleRenderTarget(windowWidth * ssaaSamples, windowHeight * ssaaSamples);

	mPbrForward = std::make_unique<PbrForward>(mPbrProbe.get(), &globalLight, mCascadedShadow.get());


	//renderTargetSingleSampled->useDepthStencilMap(pbr_mrt->getDepthStencilMapShared());
	
	mRenderTargetSingleSampled->useDepthAttachment(*mPbrMrt->getDepthAttachment());

	CubeMap* background = mPbrProbe->getEnvironmentMap();
	skyboxShader->bind();
	skyboxShader->setSkyTexture(background);
	//pbrShader->setSkyBox(background);

	m_renderBackend->getRasterizer()->enableScissorTest(false);

	auto* mAoSelector = m_renderBackend->getEffectLibrary()->getPostProcessor()->getAOSelector();
	mAoSelector->setUseAmbientOcclusion(true);
	mAoSelector->setAOTechniqueToUse(AOTechnique::SSAO);
}


void nex::PBR_Deferred_Renderer::render(nex::SceneNode* scene, nex::Camera* camera, float frameTime, int windowWidth, int windowHeight)
{
	static bool switcher = true;
	if (mInput->isPressed(Input::KEY_O))
	{
		switcher = !switcher;
	}

	if (switcher)
		renderDeferred(scene, camera, frameTime, windowWidth, windowHeight);
	else
		renderForward(scene, camera, frameTime, windowWidth, windowHeight);
}

void nex::PBR_Deferred_Renderer::setShowDepthMap(bool showDepthMap)
{
	this->mShowDepthMap = showDepthMap;
}

void nex::PBR_Deferred_Renderer::updateRenderTargets(int width, int height)
{
	//update render target dimension
	//the render target dimensions are dependent from the viewport size
	// so first update the viewport and than recreate the render targets
	m_renderBackend->resize(width, height);
	mRenderTargetSingleSampled = createLightingTarget(width, height);
	mPbrMrt = mPbrDeferred->createMultipleRenderTarget(width, height);
	mRenderTargetSingleSampled->useDepthAttachment(*mPbrMrt->getDepthAttachment());

	mPingPong = m_renderBackend->createRenderTarget();
}

nex::AmbientOcclusionSelector* nex::PBR_Deferred_Renderer::getAOSelector()
{
	return m_renderBackend->getEffectLibrary()->getPostProcessor()->getAOSelector();
}

nex::CascadedShadow* nex::PBR_Deferred_Renderer::getCSM()
{
	return mCascadedShadow.get();
}

nex::PbrDeferred* nex::PBR_Deferred_Renderer::getPBR()
{
	return mPbrDeferred.get();
}

void nex::PBR_Deferred_Renderer::renderDeferred(SceneNode* scene, Camera* camera, float frameTime, int windowWidth,
	int windowHeight)
{
	static auto* depthMapShader = RenderBackend::get()->getEffectLibrary()->getDepthMapShader();
	static auto* screenShader = RenderBackend::get()->getEffectLibrary()->getScreenShader();
	static auto* stencilTest = RenderBackend::get()->getStencilTest();

	using namespace std::chrono;

	//m_renderBackend->newFrame();
	//RenderBackend::get()->getRasterizer()->setFillMode(FillMode::FILL, PolygonSide::FRONT_BACK);
	//RenderBackend::get()->getRasterizer()->enableFaceCulling(true);
	//RenderBackend::get()->getRasterizer()->setCullMode(PolygonSide::BACK);
	RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);


	// update and render into cascades
	mPbrMrt->bind();


	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	//renderer->beginScene();

	mPbrMrt->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil); //RenderComponent::Color |


	stencilTest->enableStencilTest(true);
	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

	mPbrDeferred->drawGeometryScene(scene, camera);

	stencilTest->enableStencilTest(false);
	//glm::vec2 minMaxPositiveZ(0.0f, 1.0f);

	//minMaxPositiveZ.x = camera->getFrustum(Perspective).nearPlane;
	//minMaxPositiveZ.y = camera->getFrustum(Perspective).farPlane;

	// Update CSM if it is enabled
	if (mCascadedShadow->isEnabled())
	{
		mCascadedShadow->useTightNearFarPlane(false);
		mCascadedShadow->frameUpdate(camera, globalLight.getDirection(), mPbrMrt->getNormalizedViewSpaceZ());

		for (int i = 0; i < mCascadedShadow->getCascadeData().numCascades; ++i)
		{
			mCascadedShadow->begin(i);
			StaticMeshDrawer::draw(scene, mCascadedShadow->getDepthPassShader());
			mCascadedShadow->end();
		}

		mCascadedShadow->frameReset();
	}


	// render scene to a offscreen buffer
	mRenderTargetSingleSampled->bind();

	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	mRenderTargetSingleSampled->clear(RenderComponent::Color | RenderComponent::Depth);//RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil



	/*Dimension blitRegion = { 0,0, windowWidth * ssaaSamples, windowHeight * ssaaSamples };
	m_renderBackend->blitRenderTargets(pbr_mrt.get(),
		renderTargetSingleSampled,
		blitRegion,
		RenderComponent::Depth | RenderComponent::Stencil);*/

		//m_pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());

		//m_pbr_deferred->drawSky(camera);

	mPbrDeferred->setDirLight(&globalLight);


	stencilTest->enableStencilTest(true);
	stencilTest->setCompareFunc(CompareFunction::EQUAL, 1, 1);

	mPbrDeferred->drawLighting(scene,
		mPbrMrt.get(),
		camera);


	stencilTest->setCompareFunc(CompareFunction::NOT_EQUAL, 1, 1);

	mAtmosphericScattering->bind();
	mAtmosphericScattering->setInverseProjection(inverse(camera->getPerspProjection()));
	mAtmosphericScattering->setInverseViewRotation(inverse(camera->getView()));
	mAtmosphericScattering->setStepCount(16);
	mAtmosphericScattering->setSurfaceHeight(0.99f);
	mAtmosphericScattering->setScatterStrength(0.028f);
	mAtmosphericScattering->setSpotBrightness(10.0f);
	mAtmosphericScattering->setViewport(windowWidth, windowHeight);

	AtmosphericScattering::Light light;
	light.direction = -normalize(globalLight.getDirection());
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


	//stencilTest->enableStencilTest(false);

	//m_pbr_deferred->drawSky(camera);



//renderer->endScene();

//renderTargetSingleSampled->clear(RenderComponent::Depth | RenderComponent::Stencil);//| RenderComponent::Depth | RenderComponent::Stencil




//static auto* blur = RenderBackend::get()->getEffectLibrary()->getGaussianBlur();

	auto* colorTex = static_cast<Texture2D*>(mRenderTargetSingleSampled->getColorAttachmentTexture(0));
	auto* luminanceTexture = static_cast<Texture2D*>(mRenderTargetSingleSampled->getColorAttachmentTexture(1));

	// instead of clearing the buffer we just disable depth and stencil tests for improved performance
	RenderBackend::get()->getDepthBuffer()->enableDepthTest(false);
	RenderBackend::get()->getStencilTest()->enableStencilTest(false);

	// finally render the offscreen buffer to a quad and do post processing stuff
	RenderTarget2D* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();

	static auto* postProcessor = RenderBackend::get()->getEffectLibrary()->getPostProcessor();

	auto* aoMap = postProcessor->getAOSelector()->renderAO(camera, mPbrMrt->getNormalizedViewSpaceZ());

	auto* postProcessed = postProcessor->doPostProcessing(colorTex, luminanceTexture, aoMap, mPingPong.get());

	auto* smaa = postProcessor->getSMAA();

	if (mInput->isPressed(Input::KEY_Y))
	{
		std::cout << "SMAA is rendered: " << mShowDepthMap << std::endl;
	}

	smaa->reset();
	auto* edgeTex = smaa->renderEdgeDetectionPass(postProcessed);
	auto* blendTex = smaa->renderBlendingWeigthCalculationPass(edgeTex);

	//mPingPong->bind();
	//mPingPong->clear(Color | Depth | Stencil);
	//blendTex = postProcessor->doPostProcessing(blendTex, nullptr, mPingPong.get());

	static int switcher = 0;

	if (mInput->isPressed(Input::KEY_Y))
	{
		++switcher;
		switcher %= 4;

		std::cout << "switcher = " << switcher << std::endl;
	}

	if (switcher == 1)
	{
		screenRenderTarget->bind();
		screenRenderTarget->clear(Color | Depth | Stencil);
		screenShader->bind();
		screenShader->useTexture(postProcessed);
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), screenShader);

	}
	else if (switcher == 2)
	{
		screenRenderTarget->bind();
		screenRenderTarget->clear(Color | Depth | Stencil);
		screenShader->bind();
		screenShader->useTexture(edgeTex);
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), screenShader);

	}
	else if (switcher == 3)
	{
		screenRenderTarget->bind();
		screenRenderTarget->clear(Color | Depth | Stencil);
		screenShader->bind();
		screenShader->useTexture(blendTex);
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), screenShader);

	}
	else {
		smaa->renderNeighborhoodBlendingPass(blendTex, postProcessed, screenRenderTarget);

		/*screenRenderTarget->bind();
		screenRenderTarget->clear(Color | Depth | Stencil);
		screenSprite.setTexture(postProcessed);
		screenShader->bind();
		screenShader->useTexture(screenSprite.getTexture());
		StaticMeshDrawer::draw(&screenSprite, screenShader);*/
	}

	return;

	screenShader->bind();
	screenShader->useTexture(mRenderTargetSingleSampled->getColorAttachments()[0].texture.get());
	//screenShader->useTexture(testTexture);
	if (mShowDepthMap)
	{
		//renderer->setViewPort(width / 2 - 256, height / 2 - 256, 512, 512);
		//screenShader->useTexture(ssaoTexture);
		//modelDrawer->draw(&screenSprite, Shaders::Screen);
		//screenSprite.setTexture(shadowMap->getTexture());
		depthMapShader->bind();
		depthMapShader->useDepthMapTexture(mPbrMrt->getNormalizedViewSpaceZ());
		//screenShader->useTexture(shadowMap->getTexture());
		//modelDrawer->draw(&screenSprite, screenShader);
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), depthMapShader);
		/*if (m_aoSelector.getActiveAOTechnique() == AmbientOcclusionSelector::HBAO) {
			m_aoSelector.getHBAO()->displayAOTexture(aoTexture);
		} else
		{
			m_aoSelector.getSSAO()->displayAOTexture(aoTexture);
		}*/
		//hbao->displayTexture(pbr_mrt->getDepth());

		//m_aoSelector.getHBAO()->displayAOTexture(aoTexture);
	}
	else
	{
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), screenShader);

		//ssao_deferred->displayAOTexture();
	}
	//renderer->endScene();
}

void nex::PBR_Deferred_Renderer::renderForward(SceneNode* scene, Camera* camera, float frameTime, int windowWidth,
	int windowHeight)
{
	static auto* screenShader = RenderBackend::get()->getEffectLibrary()->getScreenShader();
	static auto* stencilTest = RenderBackend::get()->getStencilTest();
	static auto* postProcessor = RenderBackend::get()->getEffectLibrary()->getPostProcessor();
	static auto* defaultImageSampler = TextureManager::get()->getDefaultImageSampler();

	for (auto i = 0; i < 10; ++i)
	{
		defaultImageSampler->unbind(i);
	}

	RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);
	RenderBackend::get()->getDepthBuffer()->enableDepthClamp(true);

	// Update CSM if it is enabled
	if (mCascadedShadow->isEnabled())
	{
		mCascadedShadow->frameUpdate(camera, globalLight.getDirection(), nullptr);

		for (int i = 0; i < mCascadedShadow->getCascadeData().numCascades; ++i)
		{
			mCascadedShadow->begin(i);
			StaticMeshDrawer::draw(scene, mCascadedShadow->getDepthPassShader());
			mCascadedShadow->end();
		}

		mCascadedShadow->frameReset();
	}


	// render scene to a offscreen buffer
	mRenderTargetSingleSampled->bind();

	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	mRenderTargetSingleSampled->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);//RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil


	stencilTest->enableStencilTest(true);
	stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
	stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

	mPbrForward->drawLighting(scene, camera);

	stencilTest->enableStencilTest(false);
	auto* aoMap = postProcessor->getAOSelector()->renderAO(camera, (Texture2D*)mRenderTargetSingleSampled->getDepthAttachment()->texture.get());


	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	mRenderTargetSingleSampled->bind();
	stencilTest->enableStencilTest(true);
	stencilTest->setCompareFunc(CompareFunction::NOT_EQUAL, 1, 1);

	mAtmosphericScattering->bind();
	mAtmosphericScattering->setInverseProjection(inverse(camera->getPerspProjection()));
	mAtmosphericScattering->setInverseViewRotation(inverse(camera->getView()));
	mAtmosphericScattering->setStepCount(16);
	mAtmosphericScattering->setSurfaceHeight(0.99f);
	mAtmosphericScattering->setScatterStrength(0.028f);
	mAtmosphericScattering->setSpotBrightness(10.0f);
	mAtmosphericScattering->setViewport(windowWidth, windowHeight);

	AtmosphericScattering::Light light;
	light.direction = -normalize(globalLight.getDirection());
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


	auto* colorTex = static_cast<Texture2D*>(mRenderTargetSingleSampled->getColorAttachmentTexture(0));
	auto* luminanceTexture = static_cast<Texture2D*>(mRenderTargetSingleSampled->getColorAttachmentTexture(1));

	// instead of clearing the buffer we just disable depth and stencil tests for improved performance
	RenderBackend::get()->getDepthBuffer()->enableDepthTest(false);
	RenderBackend::get()->getStencilTest()->enableStencilTest(false);

	// finally render the offscreen buffer to a quad and do post processing stuff
	RenderTarget2D* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();

	auto* postProcessed = postProcessor->doPostProcessing(colorTex, luminanceTexture, aoMap, mPingPong.get());

	auto* smaa = postProcessor->getSMAA();

	if (mInput->isPressed(Input::KEY_Y))
	{
		std::cout << "SMAA is rendered: " << mShowDepthMap << std::endl;
	}

	smaa->reset();
	auto* edgeTex = smaa->renderEdgeDetectionPass(postProcessed);
	auto* blendTex = smaa->renderBlendingWeigthCalculationPass(edgeTex);

	static int switcher = 0;

	if (mInput->isPressed(Input::KEY_Y))
	{
		++switcher;
		switcher %= 4;

		std::cout << "switcher = " << switcher << std::endl;
	}

	if (switcher == 1)
	{
		screenRenderTarget->bind();
		screenRenderTarget->clear(Color | Depth | Stencil);
		screenShader->bind();
		screenShader->useTexture(postProcessed);
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), screenShader);

	}
	else if (switcher == 2)
	{
		screenRenderTarget->bind();
		screenRenderTarget->clear(Color | Depth | Stencil);
		screenShader->bind();
		screenShader->useTexture(edgeTex);
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), screenShader);

	}
	else if (switcher == 3)
	{
		screenRenderTarget->bind();
		screenRenderTarget->clear(Color | Depth | Stencil);
		screenShader->bind();
		screenShader->useTexture(blendTex);
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), screenShader);

	}
	else {
		smaa->renderNeighborhoodBlendingPass(blendTex, postProcessed, screenRenderTarget);
	}
}

std::unique_ptr<nex::RenderTarget2D> nex::PBR_Deferred_Renderer::createLightingTarget(unsigned width, unsigned height)
{
	auto result = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR());

	RenderAttachment luminance;
	luminance.colorAttachIndex = 1;
	luminance.target = TextureTarget::TEXTURE2D;
	luminance.type = RenderAttachmentType::COLOR;
	// TODO: use one color channel!
	luminance.texture = std::make_shared<Texture2D>(width, height, TextureData::createRenderTargetRGBAHDR(), nullptr);

	result->addColorAttachment(std::move(luminance));

	result->finalizeAttachments();

	return result;
}

nex::PBR_Deferred_Renderer_ConfigurationView::PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer) : mRenderer(renderer)
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

	ImGui::PopID();
}
