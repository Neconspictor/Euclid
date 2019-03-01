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

using namespace glm;
using namespace std;
using namespace nex;

int ssaaSamples = 1;

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
PBR_Deferred_Renderer::PBR_Deferred_Renderer(RenderBackend* backend) :
	Renderer(backend),
	blurEffect(nullptr),
	m_logger("PBR_Deferred_Renderer"),
	panoramaSky(nullptr),
	renderTargetSingleSampled(nullptr),
	//shadowMap(nullptr),
	showDepthMap(false)
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


	vec3 position = {1.0f, 1.0f, 1.0f };
	position = 15.0f * position;
	globalLight.setPosition(position);
	globalLight.lookAt({0,0,0});
	globalLight.update(true);
	globalLight.setColor(vec3(1.0f, 1.0f, 1.0f));

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
	pcf.sampleCountX = 0;
	pcf.sampleCountY = 0;
	pcf.useLerpFiltering = false;
	m_cascadedShadow = make_unique<CascadedShadow>(1024, 1024, 4, pcf, true);

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
}


void PBR_Deferred_Renderer::render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight)
{
	static auto* depthMapShader = RenderBackend::get()->getEffectLibrary()->getDepthMapShader();
	static auto* screenShader = RenderBackend::get()->getEffectLibrary()->getScreenShader();

	using namespace chrono;

	m_renderBackend->newFrame();
	m_renderBackend->getRasterizer()->enableScissorTest(false);

	// Update CSM if it is enabled
	if (m_cascadedShadow->isEnabled())
	{
		m_cascadedShadow->frameUpdate(camera, globalLight.getLook());

		for (int i = 0; i < m_cascadedShadow->getCascadeData().numCascades; ++i)
		{
			m_cascadedShadow->begin(i);
			StaticMeshDrawer::draw(scene, m_cascadedShadow->getDepthPassShader());
			m_cascadedShadow->end();
		}
	}


	// update and render into cascades
	pbr_mrt->bind();


	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	m_renderBackend->setScissor(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	//renderer->beginScene();

	pbr_mrt->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
	m_pbr_deferred->drawGeometryScene(scene,
		camera->getView(),
		camera->getPerspProjection());

	Texture* aoTexture = renderAO(camera, pbr_mrt->getDepth(), pbr_mrt->getNormal());

	// render scene to a offscreen buffer
	renderTargetSingleSampled->bind();

	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	renderTargetSingleSampled->clear(RenderComponent::Color | RenderComponent::Depth);//| RenderComponent::Depth | RenderComponent::Stencil

	

	/*Dimension blitRegion = { 0,0, windowWidth * ssaaSamples, windowHeight * ssaaSamples };
	m_renderBackend->blitRenderTargets(pbr_mrt.get(),
		renderTargetSingleSampled,
		blitRegion,
		RenderComponent::Depth | RenderComponent::Stencil);*/


	const auto& cascadedData = m_cascadedShadow->getCascadeData();
	Texture* cascadedDepthMap = m_cascadedShadow->getDepthTextureArray();

		//m_pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());

		m_pbr_deferred->drawLighting(scene, 
			pbr_mrt.get(), 
			//shadowMap->getTexture(), 
			aoTexture,
			globalLight, 
			camera->getView(),
			camera->getPerspProjection(),
			m_cascadedShadow->getWorldToShadowSpace(),
			cascadedData,
			cascadedDepthMap);

		m_pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());
	


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