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
#include <shader/PhongShader.hpp>
#include <camera/FPCamera.hpp>
#include <model/PhongModel.hpp>
#include <shader/PhongTextureShader.hpp>
#include <shader/SkyBoxShader.hpp>

using namespace glm;
using namespace std;
using namespace platform;

MainLoopTask::MainLoopTask(EnginePtr engine, WindowPtr window, RendererPtr renderer, unsigned int flags):
	Task(flags), logClient(getLogServer()), runtime(0), isRunning(true), nanosuitModel("nanosuit-test.obj"), 
	sky(nullptr), skyBox(nullptr)
{
	this->window = window;
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

	auto focusCallback = bind(&MainLoopTask::onWindowsFocus, this, _1, _2);
	auto scrollCallback = bind(&Camera::onScroll, camera.get(), _1);
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

	renderer->getShaderManager()->loadShaders();
	PlaygroundShader* playground = dynamic_cast<PlaygroundShader*>
		(renderer->getShaderManager()->getShader(Playground));
	playground->setTexture1("gun_d.png");
	playground->setTexture2("container.png");

	renderer->getModelManager()->loadModels();

	sky = renderer->getTextureManager()->createCubeMap("skyboxes/sky_right.jpg", "skyboxes/sky_left.jpg", 
		"skyboxes/sky_top.jpg", "skyboxes/sky_bottom.jpg",
		"skyboxes/sky_back.jpg", "skyboxes/sky_front.jpg");

	skyBox = renderer->getModelManager()->createSkyBox();

	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(renderer->getShaderManager()->getShader(SkyBox));

	skyBoxShader->setSkyTexture(sky);
}

static float frameTimeElapsed = 0;

void MainLoopTask::run()
{
	BROFILER_FRAME("MainLoopTask");

	using namespace chrono;
	
	float frameTime = timer.update();
	int millis = static_cast<int> (1.0f /(frameTime * 1000.0f));

	// manual 60 fps cap -> only temporary!
	int minimumMillis = 14;
	if (millis < minimumMillis)
	{
		this_thread::sleep_for(milliseconds(minimumMillis - millis));
		frameTime += timer.update();
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
	window->pollEvents();
	
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
	//renderer->useOffscreenBuffer();
	renderer->useScreenBuffer();
	renderer->beginScene();

	drawScene();

	//renderer->setBackgroundColor({ 0.0f, 0.0f, 0.0f });
	//renderer->useScreenBuffer();
	renderer->useOffscreenBuffer();
	renderer->beginScene();

	// backup camera look direction
	vec3 cameraLook = camera->getLookDirection();

	// draw the scene from rear view now
	camera->setLookDirection(-cameraLook);
	drawScene();

	// restore camera look direction
	camera->setLookDirection(cameraLook);

	renderer->useScreenBuffer();
	renderer->drawOffscreenBuffer();
	//renderer->present();
	BROFILER_CATEGORY("After rendering / before buffer swapping", Profiler::Color::Aqua);
	window->swapBuffers();
}

void MainLoopTask::drawScene()
{
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(renderer->getShaderManager()->getShader(SkyBox));
	PlaygroundShader* playgroundShader = dynamic_cast<PlaygroundShader*>
		(renderer->getShaderManager()->getShader(Playground));
	PhongTextureShader* phongShader = dynamic_cast<PhongTextureShader*>
		(renderer->getShaderManager()->getShader(PhongTex));

	LampShader* lampShader = dynamic_cast<LampShader*>
		(renderer->getShaderManager()->getShader(Lamp));

	ModelManager* modelManager = renderer->getModelManager();

	ModelDrawer* modelDrawer = renderer->getModelDrawer();

	Renderer::Viewport viewport = window->getViewport();

	Model* model = nullptr;

	// Positions of the point lights
	vec3 pointLightPositions[] = {
		vec3(0.7f,  0.2f,  2.0f),
		vec3(2.3f, -3.3f, -4.0f),
		vec3(-4.0f,  2.0f, -12.0f),
		vec3(0.0f,  0.0f, -3.0f)
	};

	camera->calcView();
	mat4 view = camera->getView();
	mat4 projection = perspective(radians(camera->getFOV()), (float)viewport.width / (float)viewport.height, 0.1f, 100.0f);
	mat4 viewProj = projection * view;

	vec3 lightPosition = vec3{ 1.2f, 1.0f, 2.0f };
	Shader::TransformData data = { &projection, &view, nullptr };

	// draw sky
	renderer->enableDepthWriting(false);
	renderer->enableBackfaceDrawing(true);
	mat4 identity;
	mat4 skyBoxView = mat4(mat3(view));
	data.model = &identity;
	data.view = &skyBoxView;
	modelDrawer->draw(*skyBox, skyBoxShader, data);
	renderer->enableDepthWriting(true);
	renderer->enableBackfaceDrawing(false);
	data.view = &view;


	phongShader->setLightColor({ 1.0f, 1.0f, 1.0f });
	Vob cube(SampleMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	Vob phongModel(SampleMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	Vob lampModel(SampleMeshes::CUBE_POSITION_NORMAL_TEX_NAME);
	Vob gunVob("gun.obj");

	cube.setPosition({ 0.0f, 0.0f, 0.0f });
	cube.calcTrafo();

	phongModel.setPosition({ 1.1f, 0.0f, 0.0f });
	phongModel.calcTrafo();
	phongShader->setLightPosition(vec3{ 1.1f, 1.0f, 0.0f });

	lampModel.setPosition({ 1.1f, 1.0f, 0.0f });
	lampModel.setScale({ 0.5f, 0.5f, 0.5f });
	//lampModel.setEulerXYZ({ 0.0f, 0.0f, radians(45.0f) });
	lampModel.calcTrafo();

	playgroundShader->setTextureMixValue(mixValue);
	data.model = &cube.getTrafo();
	model = modelManager->getModel(cube.getMeshName());
	modelDrawer->draw(*model, playgroundShader, data);

	data.model = &lampModel.getTrafo();
	model = modelManager->getModel(lampModel.getMeshName());
	modelDrawer->draw(*model, lampShader, data);

	phongShader->setLightPosition(lightPosition);
	phongShader->setViewPosition(camera->getPosition());
	phongShader->setSpotLightDiection(camera->getLookDirection());
	phongShader->setPointLightPositions(pointLightPositions);
	data.model = &phongModel.getTrafo();
	model = modelManager->getModel(nanosuitModel.getMeshName());
	// the nanosiut uses color information in the alpha channel -> deactivate alpha blending
	renderer->enableAlphaBlending(false);
	modelDrawer->drawOutlined(*model, phongShader, data, vec4(1.0f, 0.5f, 0.1f, 0.3f));
	renderer->enableAlphaBlending(true);
	//modelDrawer->draw(*model, phongShader, data);

	data.model = &gunVob.getTrafo();
	model = modelManager->getModel(gunVob.getMeshName());
	modelDrawer->drawOutlined(*model, phongShader, data, vec4(0.7f, 0.0f, 0.0f, 1.0f));
	modelDrawer->draw(*model, phongShader, data);

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


	if (input->isPressed(Input::KeyEscape))
	{
		window->close();
	}

	if (input->isPressed(Input::KeyEnter) || input->isPressed(Input::KeyReturn))
	{
		window->minimize();
	}

	if (input->isPressed(Input::KeyUp))
	{
		mixValue += 0.1f;
		if (mixValue >= 1.0f)
			mixValue = 1.0f;

		mixValue = round(mixValue * 10) / 10;
		LOG(logClient, Debug) << "MixValue: " << mixValue;
	}

	if (input->isPressed(Input::KeyDown))
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