#include <pbr_deferred/PBR_Deferred_Renderer.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/SampleMeshes.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/camera/TrackballQuatCamera.hpp>
#include <nex/shader/NormalsShader.hpp>
#include <nex/shader/PBRShader.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <nex/scene/SceneNode.hpp>
#include <nex/shader/DepthMapShader.hpp>
#include <nex/shader/ScreenShader.hpp>
#include <nex/shader/ShadowShader.hpp>
#include <nex/util/Math.hpp>

using namespace glm;
using namespace std;
using namespace nex;

int ssaaSamples = 1;

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
PBR_Deferred_Renderer::PBR_Deferred_Renderer(Backend backend) :
	Renderer(std::move(backend)),
	blurEffect(nullptr),
	logClient(getLogServer()),
	panoramaSky(nullptr),
	renderTargetSingleSampled(nullptr),
	shadowMap(nullptr),
	showDepthMap(false),
	m_useAmbientOcclusion(true)
{
	logClient.setPrefix("[PBR_Deferred_Renderer]");

	mixValue = 0.2f;
}


bool PBR_Deferred_Renderer::getShowDepthMap() const
{
	return showDepthMap;
}

void PBR_Deferred_Renderer::init(int windowWidth, int windowHeight)
{
	using namespace placeholders;

	ShaderManager* shaderManager = m_renderBackend->getShaderManager();
	ModelManager* modelManager = m_renderBackend->getModelManager();
	TextureManager* textureManager = m_renderBackend->getTextureManager();

	//auto rendererResizeCallback = bind(&Renderer::setViewPort, renderer, 0, 0, _1, _2);
	//window->addResizeCallback(rendererResizeCallback);


	shaderManager->loadShaders();

	modelManager->loadModels();

	panoramaSky = textureManager->getHDRImage("skyboxes/panoramas/pisa.hdr", { false, true, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32 });
	//panoramaSky = textureManager->getImage("skyboxes/panoramas/pisa.hdr", { true, true, Linear_Mipmap_Linear, Linear, ClampToEdge });
	//panoramaSky = textureManager->getHDRImage("hdr/newport_loft.hdr", { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 });

	//CubeMap* cubeMapSky = textureManager->createCubeMap("skyboxes/sky_right.jpg", "skyboxes/sky_left.jpg",
	//	"skyboxes/sky_top.jpg", "skyboxes/sky_bottom.jpg",
	//	"skyboxes/sky_back.jpg", "skyboxes/sky_front.jpg", true);
	
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBox));

	PanoramaSkyBoxShader* panoramaSkyBoxShader = dynamic_cast<PanoramaSkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBoxPanorama));

	EquirectangularSkyBoxShader* equirectangularSkyBoxShader = dynamic_cast<EquirectangularSkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBoxEquirectangular));

	PBRShader* pbrShader = dynamic_cast<PBRShader*>
		(shaderManager->getConfig(Shaders::Pbr));

	shadowMap = m_renderBackend->createDepthMap(4096, 4096);
	renderTargetSingleSampled = m_renderBackend->createRenderTarget();

	panoramaSkyBoxShader->setSkyTexture(panoramaSky);
	equirectangularSkyBoxShader->setSkyTexture(panoramaSky);


	vec3 position = {1.0f, 1.0f, 1.0f };
	position = 5.0f * position;
	globalLight.setPosition(position);
	globalLight.lookAt({0,0,0});


	// init shaders
	pbrShader->setLightColor({ 1.0f, 1.0f, 1.0f });
	pbrShader->setLightDirection(globalLight.getLook());

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

	pbr_deferred = m_renderBackend->getShadingModelFactory().create_PBR_Deferred_Model(panoramaSky);
	pbr_mrt = pbr_deferred->createMultipleRenderTarget(windowWidth * ssaaSamples, windowHeight * ssaaSamples);

	m_aoSelector.setSSAO(m_renderBackend->createDeferredSSAO());
	m_aoSelector.setHBAO(m_renderBackend->createHBAO());

	CubeMap* background = pbr_deferred->getEnvironmentMap();
	skyBoxShader->setSkyTexture(background);
	pbrShader->setSkyBox(background);
}



void PBR_Deferred_Renderer::render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight)
{
	ModelDrawer* modelDrawer = m_renderBackend->getModelDrawer();
	ScreenShader* screenShader = dynamic_cast<ScreenShader*>(
		m_renderBackend->getShaderManager()->getConfig(Shaders::Screen));
	DepthMapShader* depthMapShader = dynamic_cast<DepthMapShader*>(
		m_renderBackend->getShaderManager()->getConfig(Shaders::DepthMap));
	using namespace chrono;

	// update the scene

	//renderer->setViewPort(0, 0, window->getWidth(), window->getHeight());
	//renderer->useScreenTarget();
	//renderer->beginScene();
	//renderer->setBackgroundColor({0.5f, 0.5f, 0.5f});
	//renderer->endScene();
	
	FrustumCuboid cameraCuboid = camera->getFrustumCuboid(Perspective, 0.0f, 0.08f);
	const mat4& cameraView = camera->getView();
	mat4 inverseCameraView = inverse(cameraView);

	mat4 test = globalLight.getView();
	FrustumCuboid cameraCuboidWorld = test * inverseCameraView * cameraCuboid;
	AABB ccBB = fromCuboid(cameraCuboidWorld);

	Frustum shadowFrustum = { ccBB.min.x, ccBB.max.x, ccBB.min.y, ccBB.max.y, ccBB.min.z, ccBB.max.z };
	shadowFrustum = {-15.0f, 15.0f, -15.0f, 15.0f, -10.0f, 10.0f};
	globalLight.setOrthoFrustum(shadowFrustum);

	const mat4& lightProj = globalLight.getProjection(Orthographic);
	const mat4& lightView = globalLight.getView();


	// render scene to the shadow depth map
	m_renderBackend->useBaseRenderTarget(shadowMap);
	//renderer->beginScene();
	m_renderBackend->clearRenderTarget(shadowMap, RenderComponent::Depth);
	//renderer->enableAlphaBlending(false);
	//renderer->cullFaces(CullingMode::Back);

	pbr_deferred->drawSceneToShadowMap(scene,
		shadowMap,
		globalLight,
		lightView,
		lightProj);

	//renderer->cullFaces(CullingMode::Back);
	//renderer->endScene();

	m_renderBackend->useBaseRenderTarget(pbr_mrt.get());
	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	//renderer->beginScene();
	m_renderBackend->clearRenderTarget(pbr_mrt.get(), RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
		pbr_deferred->drawGeometryScene(scene,
			camera->getView(),
			camera->getPerspProjection());
	//renderer->endScene();

	//ssao_deferred->renderAO(pbr_mrt->getPosition(), pbr_mrt->getNormal(), camera->getPerspProjection());
	//ssao_deferred->blur();

	Texture* aoTexture = renderAO(camera);

	// render scene to a offscreen buffer
	m_renderBackend->useBaseRenderTarget(renderTargetSingleSampled);
	m_renderBackend->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	m_renderBackend->clearRenderTarget(renderTargetSingleSampled, RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
	//renderer->clearRenderTarget(renderTargetSingleSampled, RenderComponent::Stencil);
	//renderer->beginScene();
	

	Dimension blitRegion = { 0,0, windowWidth * ssaaSamples, windowHeight * ssaaSamples };
	m_renderBackend->blitRenderTargets(pbr_mrt.get(),
		renderTargetSingleSampled,
		blitRegion,
		RenderComponent::Depth | RenderComponent::Stencil);

	//pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());

		//renderer->enableAlphaBlending(true);

		pbr_deferred->drawLighting(scene, 
			pbr_mrt.get(), 
			shadowMap->getTexture(), 
			aoTexture,
			globalLight, 
			camera->getView(), 
			lightProj * lightView);

		pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());
	


	//renderer->endScene();
	
	// finally render the offscreen buffer to a quad and do post processing stuff
	BaseRenderTarget* screenRenderTarget = m_renderBackend->getDefaultRenderTarget();

	m_renderBackend->useBaseRenderTarget(screenRenderTarget);
	m_renderBackend->setViewPort(0, 0, windowWidth, windowHeight);
	//renderer->beginScene();
	m_renderBackend->clearRenderTarget(m_renderBackend->getDefaultRenderTarget(), RenderComponent::Depth | RenderComponent::Stencil);
	
	screenSprite.setTexture(renderTargetSingleSampled->getTexture());
	//screenSprite.setTexture(pbr_mrt->getAlbedo());
	//screenSprite.setTexture(ssao_deferred->getAO_Result());
	
	//screenSprite.setTexture(pbr_mrt->getAlbedo());

	depthMapShader->useDepthMapTexture(shadowMap->getTexture());

	screenShader->useTexture(screenSprite.getTexture());
	if (showDepthMap)
	{
		//renderer->setViewPort(width / 2 - 256, height / 2 - 256, 512, 512);
		//screenShader->useTexture(ssaoTexture);
		//modelDrawer->draw(&screenSprite, Shaders::Screen);
		//screenSprite.setTexture(shadowMap->getTexture());
		//depthMapShader->useDepthMapTexture(pbr_mrt->getDepth());
		//screenShader->useTexture(pbr_mrt->getDepth());
		//modelDrawer->draw(&screenSprite, Shaders::DepthMap);
		if (m_aoSelector.getActiveAOTechnique() == AmbientOcclusionSelector::HBAO) {
			m_aoSelector.getHBAO()->displayAOTexture(aoTexture);
		} else
		{
			m_aoSelector.getSSAO()->displayAOTexture(aoTexture);
		}
		//hbao->displayTexture(pbr_mrt->getDepth());
	} else
	{
		modelDrawer->draw(&screenSprite, Shaders::Screen);
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
	m_renderBackend->setViewPort(0, 0, width, height);
	m_renderBackend->destroyRenderTarget(renderTargetSingleSampled);
	renderTargetSingleSampled = m_renderBackend->createRenderTarget();
	pbr_mrt = pbr_deferred->createMultipleRenderTarget(width, height);
	//ssao_deferred->onSizeChange(width, height);
	
	if (m_aoSelector.getActiveAOTechnique() == AmbientOcclusionSelector::HBAO)
	{
		m_aoSelector.getHBAO()->onSizeChange(width, height);
	} else
	{
		m_aoSelector.getSSAO()->onSizeChange(width, height);
	}
}

hbao::HBAO* PBR_Deferred_Renderer::getHBAO()
{
	return m_aoSelector.getHBAO();
}

AmbientOcclusionSelector* PBR_Deferred_Renderer::getAOSelector()
{
	return &m_aoSelector;
}

void PBR_Deferred_Renderer::useAmbientOcclusion(bool useAO)
{
	m_useAmbientOcclusion = useAO;
}

bool PBR_Deferred_Renderer::getUseAmbientOcclusion() const
{
	return m_useAmbientOcclusion;
}

Texture* PBR_Deferred_Renderer::renderAO(Camera* camera)
{
	if (!m_aoSelector.isAmbientOcclusionActive())
		// Return a default white texture (means no ambient occlusion)
		return m_renderBackend->getTextureManager()->getDefaultWhiteTexture();

	if (m_aoSelector.getActiveAOTechnique() == AmbientOcclusionSelector::HBAO)
	{
		hbao::Projection projection;
		Frustum frustum = camera->getFrustum(Perspective);
		projection.fov = radians(camera->getFOV());
		projection.farplane = frustum.farPlane;
		projection.matrix = camera->getPerspProjection();
		projection.nearplane = frustum.nearPlane;
		projection.orthoheight = 0;
		projection.perspective = true;

		m_aoSelector.getHBAO()->renderAO(pbr_mrt->getDepth(), projection, true);

		return m_aoSelector.getHBAO()->getBlurredResult();
	}

	// use SSAO

	SSAO_Deferred* ssao = m_aoSelector.getSSAO();
	return ssao->getBlurredResult();
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
		static AmbientOcclusionSelector::AOTechnique selectedTechnique = AmbientOcclusionSelector::HBAO;
		if (ImGui::Combo("AO technique", (int*)&selectedTechnique, items, IM_ARRAYSIZE(items)))
		{
			std::cout << selectedTechnique << " is selected!" << std::endl;
		}
	}

	ImGui::PopID();
}