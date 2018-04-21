#include <PBR_MainLoopTask.hpp>
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
//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
PBR_MainLoopTask::PBR_MainLoopTask(EnginePtr engine, WindowPtr window, WindowSystemPtr windowSystem, RendererPtr renderer, unsigned int flags):
	Task(flags), blurEffect(nullptr), isRunning(true), logClient(getLogServer()), panoramaSky(nullptr), renderTargetMultisampled(nullptr), 
	renderTargetSingleSampled(nullptr), runtime(0), scene(nullptr), shadowMap(nullptr), showDepthMap(false), sky(nullptr), 
	skyBox("misc/SkyBoxPlane.obj"), ui(nullptr)
{
	this->window = window;
	this->windowSystem = windowSystem;
	this->renderer = renderer;
	this->engine = engine;
	originalTitle = window->getTitle();
	logClient.setPrefix("[MainLoop]");

	mixValue = 0.2f;

	camera = make_shared<FPCamera>(FPCamera());
}

SceneNode* PBR_MainLoopTask::createShadowScene()
{
	nodes.push_back(SceneNode(Shaders::Pbr));
	SceneNode* root = &nodes.back();

	nodes.push_back(SceneNode(Shaders::Pbr));
	SceneNode* ground = &nodes.back();
	root->addChild(ground);

	nodes.push_back(SceneNode(Shaders::Pbr));
	SceneNode* cube1 = &nodes.back();
	root->addChild(cube1);

	vobs.push_back(Vob("misc/textured_plane.obj"));
	ground->setVob(&vobs.back());
	//vobs.push_back(Vob("misc/textured_cube.obj"));
	vobs.push_back(Vob("normal_map_test/normal_map_test.obj"));
	cube1->setVob(&vobs.back());

	ground->getVob()->setPosition({ 10, 0, 0 });
	cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });
	return root;
}

void PBR_MainLoopTask::init()
{
	using namespace placeholders;

	ShaderManager* shaderManager = renderer->getShaderManager();
	ModelManager* modelManager = renderer->getModelManager();
	TextureManager* textureManager = renderer->getTextureManager();

	auto focusCallback = bind(&PBR_MainLoopTask::onWindowsFocus, this, _1, _2);
	auto scrollCallback = bind(&Camera::onScroll, camera.get(), _1, _2);
	this->window->addWindowFocusCallback(focusCallback);
	this->window->getInputDevice()->addScrollCallback(scrollCallback);

	camera->setPosition(vec3(0.0f, 3.0f, 2.0f));
	camera->setLook(vec3(0.0f, 0.0f, -1.0f));
	camera->setUp(vec3(0.0f, 1.0f, 0.0f));
	Renderer::Viewport viewport = window->getViewport();
	camera->setAspectRatio((float)viewport.width / (float)viewport.height);

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
		casted->updateOnResize(viewport.width, viewport.height);
		window->addResizeCallback(cameraResizeCallback);
	}
	//auto rendererResizeCallback = bind(&Renderer::setViewPort, renderer, 0, 0, _1, _2);
	//window->addResizeCallback(rendererResizeCallback);

	window->addResizeCallback([&](int width, int height)
	{
		camera->setAspectRatio((float)width / (float)height);
		renderer->setViewPort(0, 0, width, height);
		renderer->destroyRenderTarget(renderTargetMultisampled);
		renderer->destroyRenderTarget(renderTargetSingleSampled);
		renderTargetMultisampled = renderer->createRenderTarget(4);
		renderTargetSingleSampled = renderer->createRenderTarget(1);
	});

	shaderManager->loadShaders();

	modelManager->loadModels();

	sky = textureManager->createCubeMap("skyboxes/sky_right.jpg", "skyboxes/sky_left.jpg",
		"skyboxes/sky_top.jpg", "skyboxes/sky_bottom.jpg",
		"skyboxes/sky_back.jpg", "skyboxes/sky_front.jpg", true);
    

	panoramaSky = textureManager->getImage("skyboxes/panoramas/pisa.hdr", {true, true, Bilinear, Bilinear, ClampToEdge});
	//panoramaSky = textureManager->getHDRImage("skyboxes/panoramas/pisa.hdr", { true, true, Bilinear, Bilinear, ClampToEdge });
	
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBox));

	PanoramaSkyBoxShader* panoramaSkyBoxShader = dynamic_cast<PanoramaSkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBoxPanorama));

	PBRShader* pbrShader = dynamic_cast<PBRShader*>
		(shaderManager->getConfig(Shaders::Pbr));

	shadowMap = renderer->createDepthMap(4096, 4096);

	renderTargetMultisampled = renderer->createRenderTarget(8);
	renderTargetSingleSampled = renderer->createRenderTarget();

	skyBoxShader->setSkyTexture(sky);
	panoramaSkyBoxShader->setSkyTexture(panoramaSky);
	pbrShader->setSkyBox(sky);


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
	//scene = createAsteriodField();

	blurEffect = renderer->getEffectLibrary()->getGaussianBlur();
}

void PBR_MainLoopTask::setUI(SystemUI* ui)
{
	this->ui = ui;
}

static float frameTimeElapsed = 0;

void PBR_MainLoopTask::run()
{
	BROFILER_FRAME("MainLoopTask");
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	ScreenShader* screenShader = dynamic_cast<ScreenShader*>(
		renderer->getShaderManager()->getConfig(Shaders::Screen));
	DepthMapShader* depthMapShader = dynamic_cast<DepthMapShader*>(
		renderer->getShaderManager()->getConfig(Shaders::DepthMap));
	PBRShader* pbrShader = dynamic_cast<PBRShader*>
		(renderer->getShaderManager()->getConfig(Shaders::Pbr));
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

	updateCamera(window->getInputDevice(), timer.getLastUpdateTimeDifference());

	BROFILER_CATEGORY("After input handling / Before rendering", Profiler::Color::AntiqueWhite);

	renderer->setBackgroundColor({0.5f, 0.5f, 0.5f});

	// render shadows to a depth map
	renderer->useDepthMap(shadowMap);
	//renderer->useVarianceShadowMap(vsMap);
	renderer->enableAlphaBlending(false);

	
	FrustumCuboid cameraCuboid = camera->getFrustumCuboid(Perspective, 0.0f, 0.08f);
	const mat4& cameraView = camera->getView();
	mat4 inverseCameraView = inverse(cameraView);

	mat4 test = globalLight.getView();
	FrustumCuboid cameraCuboidWorld = test * inverseCameraView * cameraCuboid;
	AABB ccBB = fromCuboid(cameraCuboidWorld);
	//ccBB.min.z -= 3;
	//ccBB.max.z += 3;

	// Snap shadow frustum to texel bounds for avoiding edge shimmering
	/*float size = vsMap->getWidth();//shadowMap->getWidth() * shadowMap->getHeight();
	vec3 normalizedMapSize = vec3(1.0f / (float)vsMap->getWidth(), 1.0f / (float)vsMap->getHeight(), 1.0f);
	vec3 worldUnitsPerTexel = ccBB.max - ccBB.min;
	worldUnitsPerTexel *= normalizedMapSize;
	
	// We snap the camera to 1 pixel increments so that moving the camera does not cause the shadows
	// to jitter. This is a matter of integer dividing by the world space size of a texel
	ccBB.min.x /= worldUnitsPerTexel.x;
	ccBB.min.y /= worldUnitsPerTexel.y;
	ccBB.min.x = floor(ccBB.min.x);
	ccBB.min.y = floor(ccBB.min.y);
	ccBB.min.x *= worldUnitsPerTexel.x;
	ccBB.min.y *= worldUnitsPerTexel.y;

	ccBB.max.x /= worldUnitsPerTexel.x;
	ccBB.max.y /= worldUnitsPerTexel.y;
	ccBB.max.x = floor(ccBB.max.x);
	ccBB.max.y = floor(ccBB.max.y);
	ccBB.max.x *= worldUnitsPerTexel.x;
	ccBB.max.y *= worldUnitsPerTexel.y;*/


	Frustum shadowFrustum = { ccBB.min.x, ccBB.max.x, ccBB.min.y, ccBB.max.y, ccBB.min.z, ccBB.max.z };
	shadowFrustum = {-15.0f, 15.0f, -15.0f, 15.0f, -10.0f, 10.0f};
	globalLight.setOrthoFrustum(shadowFrustum);

	pbrShader->setLightProjMatrix(globalLight.getProjection(Orthographic));
	pbrShader->setLightSpaceMatrix(globalLight.getProjection(Orthographic) * globalLight.getView());
	pbrShader->setLightViewMatrix(globalLight.getView());
	
	renderer->cullFaces(CullingMode::Back);
	//renderer->cullFaces(CullingMode::Front);
	drawScene(globalLight.getOrthoProjection(), globalLight.getView(), Shaders::Shadow);
	//drawScene(globalLight.getOrthoProjection(), globalLight.getView(), Shaders::VarianceShadow);
	renderer->cullFaces(CullingMode::Back);
	renderer->endScene();


	// blur the shadow map to smooth out the edges
	//for (int i = 0; i < 1; ++i) blurEffect->blur(vsMap, vsMapCache);
	//blurEffect->blur(vsMap, vsMapCache);

	// now render scene to a offscreen buffer
	renderer->useRenderTarget(renderTargetMultisampled);
	renderer->enableAlphaBlending(true);
	renderer->beginScene();
	pbrShader->setShadowMap(shadowMap->getTexture());

	pbrShader->setCameraPosition(camera->getPosition());

	drawSky(camera->getPerspProjection(), camera->getView());
	drawScene(camera->getPerspProjection(), camera->getView());



	renderer->blitRenderTargets(renderTargetMultisampled, renderTargetSingleSampled);
	
	// finally render the offscreen buffer to a quad and do post processing stuff
	renderer->useScreenTarget();
	renderer->beginScene();
	screenSprite.setTexture(renderTargetSingleSampled->getTexture());
	depthMapShader->useDepthMapTexture(shadowMap->getTexture());

	screenShader->useTexture(screenSprite.getTexture());
	if (showDepthMap)
	{
		modelDrawer->draw(&screenSprite, Shaders::DepthMap);
	} else
	{
		modelDrawer->draw(&screenSprite, Shaders::Screen);
	}
	renderer->endScene();

	window->swapBuffers();

	BROFILER_CATEGORY("After rendering / before buffer swapping", Profiler::Color::Aqua);
}

void PBR_MainLoopTask::drawScene(const mat4& projection, const mat4& view, Shaders shaderType)
{
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	scene->update(frameTimeElapsed);
	scene->draw(renderer, modelDrawer, projection, view, shaderType);
	renderer->endScene();
}

void PBR_MainLoopTask::drawSky(const mat4& projection, const mat4& view)
{
	PanoramaSkyBoxShader* panoramaSkyBoxShader = dynamic_cast<PanoramaSkyBoxShader*>
		(renderer->getShaderManager()->getConfig(Shaders::SkyBoxPanorama));
	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	mat4 identity;
	mat4 skyBoxView = mat4(mat3(view));

	TransformData data = { &projection, &view, nullptr };
	data.model = &identity;
	data.view = &skyBoxView;
	//modelDrawer->draw(&skyBox, skyBoxShader, data);
	modelDrawer->draw(&skyBox, Shaders::SkyBoxPanorama, data);
}

void PBR_MainLoopTask::updateCamera(Input* input, float deltaTime)
{
	if (window->hasFocus())
	{
		camera.get()->update(input, deltaTime);
		
		if (dynamic_cast<FPCameraBase*>(camera.get()))
		{
			Renderer::Viewport viewport = window->getViewport();
			window->setCursorPosition(viewport.width / 2, viewport.height / 2);
		}
	}
}

void PBR_MainLoopTask::handleInputEvents()
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
}

void PBR_MainLoopTask::updateWindowTitle(float frameTime, float fps)
{
	runtime += frameTime;
	if (runtime > 1)
	{
		stringstream ss; ss << originalTitle << " : FPS= " << fps;
		window->setTitle(ss.str());
		runtime -= 1;
	}
}

void PBR_MainLoopTask::onWindowsFocus(Window* window, bool receivedFocus)
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