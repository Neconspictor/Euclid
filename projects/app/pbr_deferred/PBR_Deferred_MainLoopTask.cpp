#include <pbr_deferred/PBR_Deferred_MainLoopTask.hpp>
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
PBR_Deferred_MainLoopTask::PBR_Deferred_MainLoopTask(RenderBackendPtr renderer) :
	blurEffect(nullptr),
	m_isRunning(true),
	logClient(getLogServer()),
	panoramaSky(nullptr),
	renderTargetSingleSampled(nullptr),
	renderer(renderer),
	shadowMap(nullptr),
	showDepthMap(false)
{
	logClient.setPrefix("[PBR_Deferred_MainLoopTask]");

	mixValue = 0.2f;
}

bool PBR_Deferred_MainLoopTask::getShowDepthMap() const
{
	return showDepthMap;
}

void PBR_Deferred_MainLoopTask::init(int windowWidth, int windowHeight)
{
	using namespace placeholders;

	ShaderManager* shaderManager = renderer->getShaderManager();
	ModelManager* modelManager = renderer->getModelManager();
	TextureManager* textureManager = renderer->getTextureManager();

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

	shadowMap = renderer->createDepthMap(4096, 4096);
	renderTargetSingleSampled = renderer->createRenderTarget();

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

	blurEffect = renderer->getEffectLibrary()->getGaussianBlur();

	pbr_deferred = renderer->getShadingModelFactory().create_PBR_Deferred_Model(panoramaSky);
	pbr_mrt = pbr_deferred->createMultipleRenderTarget(windowWidth * ssaaSamples, windowHeight * ssaaSamples);

	ssao_deferred = renderer->createDeferredSSAO();
	hbao = renderer->createHBAO();

	CubeMap* background = pbr_deferred->getEnvironmentMap();
	skyBoxShader->setSkyTexture(background);
	pbrShader->setSkyBox(background);
}



void PBR_Deferred_MainLoopTask::run(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight)
{
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	ScreenShader* screenShader = dynamic_cast<ScreenShader*>(
		renderer->getShaderManager()->getConfig(Shaders::Screen));
	DepthMapShader* depthMapShader = dynamic_cast<DepthMapShader*>(
		renderer->getShaderManager()->getConfig(Shaders::DepthMap));
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
	renderer->useBaseRenderTarget(shadowMap);
	//renderer->beginScene();
	renderer->clearRenderTarget(shadowMap, RenderComponent::Depth);
	//renderer->enableAlphaBlending(false);
	//renderer->cullFaces(CullingMode::Back);

	pbr_deferred->drawSceneToShadowMap(scene,
		shadowMap,
		globalLight,
		lightView,
		lightProj);

	//renderer->cullFaces(CullingMode::Back);
	//renderer->endScene();

	renderer->useBaseRenderTarget(pbr_mrt.get());
	renderer->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	//renderer->beginScene();
	renderer->clearRenderTarget(pbr_mrt.get(), RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
		pbr_deferred->drawGeometryScene(scene,
			camera->getView(),
			camera->getPerspProjection());
	//renderer->endScene();

	//ssao_deferred->renderAO(pbr_mrt->getPosition(), pbr_mrt->getNormal(), camera->getPerspProjection());
	//ssao_deferred->blur();

	hbao::Projection projection;
	Frustum frustum = camera->getFrustum(Perspective);
	projection.fov = radians(camera->getFOV());
	projection.farplane = frustum.farPlane;
	projection.matrix = camera->getPerspProjection();
	projection.nearplane = frustum.nearPlane;
	projection.orthoheight = 0;
	projection.perspective = true;

	hbao->renderAO(pbr_mrt->getDepth(),  projection, true);

	Texture* aoTexture = hbao->getBlurredResult();

	// render scene to a offscreen buffer
	renderer->useBaseRenderTarget(renderTargetSingleSampled);
	renderer->setViewPort(0, 0, windowWidth * ssaaSamples, windowHeight * ssaaSamples);
	renderer->clearRenderTarget(renderTargetSingleSampled, RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
	//renderer->clearRenderTarget(renderTargetSingleSampled, RenderComponent::Stencil);
	//renderer->beginScene();
	

	Dimension blitRegion = { 0,0, windowWidth * ssaaSamples, windowHeight * ssaaSamples };
	renderer->blitRenderTargets(pbr_mrt.get(),
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
	BaseRenderTarget* screenRenderTarget = renderer->getDefaultRenderTarget();

	renderer->useBaseRenderTarget(screenRenderTarget);
	renderer->setViewPort(0, 0, windowWidth, windowHeight);
	//renderer->beginScene();
	renderer->clearRenderTarget(renderer->getDefaultRenderTarget(), RenderComponent::Depth | RenderComponent::Stencil);
	
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
		hbao->displayAOTexture();
		//hbao->displayTexture(pbr_mrt->getDepth());
	} else
	{
		modelDrawer->draw(&screenSprite, Shaders::Screen);
		//ssao_deferred->displayAOTexture();
	}
	//renderer->endScene();
}

void PBR_Deferred_MainLoopTask::setShowDepthMap(bool showDepthMap)
{
	this->showDepthMap = showDepthMap;
}

void PBR_Deferred_MainLoopTask::setRunning(bool isRunning)
{
	m_isRunning = isRunning;
}

bool PBR_Deferred_MainLoopTask::isRunning() const
{
	return m_isRunning;
}

void PBR_Deferred_MainLoopTask::updateRenderTargets(int width, int height)
{
	//update render target dimension
	//the render target dimensions are dependent from the viewport size
	// so first update the viewport and than recreate the render targets
	renderer->setViewPort(0, 0, width, height);
	renderer->destroyRenderTarget(renderTargetSingleSampled);
	renderTargetSingleSampled = renderer->createRenderTarget();
	pbr_mrt = pbr_deferred->createMultipleRenderTarget(width, height);
	//ssao_deferred->onSizeChange(width, height);
	hbao->onSizeChange(width, height);
}

hbao::HBAO* PBR_Deferred_MainLoopTask::getHBAO()
{
	return hbao.get();
}
