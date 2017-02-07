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

using namespace glm;
using namespace std;
using namespace platform;

MainLoopTask::MainLoopTask(EnginePtr engine, WindowPtr window, WindowSystemPtr windowSystem, RendererPtr renderer, unsigned int flags):
	Task(flags), logClient(getLogServer()), runtime(0), isRunning(true), nanosuitModel("nanosuit_reflection/nanosuit.obj"), 
	sky(nullptr), skyBox(nullptr)
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

	camera->setPosition(vec3(0.0f, 0.0f, 3.0f));
	camera->setLookDirection(vec3(0.0f, 0.0f, -1.0f));
	camera->setUpDirection(vec3(0.0f, 1.0f, 0.0f));
	Renderer::Viewport viewport = window->getViewport();

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

	sky = textureManager->createCubeMap("skyboxes/sky_right.jpg", "skyboxes/sky_left.jpg",
		"skyboxes/sky_top.jpg", "skyboxes/sky_bottom.jpg",
		"skyboxes/sky_back.jpg", "skyboxes/sky_front.jpg", true);

	skyBox = modelManager->createSkyBox();

	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(shaderManager->getShader(SkyBox));

	SimpleReflectionShader* reflectionShader = dynamic_cast<SimpleReflectionShader*>
		(shaderManager->getShader(SimpleReflection));

	PhongTextureShader* phongShader = dynamic_cast<PhongTextureShader*>
		(shaderManager->getShader(BlinnPhongTex));

	skyBoxShader->setSkyTexture(sky);
	reflectionShader->setReflectionTexture(sky);
	phongShader->setSkyBox(sky);


	asteriodSize = 100;
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

	//drawScene();
	drawAsteriods(asteriodTrafos, asteriodSize);

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

void MainLoopTask::drawAsteriods(mat4* asteriodTrafos, uint asteriodSize)
{
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(renderer->getShaderManager()->getShader(SkyBox));
	PhongTextureShader* phongTexShader = dynamic_cast<PhongTextureShader*>
		(renderer->getShaderManager()->getShader(BlinnPhongTex));

	ModelManager* modelManager = renderer->getModelManager();
	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	Renderer::Viewport viewport = window->getViewport();

	// Positions of the point lights
	vec3 pointLightPositions[] = {
		vec3(7.0f,  2.0f,  20.0f),
		vec3(23.f, -33.0f, -40.0f),
		vec3(-40.0f,  20.0f, -120.0f),
		vec3(0.0f,  0.0f, -30.0f)
	};

	Model* model = nullptr;

	camera->calcView();
	mat4 view = camera->getView();
	mat4 projection = perspective(radians(static_cast<float>(camera->getFOV())), (float)viewport.width / (float)viewport.height, 0.1f, 150.0f);
	mat4 viewProj = projection * view;
	mat4 identity;
	mat4 skyBoxView = mat4(mat3(view));

	Shader::TransformData data = { &projection, &view, nullptr };

	Vob planet("planet/planet.obj");
	Vob rock("rock/rock.obj");
	planet.setPosition({ 0.0f, 0.0f, 0.0f });
	planet.calcTrafo();

	phongTexShader->setLightPosition(vec3{ 1.1f, 1.0f, 0.0f });


	phongTexShader->setLightPosition({ 0,1,-3 });
	phongTexShader->setViewPosition(camera->getPosition());
	phongTexShader->setSpotLightDiection(camera->getLookDirection());
	phongTexShader->setPointLightPositions(pointLightPositions);
	data.model = &planet.getTrafo();
	model = modelManager->getModel(planet.getMeshName());
	modelDrawer->draw(*model, phongTexShader, data);



	//rock.setTrafo(asteriodTrafos[i]);
	rock.calcTrafo();
	data.model = &rock.getTrafo();
	model = modelManager->getModel(rock.getMeshName());
	modelDrawer->drawInstanced(*model, phongTexShader, data, asteriodSize);
	renderer->endScene();

	// draw sky as last object
	renderer->enableBackfaceDrawing(true);
	data.model = &identity;
	data.view = &skyBoxView;
	modelDrawer->draw(*skyBox, skyBoxShader, data);
	renderer->enableBackfaceDrawing(false);
	renderer->endScene();
}

void MainLoopTask::drawScene()
{
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(renderer->getShaderManager()->getShader(SkyBox));
	PlaygroundShader* playgroundShader = dynamic_cast<PlaygroundShader*>
		(renderer->getShaderManager()->getShader(Playground));
	PhongTextureShader* phongTexShader = dynamic_cast<PhongTextureShader*>
		(renderer->getShaderManager()->getShader(PhongTex));

	PhongShader* phongShader = dynamic_cast<PhongShader*>
		(renderer->getShaderManager()->getShader(Phong));

	LampShader* lampShader = dynamic_cast<LampShader*>
		(renderer->getShaderManager()->getShader(Lamp));

	SimpleReflectionShader* reflectionShader = dynamic_cast<SimpleReflectionShader*>
		(renderer->getShaderManager()->getShader(SimpleReflection));

	NormalsShader* normalsShader = dynamic_cast<NormalsShader*>
		(renderer->getShaderManager()->getShader(Normals));

	ModelManager* modelManager = renderer->getModelManager();

	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	Renderer::Viewport viewport = window->getViewport();

	Model* model = nullptr;

	// Positions of the point lights
	vec3 pointLightPositions[] = {
		vec3(7.0f,  2.0f,  20.0f),
		vec3(23.f, -33.0f, -40.0f),
		vec3(-40.0f,  20.0f, -120.0f),
		vec3(0.0f,  0.0f, -30.0f)
	};

	camera->calcView();
	mat4 view = camera->getView();
	mat4 projection = perspective(radians(static_cast<float>(camera->getFOV())), (float)viewport.width / (float)viewport.height, 0.1f, 100.0f);
	mat4 viewProj = projection * view;
	mat4 identity;
	mat4 skyBoxView = mat4(mat3(view));

	vec3 lightPosition = vec3{ 12.0f, 10.0f, 20.0f };
	Shader::TransformData data = { &projection, &view, nullptr };

	reflectionShader->setCameraPosition(camera->getPosition());

	//renderer->enableDepthWriting(false);
	//renderer->enableBackfaceDrawing(true);
	//renderer->enableDepthWriting(true);
	//renderer->enableBackfaceDrawing(false);


	phongTexShader->setLightColor({ 1.0f, 1.0f, 1.0f });
	Vob cube(SampleMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	Vob phongModel(SampleMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	Vob lampModel(SampleMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	Vob gunVob("gun.obj");

	cube.setPosition({ 0.0f, 0.0f, 0.0f });
	cube.calcTrafo();

	phongModel.setPosition({ 1.1f, 0.0f, 0.0f });
	phongModel.calcTrafo();
	phongTexShader->setLightPosition(vec3{ 1.1f, 1.0f, 0.0f });

	lampModel.setPosition(lightPosition);
	lampModel.setScale({ 0.5f, 0.5f, 0.5f });
	//lampModel.setEulerXYZ({ 0.0f, 0.0f, radians(45.0f) });
	lampModel.calcTrafo();

	playgroundShader->setTextureMixValue(mixValue);
	data.model = &cube.getTrafo();
	//model = modelManager->getModel(cube.getMeshName());
	model = modelManager->getModel(gunVob.getMeshName());
	renderer->enableBackfaceDrawing(true);
	//modelDrawer->draw(*model, playgroundShader, data);
	renderer->enableBackfaceDrawing(false);

	data.model = &lampModel.getTrafo();
	model = modelManager->getModel(lampModel.getMeshName());
	renderer->enableBackfaceDrawing(true);
	//modelDrawer->draw(*model, lampShader, data);
	renderer->enableBackfaceDrawing(false);

	phongTexShader->setLightPosition({0,1,-3});
	phongTexShader->setViewPosition(camera->getPosition());
	phongTexShader->setSpotLightDiection(camera->getLookDirection());
	phongTexShader->setPointLightPositions(pointLightPositions);
	data.model = &phongModel.getTrafo();
	model = modelManager->getModel("rock/rock.obj");
	//model = modelManager->getModel(nanosuitModel.getMeshName());


	phongShader->setLightColor({ 1,1,1 });
	phongShader->setLightPosition({10,10,10});
	phongShader->setMaterial(PhongMaterial({0,0,0,1}, {1,0,0,1}, {1,1,1,1}, 32));

	// the nanosiut uses color information in the alpha channel -> deactivate alpha blending
	renderer->enableAlphaBlending(false);
	modelDrawer->draw(*model, phongTexShader, data);
	renderer->enableAlphaBlending(true);
	//modelDrawer->draw(*model, reflectionShader, data);

	data.model = &gunVob.getTrafo();
	model = modelManager->getModel(gunVob.getMeshName());
	//modelDrawer->draw(*model, reflectionShader, data);
	//modelDrawer->drawOutlined(*model, phongShader, data, vec4(0.7f, 0.0f, 0.0f, 1.0f));
	//modelDrawer->draw(*model, phongTexShader, data);

	normalsShader->setNormalColor({1,1,0,1});
	data.model = &gunVob.getTrafo();
	model = modelManager->getModel(gunVob.getMeshName());
	//modelDrawer->draw(*model, normalsShader, data);


	// draw sky as last object
	renderer->enableBackfaceDrawing(true);
	data.model = &identity;
	data.view = &skyBoxView;
	modelDrawer->draw(*skyBox, skyBoxShader, data);
	renderer->enableBackfaceDrawing(false);
	renderer->endScene();
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