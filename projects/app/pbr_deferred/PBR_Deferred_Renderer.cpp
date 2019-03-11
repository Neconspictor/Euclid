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
#include <nex/texture/Gbuffer.hpp>
#include "nex/shadow/CascadedShadow.hpp"
#include <nex/drawing/StaticMeshDrawer.hpp>
#include "nex/RenderBackend.hpp"
#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>

using namespace glm;
using namespace std;
using namespace nex;

int ssaaSamples = 1;

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
PBR_Deferred_Renderer::PBR_Deferred_Renderer(RenderBackend* backend, Input* input) :
	Renderer(backend),
	blurEffect(nullptr),
	m_logger("PBR_Deferred_Renderer"),
	panoramaSky(nullptr),
	renderTargetSingleSampled(nullptr),
	//shadowMap(nullptr),
	showDepthMap(false),
	mInput(input)
{
	m_aoSelector.setUseAmbientOcclusion(true);
	//m_aoSelector.setAOTechniqueToUse(AmbientOcclusionSelector::SSAO);
}


bool PBR_Deferred_Renderer::getShowDepthMap() const
{
	return showDepthMap;
}

void PBR_Deferred_Renderer::init(int windowWidth, int windowHeight)
{
	using namespace placeholders;


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
	renderTargetSingleSampled = m_renderBackend->createRenderTarget();

	panoramaSkyBoxShader->bind();
	panoramaSkyBoxShader->setSkyTexture(panoramaSky);

	equirectangularSkyBoxShader->bind();
	equirectangularSkyBoxShader->setSkyTexture(panoramaSky);


	globalLight.setColor(vec3(1.0f, 1.0f, 1.0f));
	globalLight.setPower(3.0f);
	globalLight.setDirection({-1,-1,-1});

	vec2 dim = {1.0, 1.0};
	vec2 pos = {0, 0};

	// center
	pos.x = 0.5f * (1.0f - dim.x);
	pos.y = 0.5f * (1.0f - dim.y);

	// align to bottom corner
	//pos.x = 1.0f - dim.x;
	//pos.y = 1.0f - dim.y;

	//align to top right corner
	//pos.x = 1.0f - dim.x;
	//pos.y = 0;

	screenSprite.setPosition(pos);
	screenSprite.setWidth(dim.x);
	screenSprite.setHeight(dim.y);
	//screenSprite.setZRotation(radians(45.0f));

	blurEffect = m_renderBackend->getEffectLibrary()->getGaussianBlur();

	CascadedShadow::PCFFilter pcf;
	pcf.sampleCountX = 2;
	pcf.sampleCountY = 2;
	pcf.useLerpFiltering = true;
	m_cascadedShadow = make_unique<CascadedShadow>(2048, 2048, 4, pcf, 6.0f, true);

	m_pbr_deferred = make_unique<PBR_Deferred>(panoramaSky, m_cascadedShadow.get());
	pbr_mrt = m_pbr_deferred->createMultipleRenderTarget(windowWidth * ssaaSamples, windowHeight * ssaaSamples);

	//renderTargetSingleSampled->useDepthStencilMap(pbr_mrt->getDepthStencilMapShared());
	
	renderTargetSingleSampled->useDepthAttachment(*pbr_mrt->getDepthAttachment());

	m_aoSelector.setSSAO(make_unique<SSAO_Deferred>(windowWidth * ssaaSamples, windowHeight * ssaaSamples));
	m_aoSelector.setHBAO(make_unique<HBAO>(windowWidth * ssaaSamples, windowHeight * ssaaSamples));

	CubeMap* background = m_pbr_deferred->getEnvironmentMap();
	skyboxShader->bind();
	skyboxShader->setSkyTexture(background);
	//pbrShader->setSkyBox(background);

	mSceneNearFarComputeShader = make_unique<SceneNearFarComputeShader>();

	m_renderBackend->getRasterizer()->enableScissorTest(false);
}


void PBR_Deferred_Renderer::render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight)
{
	static auto* depthMapShader = RenderBackend::get()->getEffectLibrary()->getDepthMapShader();
	static auto* screenShader = RenderBackend::get()->getEffectLibrary()->getScreenShader();

	using namespace chrono;

	m_renderBackend->newFrame();


	// update and render into cascades
	pbr_mrt->bind();


	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	//renderer->beginScene();

	pbr_mrt->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
	m_pbr_deferred->drawGeometryScene(scene,
		camera->getView(),
		camera->getPerspProjection());

	Texture* aoTexture = renderAO(camera, pbr_mrt->getDepth(), pbr_mrt->getNormal());
	glm::vec2 minMaxPositiveZ = computeNearFarTest(camera, windowWidth, windowHeight, pbr_mrt->getDepth());

	//minMaxPositiveZ.x = camera->getFrustum(Perspective).nearPlane;
	//minMaxPositiveZ.y = camera->getFrustum(Perspective).farPlane;

	// Update CSM if it is enabled
	if (m_cascadedShadow->isEnabled())
	{
		m_cascadedShadow->frameUpdate(camera, globalLight.getDirection(), minMaxPositiveZ);

		for (int i = 0; i < m_cascadedShadow->getCascadeData().numCascades; ++i)
		{
			m_cascadedShadow->begin(i);
			StaticMeshDrawer::draw(scene, m_cascadedShadow->getDepthPassShader());
			m_cascadedShadow->end();
		}
	}


	// render scene to a offscreen buffer
	renderTargetSingleSampled->bind();

	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	renderTargetSingleSampled->clear(RenderComponent::Color | RenderComponent::Depth);//| RenderComponent::Depth | RenderComponent::Stencil

	

	/*Dimension blitRegion = { 0,0, windowWidth * ssaaSamples, windowHeight * ssaaSamples };
	m_renderBackend->blitRenderTargets(pbr_mrt.get(),
		renderTargetSingleSampled,
		blitRegion,
		RenderComponent::Depth | RenderComponent::Stencil);*/

		//m_pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());

		//m_pbr_deferred->drawSky(camera);


		mAtmosphericScattering.bind();
		mAtmosphericScattering.setInverseProjection(inverse(camera->getPerspProjection()));
		mAtmosphericScattering.setInverseViewRotation(inverse(camera->getView()));
		mAtmosphericScattering.setStepCount(16);
		mAtmosphericScattering.setSurfaceHeight(0.99f);
		mAtmosphericScattering.setScatterStrength(0.028f);
		mAtmosphericScattering.setSpotBrightness(10.0f);
		mAtmosphericScattering.setViewport(windowWidth, windowHeight);

		AtmosphericScattering::Light light;
		light.direction = -normalize(globalLight.getDirection());
		light.intensity = 1.8f;
		mAtmosphericScattering.setLight(light);

		AtmosphericScattering::Mie mie;
		mie.brightness = 0.1f;
		mie.collectionPower = 0.39f;
		mie.distribution = 0.63f;
		mie.strength = 0.264f;
		mAtmosphericScattering.setMie(mie);

		AtmosphericScattering::Rayleigh rayleigh;
		rayleigh.brightness = 3.3f;
		rayleigh.collectionPower = 0.81f;
		rayleigh.strength = 0.139f;
		mAtmosphericScattering.setRayleigh(rayleigh);

		mAtmosphericScattering.renderSky();

		m_pbr_deferred->setDirLight(&globalLight);

		m_pbr_deferred->drawLighting(scene, 
			pbr_mrt.get(), 
			camera, 
			aoTexture);

		//m_pbr_deferred->drawSky(camera);
	


	//renderer->endScene();
	
	// finally render the offscreen buffer to a quad and do post processing stuff
	RenderTarget2D* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();

	screenRenderTarget->bind();
	m_renderBackend->setViewPort(0, 0, windowWidth, windowHeight);
	//renderer->beginScene();
	screenRenderTarget->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
	
	screenSprite.setTexture(renderTargetSingleSampled->getColorAttachments()[0].texture.get()); //TODO

	//screenSprite.setTexture(pbr_mrt->getAlbedo());
	//screenSprite.setTexture(ssao_deferred->getAO_Result());
	
	//screenSprite.setTexture(pbr_mrt->getAlbedo());

	//depthMapShader->bind();
	//depthMapShader->useDepthMapTexture(shadowMap->getTexture());

	screenShader->bind();
	screenShader->useTexture(screenSprite.getTexture());
	//screenShader->useTexture(testTexture);
	if (showDepthMap)
	{
		//renderer->setViewPort(width / 2 - 256, height / 2 - 256, 512, 512);
		//screenShader->useTexture(ssaoTexture);
		//modelDrawer->draw(&screenSprite, Shaders::Screen);
		//screenSprite.setTexture(shadowMap->getTexture());
		depthMapShader->bind();
		depthMapShader->useDepthMapTexture(pbr_mrt->getDepth());
		//screenShader->useTexture(shadowMap->getTexture());
		//modelDrawer->draw(&screenSprite, screenShader);
		StaticMeshDrawer::draw(&screenSprite, depthMapShader);
		/*if (m_aoSelector.getActiveAOTechnique() == AmbientOcclusionSelector::HBAO) {
			m_aoSelector.getHBAO()->displayAOTexture(aoTexture);
		} else
		{
			m_aoSelector.getSSAO()->displayAOTexture(aoTexture);
		}*/
		//hbao->displayTexture(pbr_mrt->getDepth());
		
		//m_aoSelector.getHBAO()->displayAOTexture(aoTexture);
	} else
	{
		StaticMeshDrawer::draw(&screenSprite, screenShader);
		
		//ssao_deferred->displayAOTexture();
	}
	//renderer->endScene();


	
}

void PBR_Deferred_Renderer::setShowDepthMap(bool showDepthMap)
{
	this->showDepthMap = showDepthMap;
}

void PBR_Deferred_Renderer::updateRenderTargets(int width, int height)
{
	//update render target dimension
	//the render target dimensions are dependent from the viewport size
	// so first update the viewport and than recreate the render targets
	m_renderBackend->resize(width, height);
	renderTargetSingleSampled = m_renderBackend->createRenderTarget();
	pbr_mrt = m_pbr_deferred->createMultipleRenderTarget(width, height);

	//renderTargetSingleSampled->useDepthStencilMap(pbr_mrt->getDepthStencilMapShared());
	renderTargetSingleSampled->useDepthAttachment(*pbr_mrt->getDepthAttachment());
	//ssao_deferred->onSizeChange(width, height);

	m_aoSelector.getHBAO()->onSizeChange(width, height);
	m_aoSelector.getSSAO()->onSizeChange(width, height);

	/*if (m_aoSelector.getActiveAOTechnique() == AmbientOcclusionSelector::HBAO)
	{
		m_aoSelector.getHBAO()->onSizeChange(width, height);
	} else
	{
		m_aoSelector.getSSAO()->onSizeChange(width, height);
	}*/
}

nex::HBAO* PBR_Deferred_Renderer::getHBAO()
{
	return m_aoSelector.getHBAO();
}

AmbientOcclusionSelector* PBR_Deferred_Renderer::getAOSelector()
{
	return &m_aoSelector;
}

CascadedShadow* PBR_Deferred_Renderer::getCSM()
{
	return m_cascadedShadow.get();
}

PBR_Deferred* PBR_Deferred_Renderer::getPBR()
{
	return m_pbr_deferred.get();
}

Texture* PBR_Deferred_Renderer::renderAO(Camera* camera, Texture* gDepth, Texture* gNormal)
{
	//TODO
	//return m_renderBackend->getTextureManager()->getDefaultWhiteTexture();
	if (!m_aoSelector.isAmbientOcclusionActive())
		// Return a default white texture (means no ambient occlusion)
		return TextureManager::get()->getDefaultWhiteTexture();

	if (m_aoSelector.getActiveAOTechnique() == AmbientOcclusionSelector::HBAO)
	{
		nex::Projection projection;
		Frustum frustum = camera->getFrustum(Perspective);
		projection.fov = radians(camera->getFOV());
		projection.farplane = frustum.farPlane;
		projection.matrix = camera->getPerspProjection();
		projection.nearplane = frustum.nearPlane;
		projection.orthoheight = 0;
		projection.perspective = true;

		m_aoSelector.getHBAO()->renderAO(gDepth, projection, true);

		return m_aoSelector.getHBAO()->getBlurredResult();
	}

	// use SSAO

	SSAO_Deferred* ssao = m_aoSelector.getSSAO();
	ssao->renderAO(gDepth, gNormal, camera->getPerspProjection());
	ssao->blur();
	return ssao->getBlurredResult();
	//return ssao->getAO_Result();
}

glm::vec2 PBR_Deferred_Renderer::computeNearFarTest(Camera* camera, int windowWidth, int windowHeight, Texture* depth)
{
	mSceneNearFarComputeShader->bind();


	unsigned xDim = 16 * 16; // 256
	unsigned yDim = 8 * 8; // 128

	unsigned dispatchX = windowWidth % xDim == 0 ? windowWidth / xDim : windowWidth / xDim + 1;
	unsigned dispatchY = windowHeight % yDim == 0 ? windowHeight / yDim : windowHeight / yDim + 1;

	const auto frustum = camera->getFrustum(Perspective);

	mSceneNearFarComputeShader->setConstants(frustum.nearPlane + 0.05, frustum.farPlane - 0.05, camera->getPerspProjection());
	mSceneNearFarComputeShader->setDepthTexture(depth);

	mSceneNearFarComputeShader->dispatch(dispatchX, dispatchY, 1);


	// TODO : readback is very slow -> optimize by calculating csm bounds on the gpu!
	auto result = mSceneNearFarComputeShader->readResult();
	glm::vec2 vecResult(result.minMax);

	static bool printed = false;
	

	if (!printed || mInput->isPressed(Input::KEY_K))
	{
		//std::cout << "result->lock = " << result->lock << "\n";

		std::cout << "----------------------------------------------------------------------------\n";
		std::cout << "result min/max = " << glm::to_string(vecResult) << "\n";
		std::cout << "----------------------------------------------------------------------------\n" << std::endl;

		printed = true;
	}

	// reset
	mSceneNearFarComputeShader->reset();

	return vecResult;
}

PBR_Deferred_Renderer_ConfigurationView::PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer) : m_renderer(renderer)
{
}

void PBR_Deferred_Renderer_ConfigurationView::drawSelf()
{
	// render configuration properties
	ImGui::PushID(m_id.c_str());

	AmbientOcclusionSelector* aoSelector = m_renderer->getAOSelector();
	bool useAmbientOcclusion = aoSelector->isAmbientOcclusionActive();

	if (ImGui::Checkbox("Ambient occlusion", &useAmbientOcclusion))
	{
		aoSelector->setUseAmbientOcclusion(useAmbientOcclusion);
	}

	if (useAmbientOcclusion)
	{
		std::stringstream ss;
		ss << AmbientOcclusionSelector::HBAO;
		std::string hbaoText = ss.str();

		ss.str("");
		ss << AmbientOcclusionSelector::SSAO;
		std::string ssaoText = ss.str();

		const char* items[] = { hbaoText.c_str(), ssaoText.c_str() };
		AmbientOcclusionSelector::AOTechnique selectedTechnique = aoSelector->getActiveAOTechnique();

		ImGui::SameLine(0, 70);
		if (ImGui::Combo("AO technique", (int*)&selectedTechnique, items, IM_ARRAYSIZE(items)))
		{
			std::cout << selectedTechnique << " is selected!" << std::endl;
			aoSelector->setAOTechniqueToUse(selectedTechnique);
		}
	}

	ImGui::PopID();
}