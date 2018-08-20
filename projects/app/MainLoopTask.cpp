#include <MainLoopTask.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <glm/glm.hpp>
#include <nex/mesh/SampleMeshes.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/camera/TrackballQuatCamera.hpp>
#include <nex/camera/FPQuaternionCamera.hpp>
#include <nex/shader/NormalsShader.hpp>
#include <nex/camera/FPCamera.hpp>
#include <nex/shader/PhongTextureShader.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <nex/scene/SceneNode.hpp>
#include <nex/shader/DepthMapShader.hpp>
#include <nex/shader/ScreenShader.hpp>
#include <nex/shader/ShadowShader.hpp>
#include <nex/util/Math.hpp>

using namespace glm;
using namespace std;
using namespace nex;
//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
MainLoopTask::MainLoopTask(EnginePtr engine, WindowPtr window, WindowSystemPtr windowSystem, RendererPtr renderer, unsigned int flags):
	Task(flags), asteriodSize(0), asteriodTrafos(nullptr), blurEffect(nullptr), isRunning(true), logClient(getLogServer()),
	nanosuitModel("nanosuit_reflection/nanosuit.obj", Shaders::BlinnPhongTex), panoramaSky(nullptr), pointShadowMap(nullptr), renderTargetMultisampled(nullptr), 
	renderTargetSingleSampled(nullptr), runtime(0), scene(nullptr), shadowMap(nullptr), showDepthMap(false), sky(nullptr), 
	skyBox("misc/SkyBoxPlane.obj", Shaders::BlinnPhongTex), ui(nullptr), vsMap(nullptr), vsMapCache(nullptr)
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

SceneNode* MainLoopTask::createAsteriodField()
{
	ModelManager* modelManager = renderer->getModelManager();

	nodes.push_back(SceneNode());
	SceneNode* root = &nodes.back();
	
	nodes.push_back(SceneNode());
	SceneNode* planetNode = &nodes.back();
	root->addChild(planetNode);

	vobs.push_back(Vob("planet/planet.obj", Shaders::BlinnPhongTex));
	planetNode->setVob(&vobs.back());
	planetNode->getVob()->setPosition({ 0.0f, 0.0f, 0.0f });
	planetNode->setDrawingType(DrawingTypes::SOLID);

	asteriodSize = 1000;
	asteriodTrafos = new mat4[asteriodSize];

	srand((unsigned int)chrono::system_clock::now().time_since_epoch().count()); // initialize random seed	
	float radius = 50.0;
	float offset = 2.5f;
	for (uint i = 0; i < asteriodSize; i++)
	{
		mat4 model;
		// 1. Translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)asteriodSize * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // y value has smaller displacement
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = translate(model, vec3(x, y, z));
		// 2. Scale: Scale between 0.05 and 0.25f
		float scale = (float)(rand() % 20) / 100.0f + 0.05f;
		model = glm::scale(model, vec3(scale));
		// 3. Rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (float)(rand() % 360);
		model = glm::rotate(model, rotAngle, vec3(0.4f, 0.6f, 0.8f));
		// 4. Now add to list of matrices
		asteriodTrafos[i] = model;
	}

	Model* instanced = modelManager->getModel("rock/rock.obj", Shaders::BlinnPhongTex);
	modelManager->useInstances(instanced, asteriodTrafos, asteriodSize);

	nodes.push_back(SceneNode());
	SceneNode* asteriodNode = &nodes.back();
	root->addChild(asteriodNode);
	vobs.push_back(Vob("rock/rock.obj", Shaders::BlinnPhongTex));
	asteriodNode->setVob(&vobs.back());
	asteriodNode->setDrawingType(DrawingTypes::INSTANCED);
	asteriodNode->setInstanceCount(asteriodSize);

	return root;
}

SceneNode* MainLoopTask::createShadowScene()
{
	nodes.push_back(SceneNode());
	SceneNode* root = &nodes.back();

	nodes.push_back(SceneNode());
	SceneNode* ground = &nodes.back();
	root->addChild(ground);

	nodes.push_back(SceneNode());
	SceneNode* cube1 = &nodes.back();
	root->addChild(cube1);

	vobs.push_back(Vob("misc/textured_plane.obj", Shaders::BlinnPhongTex));
	ground->setVob(&vobs.back());
	//vobs.push_back(Vob("misc/textured_cube.obj"));
	vobs.push_back(Vob("normal_map_test/normal_map_test.obj", Shaders::BlinnPhongTex));
	cube1->setVob(&vobs.back());

	ground->getVob()->setPosition({ 10, 0, 0 });
	cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });
	return root;
}

void MainLoopTask::init()
{
	using namespace placeholders;

	ShaderManager* shaderManager = renderer->getShaderManager();
	ModelManager* modelManager = renderer->getModelManager();
	TextureManager* textureManager = renderer->getTextureManager();
	Input* input = window->getInputDevice();

	auto focusCallback = bind(&MainLoopTask::onWindowsFocus, this, placeholders::_1, placeholders::_2);
	auto scrollCallback = bind(&Camera::onScroll, camera.get(), placeholders::_1, placeholders::_2);
	input->addWindowFocusCallback(focusCallback);
	input->addScrollCallback(scrollCallback);

	camera->setPosition(vec3(0.0f, 3.0f, 2.0f));
	camera->setLook(vec3(0.0f, 0.0f, -1.0f));
	camera->setUp(vec3(0.0f, 1.0f, 0.0f));
	camera->setAspectRatio((float)window->getWidth() / (float)window->getHeight());

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
		auto cameraResizeCallback = bind(&TrackballQuatCamera::updateOnResize, casted, placeholders::_1, placeholders::_2);
		casted->updateOnResize(window->getWidth(), window->getHeight());
		input->addResizeCallback(cameraResizeCallback);
	}
	//auto rendererResizeCallback = bind(&Renderer::setViewPort, renderer, 0, 0, _1, _2);
	//input->addResizeCallback(rendererResizeCallback);

	input->addResizeCallback([&](int width, int height)
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
    
	/*sky = textureManager->createCubeMap("skyboxes/test/test_right.jpg", "skyboxes/test/test_left.jpg",
		"skyboxes/test/test_top.jpg", "skyboxes/test/test_bottom.jpg",
		"skyboxes/test/test_front.jpg", "skyboxes/test/test_back.jpg", true);
    */
	panoramaSky = textureManager->getImage("skyboxes/panoramas/pisa.hdr", {true, true, Bilinear, Bilinear, ClampToEdge, RGB, true, BITS_32});
	//panoramaSky = textureManager->getHDRImage("skyboxes/panoramas/pisa.hdr", { true, true, Bilinear, Bilinear, ClampToEdge });
	
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBox));

	PanoramaSkyBoxShader* panoramaSkyBoxShader = dynamic_cast<PanoramaSkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBoxPanorama));

	PhongTextureShader* phongShader = dynamic_cast<PhongTextureShader*>
		(shaderManager->getConfig(Shaders::BlinnPhongTex));

	shadowMap = renderer->createDepthMap(4096, 4096);

	pointShadowMap = renderer->createCubeDepthMap(1024, 1024);
	//vsMap = renderer->createVarianceShadowMap(512, 512);
	//vsMapCache = renderer->createVarianceShadowMap(512, 512);
	vsMap = nullptr;
	vsMapCache = nullptr;

	renderTargetMultisampled = renderer->createRenderTarget(8);
	renderTargetSingleSampled = renderer->createRenderTarget();

	skyBoxShader->setSkyTexture(sky);
	panoramaSkyBoxShader->setSkyTexture(panoramaSky);
	phongShader->setSkyBox(sky);


	// Positions of the point lights
	/*pointLightPositions[0] = vec3(7.0f, 2.0f, 20.0f);
	pointLightPositions[1] = vec3(23.f, -33.0f, -40.0f);
	pointLightPositions[2] = vec3(-40.0f, 20.0f, -120.0f);
	pointLightPositions[3] = vec3(0.0f, 0.0f, -30.0f);*/

	vec3 farAway = vec3(0.0f, -1000.0f, 0.0f);

	pointLightPositions[0] = vec3(-3.0f, 2.0f, 0.0f);
	pointLightPositions[1] = farAway;
	pointLightPositions[2] = farAway;
	pointLightPositions[3] = farAway;

	vec3 position = {1.0f, 1.0f, 1.0f };
	position = 5.0f * position;
	globalLight.setPosition(position);
	globalLight.lookAt({0,0,0});
	//globalLight.setOrthoFrustum({-11.5f, 32.8f, -15.0f, 25.0f, 2.0f, 40.0f});
	//globalLight.setLook(normalize(vec3( 0.01f, 1.0f,0.1f )));

	pointLight.setPosition({ -3.0, 2.0f, 0.0 });
	pointLight.setRange(10.0f);
	pointLight.setAspectRatio((float)pointShadowMap->getWidth() / (float)pointShadowMap->getHeight());


	// init shaders
	PhongTextureShader* phongTexShader = dynamic_cast<PhongTextureShader*>
		(renderer->getShaderManager()->getConfig(Shaders::BlinnPhongTex));

	phongTexShader->setLightColor({ 1.0f, 1.0f, 1.0f });
	phongTexShader->setLightDirection(globalLight.getLook());
	phongTexShader->setPointLightPositions(pointLightPositions);

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

void MainLoopTask::setUI(SystemUI* ui)
{
	this->ui = ui;
}

static float frameTimeElapsed = 0;

void MainLoopTask::run()
{
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	ScreenShader* screenShader = dynamic_cast<ScreenShader*>(
		renderer->getShaderManager()->getConfig(Shaders::Screen));
	DepthMapShader* depthMapShader = dynamic_cast<DepthMapShader*>(
		renderer->getShaderManager()->getConfig(Shaders::DepthMap));
	PhongTextureShader* phongShader = dynamic_cast<PhongTextureShader*>
		(renderer->getShaderManager()->getConfig(Shaders::BlinnPhongTex));
	ShadowShader* shadowShader = dynamic_cast<ShadowShader*>
		(renderer->getShaderManager()->getConfig(Shaders::Shadow));
	PointShadowShader* pointShadowShader = dynamic_cast<PointShadowShader*>
		(renderer->getShaderManager()->getConfig(Shaders::ShadowPoint));
	CubeDepthMapShader* cubeDepthMapShader = dynamic_cast<CubeDepthMapShader*>
		(renderer->getShaderManager()->getConfig(Shaders::CubeDepthMap));
	VarianceDepthMapShader* varianceDMShader = dynamic_cast<VarianceDepthMapShader*>
		(renderer->getShaderManager()->getConfig(Shaders::VarianceDepthMap));
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

	renderer->setBackgroundColor({0.5f, 0.5f, 0.5f});

	// render shadows to a depth map
	renderer->useBaseRenderTarget(shadowMap);
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

	phongShader->setLightProjMatrix(globalLight.getProjection(Orthographic));
	phongShader->setLightSpaceMatrix(globalLight.getProjection(Orthographic) * globalLight.getView());
	phongShader->setLightViewMatrix(globalLight.getView());
	
	renderer->cullFaces(CullingMode::Back);
	//renderer->cullFaces(CullingMode::Front);
	drawScene(globalLight.getOrthoProjection(), globalLight.getView(), Shaders::Shadow);
	//drawScene(globalLight.getOrthoProjection(), globalLight.getView(), Shaders::VarianceShadow);
	renderer->cullFaces(CullingMode::Back);
	renderer->endScene();
	//drawScene(&globalLight, ProjectionMode::Perspective, Shaders::Shadow);

	//renderer->useCubeDepthMap(pointShadowMap);
	//pointShadowShader->setLightPosition(pointLight.getPosition());
	//pointShadowShader->setRange(pointLight.getRange());
	//pointShadowShader->setShadowMatrices(pointLight.getMatrices());
	//drawScene(pointLight.getPerspProjection(), pointLight.getView(), Shaders::ShadowPoint);

	// blur the shadow map to smooth out the edges
	//for (int i = 0; i < 1; ++i) blurEffect->blur(vsMap, vsMapCache);
	//blurEffect->blur(vsMap, vsMapCache);

	// now render scene to a offscreen buffer
	renderer->useBaseRenderTarget(renderTargetMultisampled);
	renderer->enableAlphaBlending(true);
	renderer->beginScene();
	phongShader->setShadowMap(shadowMap->getTexture());
	//phongShader->setVarianceShadowMap(vsMap->getTexture());
	//phongShader->setPointLightShadowMap(pointShadowMap);
	phongShader->setPointLightRange(pointLight.getRange());
	phongShader->setViewPosition(camera->getPosition());
	//cubeDepthMapShader->useCubeDepthMap(pointShadowMap->getCubeMap());
	//cubeDepthMapShader->setLightPos(pointLight.getPosition());
	//cubeDepthMapShader->setRange(pointLight.getRange());

	drawSky(camera->getPerspProjection(), camera->getView());
	drawScene(camera->getPerspProjection(), camera->getView());
	//drawScene(camera.get(), ProjectionMode::Perspective, Shaders::CubeDepthMap);


	using r = RenderComponent;
	Dimension blitRegion = { 0,0, window->getWidth(), window->getHeight() };
	renderer->blitRenderTargets(renderTargetMultisampled, renderTargetSingleSampled, blitRegion, r::Color | r::Depth | r::Stencil);
	//ui->frameUpdate();
	//Before presenting the scene, antialise it!
	//SMAA* smaa = renderer->getSMAA();
	//smaa->reset();
	//smaa->antialias(renderTargetSingleSampled); // TODO use render target

	//renderer->endScene();
	
	// finally render the offscreen buffer to a quad and do post processing stuff
	renderer->useScreenTarget();
	renderer->beginScene();
	screenSprite.setTexture(renderTargetSingleSampled->getTexture());
	depthMapShader->useDepthMapTexture(shadowMap->getTexture());
	//varianceDMShader->useVDepthMapTexture(vsMap->getTexture());
	screenShader->useTexture(screenSprite.getTexture());
	if (showDepthMap)
	{
		modelDrawer->draw(&screenSprite, Shaders::DepthMap);
		//modelDrawer->draw(&screenSprite, Shaders::VarianceDepthMap);
	} else
	{
		//modelDrawer->draw(&screenSprite, Shaders::VarianceDepthMap);
		modelDrawer->draw(&screenSprite, Shaders::Screen);
	}
	renderer->endScene();

	window->swapBuffers();
}

void MainLoopTask::drawScene(const mat4& projection, const mat4& view, Shaders shaderType)
{
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	scene->update(frameTimeElapsed);
	scene->draw(renderer, modelDrawer, projection, view, shaderType);
	renderer->endScene();
}

void MainLoopTask::drawSky(const mat4& projection, const mat4& view)
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

void MainLoopTask::updateCamera(Input* input, float deltaTime)
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

void MainLoopTask::handleInputEvents()
{
	using namespace nex;
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

void MainLoopTask::updateWindowTitle(float frameTime, float fps)
{
	runtime += frameTime;
	if (runtime > 1)
	{
		stringstream ss; ss << originalTitle << " : FPS= " << fps;
		window->setTitle(ss.str());
		runtime -= 1;
	}
}

void MainLoopTask::onWindowsFocus(Window* window, bool receivedFocus)
{
	if (receivedFocus)
	{
		LOG(logClient, nex::Debug) << "received focus!";
		isRunning = true;
	} else
	{
		LOG(logClient, nex::Debug) << "lost focus!";
		isRunning = false;
		if (window->isInFullscreenMode())
		{
			window->minimize();
		}
	}
}