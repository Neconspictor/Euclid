#include <PBR_Deferred_MainLoopTask.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <Brofiler.h>
#include <glm/glm.hpp>
#include <mesh/SampleMeshes.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <camera/TrackballQuatCamera.hpp>
#include <camera/FPQuaternionCamera.hpp>
#include <shader/NormalsShader.hpp>
#include <camera/FPCamera.hpp>
#include <shader/PBRShader.hpp>
#include <shader/SkyBoxShader.hpp>
#include <platform/SystemUI.hpp>
#include <scene/SceneNode.hpp>
#include <shader/DepthMapShader.hpp>
#include <shader/ScreenShader.hpp>
#include <shader/ShadowShader.hpp>
#include <util/Math.hpp>

using namespace glm;
using namespace std;
using namespace platform;

int ssaaSamples = 1;

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
PBR_Deferred_MainLoopTask::PBR_Deferred_MainLoopTask(EnginePtr engine, WindowPtr window, WindowSystemPtr windowSystem, RendererPtr renderer, unsigned int flags):
	Task(flags), blurEffect(nullptr), isRunning(true), logClient(getLogServer()), panoramaSky(nullptr), renderTargetMultisampled(nullptr), 
	renderTargetSingleSampled(nullptr), runtime(0), scene(nullptr), shadowMap(nullptr), showDepthMap(false),  ui(nullptr)
{
	this->window = window;
	this->windowSystem = windowSystem;
	this->renderer = renderer;
	this->engine = engine;
	originalTitle = window->getTitle();
	logClient.setPrefix("[PBR_Deferred_MainLoopTask]");

	mixValue = 0.2f;

	camera = make_shared<FPCamera>(FPCamera());
}

PBR_Deferred_MainLoopTask::~PBR_Deferred_MainLoopTask()
{
	int i = 0;
}

SceneNode* PBR_Deferred_MainLoopTask::createShadowScene()
{
	nodes.push_back(SceneNode());
	SceneNode* root = &nodes.back();

	nodes.push_back(SceneNode());
	SceneNode* ground = &nodes.back();
	root->addChild(ground);

	nodes.push_back(SceneNode());
	SceneNode* cube1 = &nodes.back();
	root->addChild(cube1);

	nodes.push_back(SceneNode());
	SceneNode* sphere = &nodes.back();
	root->addChild(sphere);

	vobs.push_back(Vob("misc/textured_plane.obj", Shaders::Pbr));
	ground->setVob(&vobs.back());
	//vobs.push_back(Vob("misc/textured_cube.obj"));
	vobs.push_back(Vob("normal_map_test/normal_map_test.obj", Shaders::Pbr));
	cube1->setVob(&vobs.back());

	vobs.push_back(Vob("normal_map_test/normal_map_sphere.obj", Shaders::Pbr));
	sphere->setVob(&vobs.back());

	ground->getVob()->setPosition({ 10, 0, 0 });
	cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });

	sphere->getVob()->setPosition({3.0f, 3.8f, -1.0f});

	return root;
}

SceneNode * PBR_Deferred_MainLoopTask::createCubeReflectionScene()
{
	nodes.push_back(SceneNode());
	SceneNode* root = &nodes.back();

	nodes.push_back(SceneNode());
	SceneNode* cube1 = &nodes.back();
	root->addChild(cube1);

	vobs.push_back(Vob("misc/untextured_cube.obj", Shaders::Pbr));
	cube1->setVob(&vobs.back());

	cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });
	return root;
}

void PBR_Deferred_MainLoopTask::init()
{
	using namespace placeholders;

	window->activate();

	int windowWidth = window->getWidth();
	int windowHeight = window->getHeight();

	ShaderManager* shaderManager = renderer->getShaderManager();
	ModelManager* modelManager = renderer->getModelManager();
	TextureManager* textureManager = renderer->getTextureManager();
	Input* input = window->getInputDevice();

	auto focusCallback = bind(&PBR_Deferred_MainLoopTask::onWindowsFocus, this, _1, _2);
	auto scrollCallback = bind(&Camera::onScroll, camera.get(), _1, _2);
	input->addWindowFocusCallback(focusCallback);
	input->addScrollCallback(scrollCallback);

	camera->setPosition(vec3(0.0f, 3.0f, 2.0f));
	camera->setLook(vec3(0.0f, 0.0f, -1.0f));
	camera->setUp(vec3(0.0f, 1.0f, 0.0f));
	camera->setAspectRatio((float)windowWidth / (float)windowHeight);

	Frustum frustum = camera->getFrustum(Perspective);
	frustum.left = -10.0f;
	frustum.right = 10.0f;
	frustum.bottom = -10.0f;
	frustum.top = 10.0f;
	frustum.nearPlane = 0.1f;
	frustum.farPlane = 10.0f;
	camera->setOrthoFrustum(frustum);


	if (TrackballQuatCamera* casted = dynamic_cast<TrackballQuatCamera*>(camera.get()))
	{
		auto cameraResizeCallback = bind(&TrackballQuatCamera::updateOnResize, casted, _1, _2);
		casted->updateOnResize(window->getWidth(), window->getHeight());
		input->addResizeCallback(cameraResizeCallback);
	}
	//auto rendererResizeCallback = bind(&Renderer::setViewPort, renderer, 0, 0, _1, _2);
	//window->addResizeCallback(rendererResizeCallback);

	input->addResizeCallback([&](int width, int height)
	{
		LOG(logClient, Debug) << "addResizeCallback : width: " << width << ", height: " << height;

		if (!window->hasFocus()) {
			LOG(logClient, Debug) << "addResizeCallback : no focus!";
		}

		if (width == 0 || height == 0) {
			LOG(logClient, Warning) << "addResizeCallback : width or height is 0!";
			return;
		}

		camera->setAspectRatio((float)width / (float)height);

		//update render target dimension
		//the render target dimensions are dependent from the viewport size
		// so first update the viewport and than recreate the render targets
		// TODO, simplify this process -> render targets should be indepedent from vieport dimension?
		renderer->setViewPort(0, 0, width, height);
		renderer->destroyRenderTarget(renderTargetMultisampled);
		renderer->destroyRenderTarget(renderTargetSingleSampled);
		renderTargetMultisampled = renderer->createRenderTarget(1);
		renderTargetSingleSampled = renderer->createRenderTarget(1);
	});

	input->addRefreshCallback([&]() {
		LOG(logClient, Warning) << "addRefreshCallback : called!";
		if (!window->hasFocus()) {
			LOG(logClient, Warning) << "addRefreshCallback : no focus!";
			return;
		}
	});

	shaderManager->loadShaders();

	modelManager->loadModels();

	window->activate();

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

	renderTargetMultisampled = renderer->createRenderTarget(1);
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

	// init scene
	scene = createShadowScene();
	//scene = createCubeReflectionScene();
	//scene = createAsteriodField();

	blurEffect = renderer->getEffectLibrary()->getGaussianBlur();

	pbr_deferred = renderer->getShadingModelFactory().create_PBR_Deferred_Model(panoramaSky);
	//pbr_deferred.init(renderer, panoramaSky);

	CubeMap* background = pbr_deferred->getEnvironmentMap();

	//CubeRenderTarget* testCubeMap = renderer->renderCubeMap(2048, 2048, panoramaSky);
	skyBoxShader->setSkyTexture(background);
	pbrShader->setSkyBox(background);

	pbr_mrt = pbr_deferred->createMultipleRenderTarget(windowWidth, windowHeight);
}

void PBR_Deferred_MainLoopTask::setUI(SystemUI* ui)
{
	this->ui = ui;
}

static float frameTimeElapsed = 0;

void PBR_Deferred_MainLoopTask::run()
{
	BROFILER_FRAME("MainLoopTask");
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	ScreenShader* screenShader = dynamic_cast<ScreenShader*>(
		renderer->getShaderManager()->getConfig(Shaders::Screen));
	DepthMapShader* depthMapShader = dynamic_cast<DepthMapShader*>(
		renderer->getShaderManager()->getConfig(Shaders::DepthMap));
	using namespace chrono;
	
	float frameTime = timer.update();
	int millis = static_cast<int> (1.0f /(frameTime * 1000.0f));

	// manual 60 fps cap -> only temporary!
	int minimumMillis = 16;
	if (millis < minimumMillis)
	{
		//this_thread::sleep_for(milliseconds(minimumMillis - millis));
		//frameTime += timer.update();
	}
	frameTimeElapsed += frameTime;

	float fps = counter.update(frameTimeElapsed);
	updateWindowTitle(frameTimeElapsed, fps);
	frameTimeElapsed = 0.0f;


	if (!window->isOpen())
	{
		engine->stop();
		return;
	}

	// Poll input events before checking if the app is running, otherwise 
	// the window is likely to hang or crash (at least on windows platform)
	windowSystem->pollEvents();
	
	// pause app if it is not active (e.g. window isn't focused)
	if (!isRunning)
	{
		this_thread::sleep_for(milliseconds(500));
		return;
	}


	window->activate();
	handleInputEvents();
	window->activate();

	updateCamera(window->getInputDevice(), timer.getLastUpdateTimeDifference());

	BROFILER_CATEGORY("After input handling / Before rendering", Profiler::Color::AntiqueWhite);

	renderer->setViewPort(0, 0, window->getWidth(), window->getHeight());
	renderer->useScreenTarget();
	renderer->beginScene();
	renderer->setBackgroundColor({0.5f, 0.5f, 0.5f});
	renderer->endScene();
	
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
	renderer->beginScene();
	renderer->useDepthMap(shadowMap);
	renderer->enableAlphaBlending(false);
	renderer->cullFaces(CullingMode::Back);

	pbr_deferred->drawSceneToShadowMap(scene,
		frameTimeElapsed,
		shadowMap,
		globalLight,
		lightView,
		lightProj);

	renderer->cullFaces(CullingMode::Back);
	renderer->endScene();

	renderer->useBaseRenderTarget(pbr_mrt);
	renderer->setViewPort(0, 0, window->getWidth() * ssaaSamples, window->getHeight() * ssaaSamples);
	renderer->beginScene();
		pbr_deferred->drawGeometryScene(scene,
		frameTimeElapsed,
		camera->getView(),
		camera->getPerspProjection());
	renderer->endScene();

	// render scene to a offscreen buffer
	renderer->useBaseRenderTarget(renderTargetMultisampled);
	renderer->setViewPort(0,0, window->getWidth() * ssaaSamples, window->getHeight() * ssaaSamples);
	renderer->beginScene();
		renderer->enableAlphaBlending(true);
		pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());

		pbr_deferred->drawLighting(scene, 
			frameTimeElapsed, 
			pbr_mrt, 
			shadowMap->getTexture(), 
			globalLight, 
			camera->getView(), 
			lightProj * lightView);

	/*pbr_deferred->drawScene(scene,
		camera->getPosition(),
		frameTimeElapsed,
		shadowMap->getTexture(),
		globalLight,
		lightView,
		lightProj,
		camera->getView(),
		camera->getPerspProjection());*/
	


	renderer->endScene();


	// copy rendered scene to the single sampled render target
	//renderer->blitRenderTargets(renderTargetMultisampled, renderTargetSingleSampled);
	
	// finally render the offscreen buffer to a quad and do post processing stuff
	renderer->setViewPort(0, 0, window->getWidth(), window->getHeight());
	renderer->useScreenTarget();
	renderer->beginScene();
	screenSprite.setTexture(renderTargetMultisampled->getTexture());
	
	//screenSprite.setTexture(pbr_mrt->getAlbedo());

	depthMapShader->useDepthMapTexture(shadowMap->getTexture());

	screenShader->useTexture(screenSprite.getTexture());
	if (showDepthMap)
	{
		int width = window->getWidth();
		int height = window->getHeight();
		//renderer->setViewPort(width / 2 - 256, height / 2 - 256, 512, 512);
		//screenShader->useTexture(pbr_deferred->getBrdfLookupTexture());
		//modelDrawer->draw(&screenSprite, Shaders::Screen);
		screenSprite.setTexture(shadowMap->getTexture());
		modelDrawer->draw(&screenSprite, Shaders::DepthMap);
	} else
	{
		modelDrawer->draw(&screenSprite, Shaders::Screen);
	}
	renderer->endScene();

	window->swapBuffers();

	BROFILER_CATEGORY("After rendering / before buffer swapping", Profiler::Color::Aqua);
}

void PBR_Deferred_MainLoopTask::drawScene(const mat4& projection, const mat4& view, Shaders shaderType)
{
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	scene->update(frameTimeElapsed);
	scene->draw(renderer, modelDrawer, projection, view, shaderType);
	renderer->endScene();
}

void PBR_Deferred_MainLoopTask::updateCamera(Input* input, float deltaTime)
{
	if (window->hasFocus())
	{
		camera.get()->update(input, deltaTime);
		
		if (dynamic_cast<FPCameraBase*>(camera.get()))
		{
			window->setCursorPosition(window->getWidth() / 2, window->getHeight() / 2);
		}
	}
}

void PBR_Deferred_MainLoopTask::handleInputEvents()
{
	using namespace platform;
	BROFILER_CATEGORY("Before input handling", Profiler::Color::AliceBlue);
	Input* input = window->getInputDevice();


	if (input->isPressed(Input::KEY_ESCAPE))
	{
		window->close();
	}

	/*if (input->isPressed(Input::KEY_KP_ENTER) || input->isPressed(Input::KEY_RETURN))
	{
		window->minimize();
	}*/

	if (input->isPressed(Input::KEY_UP))
	{
		mixValue += 0.1f;
		if (mixValue >= 1.0f)
			mixValue = 1.0f;

		mixValue = round(mixValue * 10) / 10;
		LOG(logClient, Debug) << "MixValue: " << mixValue;
	}

	if (input->isPressed(Input::KEY_DOWN))
	{
		mixValue -= 0.1f;
		if (mixValue <= 0.0f)
			mixValue = 0.0f;

		mixValue = round(mixValue * 10) / 10;
		LOG(logClient, Debug) << "MixValue: " << mixValue;
	}

	if (input->isPressed(Input::KEY_Y))
	{
		showDepthMap = !showDepthMap;
	}


	// Context refresh Does not work right now!
	if (input->isPressed(Input::KEY_B)) {
		if (window->isInFullscreenMode()) {
			window->setWindowed();
			//renderer->endScene();
		}
		else {
			window->setFullscreen();
			//renderer->endScene();
		}

		LOG(logClient, Debug) << "toggle";
	}
}

void PBR_Deferred_MainLoopTask::updateWindowTitle(float frameTime, float fps)
{
	runtime += frameTime;
	if (runtime > 1)
	{
		stringstream ss; ss << originalTitle << " : FPS= " << fps;
		window->setTitle(ss.str());
		runtime -= 1;
	}
}

void PBR_Deferred_MainLoopTask::onWindowsFocus(Window* window, bool receivedFocus)
{
	if (receivedFocus)
	{
		LOG(logClient, platform::Debug) << "received focus!";
		isRunning = true;
	} else
	{
		LOG(logClient, platform::Debug) << "lost focus!";
		isRunning = false;
		if (window->isInFullscreenMode())
		{
			window->minimize();
		}
	}
}