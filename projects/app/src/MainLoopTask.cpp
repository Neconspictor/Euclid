#include <MainLoopTask.hpp>
#include <shader/PlaygroundShader.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <Brofiler.h>
#include <glm/glm.hpp>
#include <mesh/SampleMeshes.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <camera/TrackballQuatCamera.hpp>
#include <camera/FPQuaternionCamera.hpp>
#include <shader/SimpleLightShader.hpp>
#include <shader/LampShader.hpp>
#include <shader/NormalsShader.hpp>
#include <shader/PhongShader.hpp>
#include <camera/FPCamera.hpp>
#include <model/PhongModel.hpp>
#include <shader/PhongTextureShader.hpp>
#include <shader/SkyBoxShader.hpp>
#include <shader/SimpleReflectionShader.hpp>
#include <platform/SystemUI.hpp>
#include <scene/SceneNode.hpp>

using namespace glm;
using namespace std;
using namespace platform;

MainLoopTask::MainLoopTask(EnginePtr engine, WindowPtr window, WindowSystemPtr windowSystem, RendererPtr renderer, unsigned int flags):
	Task(flags), logClient(getLogServer()), runtime(0), isRunning(true), nanosuitModel("nanosuit_reflection/nanosuit.obj"), 
	sky(nullptr), skyBox(ModelManager::SKYBOX_MODEL_NAME), scene(nullptr), ui(nullptr), asteriodTrafos(nullptr), asteriodSize(0)
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

	vobs.push_back(Vob("planet/planet.obj"));
	planetNode->setVob(&vobs.back());
	planetNode->getVob()->setPosition({ 0.0f, 0.0f, 0.0f });
	planetNode->setDrawingType(DrawingTypes::SOLID);

	asteriodSize = 1000;
	asteriodTrafos = new mat4[asteriodSize];

	srand(chrono::system_clock::now().time_since_epoch().count()); // initialize random seed	
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
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, vec3(scale));
		// 3. Rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, vec3(0.4f, 0.6f, 0.8f));
		// 4. Now add to list of matrices
		asteriodTrafos[i] = model;
	}

	Model* instanced = modelManager->getModel("rock/rock.obj");
	modelManager->useInstances(instanced, asteriodTrafos, asteriodSize);

	nodes.push_back(SceneNode());
	SceneNode* asteriodNode = &nodes.back();
	root->addChild(asteriodNode);
	vobs.push_back(Vob("rock/rock.obj"));
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

	vobs.push_back(Vob("misc/textured_plane.obj"));
	ground->setVob(&vobs.back());
	vobs.push_back(Vob("misc/textured_cube.obj"));
	cube1->setVob(&vobs.back());

	ground->getVob()->setPosition({ 10, 0, 0 });
	cube1->getVob()->setPosition({ 0.0f, 2.0f, 0.0f });
	return root;
}

void MainLoopTask::init()
{
	using namespace placeholders;

	ShaderManager* shaderManager = renderer->getShaderManager();
	ModelManager* modelManager = renderer->getModelManager();
	TextureManager* textureManager = renderer->getTextureManager();

	auto focusCallback = bind(&MainLoopTask::onWindowsFocus, this, _1, _2);
	auto scrollCallback = bind(&Camera::onScroll, camera.get(), _1, _2);
	this->window->addWindowFocusCallback(focusCallback);
	this->window->getInputDevice()->addScrollCallback(scrollCallback);

	camera->setPosition(vec3(0.0f, 3.0f, 3.0f));
	camera->setLookDirection(vec3(1.0f, 0.0f, 0.0f));
	camera->setUpDirection(vec3(0.0f, 1.0f, 0.0f));
	Renderer::Viewport viewport = window->getViewport();
	camera->setHDimension(viewport.width);
	camera->setVDimension(viewport.height);
	camera->setNearPlaneDistance(0.1f);
	camera->setFarPlaneDistance(150.0f);

	window->addResizeCallback([&](int width, int height)
	{
		camera->setHDimension(width);
		camera->setVDimension(height);
	});

	if (TrackballQuatCamera* casted = dynamic_cast<TrackballQuatCamera*>(camera.get()))
	{
		auto cameraResizeCallback = bind(&TrackballQuatCamera::updateOnResize, casted, _1, _2);
		casted->updateOnResize(viewport.width, viewport.height);
		window->addResizeCallback(cameraResizeCallback);
	}
	auto rendererResizeCallback = bind(&Renderer::setViewPort, renderer, 0, 0, _1, _2);

	window->addResizeCallback(rendererResizeCallback);

	shaderManager->loadShaders();
	PlaygroundShader* playground = dynamic_cast<PlaygroundShader*>
		(shaderManager->getShader(Playground));
	playground->setTexture1("gun_d.png");
	playground->setTexture2("container.png");

	modelManager->loadModels();

	/*sky = textureManager->createCubeMap("skyboxes/sky_right.jpg", "skyboxes/sky_left.jpg",
		"skyboxes/sky_top.jpg", "skyboxes/sky_bottom.jpg",
		"skyboxes/sky_back.jpg", "skyboxes/sky_front.jpg", true);
    */
	sky = textureManager->createCubeMap("skyboxes/test/test_right.jpg", "skyboxes/test/test_left.jpg",
		"skyboxes/test/test_top.jpg", "skyboxes/test/test_bottom.jpg",
		"skyboxes/test/test_front.jpg", "skyboxes/test/test_back.jpg", true);
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(shaderManager->getShader(SkyBox));

	SimpleReflectionShader* reflectionShader = dynamic_cast<SimpleReflectionShader*>
		(shaderManager->getShader(SimpleReflection));

	PhongTextureShader* phongShader = dynamic_cast<PhongTextureShader*>
		(shaderManager->getShader(BlinnPhongTex));

	skyBoxShader->setSkyTexture(sky);
	reflectionShader->setReflectionTexture(sky);
	phongShader->setSkyBox(sky);


	// Positions of the point lights
	/*pointLightPositions[0] = vec3(7.0f, 2.0f, 20.0f);
	pointLightPositions[1] = vec3(23.f, -33.0f, -40.0f);
	pointLightPositions[2] = vec3(-40.0f, 20.0f, -120.0f);
	pointLightPositions[3] = vec3(0.0f, 0.0f, -30.0f);*/

	pointLightPositions[0] = vec3(0.0f, 200.0f, 0.0f);
	pointLightPositions[1] = pointLightPositions[0];
	pointLightPositions[2] = pointLightPositions[0];
	pointLightPositions[3] = pointLightPositions[0];

	// init shaders
	PhongTextureShader* phongTexShader = dynamic_cast<PhongTextureShader*>
		(renderer->getShaderManager()->getShader(BlinnPhongTex));

	phongTexShader->setLightColor({ 1.0f, 1.0f, 1.0f });
	phongTexShader->setLightPosition({ 1, 1, 0 });
	phongTexShader->setPointLightPositions(pointLightPositions);

	directionalLightCamera.setPosition({10,10,0});
	directionalLightCamera.setLookDirection({1,1,0});

	// init scene
	scene = createShadowScene();
}

void MainLoopTask::setUI(SystemUI* ui)
{
	this->ui = ui;
}

static float frameTimeElapsed = 0;

void MainLoopTask::run()
{
	BROFILER_FRAME("MainLoopTask");

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
	renderer->useOffscreenBuffer();
	//renderer->useScreenBuffer();
	renderer->beginScene();

	drawScene();

	//renderer->setBackgroundColor({ 0.0f, 0.0f, 0.0f });
	renderer->useScreenBuffer();

	//renderer->useOffscreenBuffer();
	renderer->beginScene();

	// backup camera look direction
	vec3 cameraLook = camera->getLookDirection();

	// draw the scene from rear view now
	camera->setLookDirection(-cameraLook);
	//drawScene();

	// restore camera look direction
	camera->setLookDirection(cameraLook);

	//renderer->useScreenBuffer();
	renderer->drawOffscreenBuffer();
	//renderer->present();

	BROFILER_CATEGORY("After rendering / before buffer swapping", Profiler::Color::Aqua);

	//ui->frameUpdate();
	//Before presenting the scene, antialise it!
	SMAA* smaa = renderer->getSMAA();
	smaa->reset();
	smaa->antialias(renderer->getScreenBuffer());

	window->swapBuffers();
}

void MainLoopTask::drawScene()
{
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(renderer->getShaderManager()->getShader(SkyBox));
	PhongTextureShader* phongTexShader = dynamic_cast<PhongTextureShader*>
		(renderer->getShaderManager()->getShader(BlinnPhongTex));

	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	Renderer::Viewport viewport = window->getViewport();

	camera->calcView();
	const mat4& view = camera->getView();
	//const mat4& projection = perspective(radians(static_cast<float>(camera->getFOV())), (float)viewport.width / (float)viewport.height, 0.1f, 150.0f);
	const mat4& projection = camera->getPerspectiveProjection();
	//const mat4& projection = camera->getOrthogonalProjection();
	mat4 viewProj = projection * view;
	mat4 identity;
	mat4 skyBoxView = mat4(mat3(view));


	// draw sky as last object
	//renderer->enableBackfaceDrawing(true);
	Shader::TransformData data = { &projection, &view, nullptr };
	data.model = &identity;
	data.view = &skyBoxView;
	modelDrawer->draw(&skyBox, skyBoxShader, data);
	//modelDrawer->draw(&skyBox, phongTexShader, data);
	//renderer->enableBackfaceDrawing(false);
	renderer->endScene();

	scene->update(frameTimeElapsed);
	scene->draw(renderer, modelDrawer, projection, view);
}

void MainLoopTask::updateCamera(Input* input, float deltaTime)
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

void MainLoopTask::handleInputEvents()
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