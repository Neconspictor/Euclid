#include <MainLoopTask.hpp>
#include <shader/PlaygroundShader.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <Brofiler.h>
#include <glm/glm.hpp>
#include <mesh/TestMeshes.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <camera/TrackballQuatCamera.hpp>
#include <camera/FPQuaternionCamera.hpp>
#include <shader/SimpleLightShader.hpp>
#include <shader/LampShader.hpp>

using namespace glm;
using namespace std;
using namespace platform;

MainLoopTask::MainLoopTask(EnginePtr engine, WindowPtr window, RendererPtr renderer, unsigned int flags):
	Task(flags), logClient(platform::getLogServer()), runtime(0), isRunning(true)
{
	this->window = window;
	this->renderer = renderer;
	this->engine = engine;
	originalTitle = window->getTitle();
	logClient.setPrefix("[MainLoop]");

	mixValue = 0.2f;

	camera = make_shared<TrackballQuatCamera>(TrackballQuatCamera());
}

void MainLoopTask::init()
{
	using namespace placeholders;

	auto focusCallback = bind(&MainLoopTask::onWindowsFocus, this, _1, _2);
	auto scrollCallback = bind(&Camera::onScroll, camera.get(), _1);
	this->window->addWindowFocusCallback(focusCallback);
	this->window->getInputDevice()->addScrollCallback(scrollCallback);

	//camera.setPosition(vec3(0.0f, 0.0f, 3.0f));
	//camera.setLookDirection(vec3(0.0f, 0.0f, -1.0f));
	//camera.setUpDirection(vec3(0.0f, 1.0f, 0.0f));
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
	playground->setTexture1("jpg.jpg");
	playground->setTexture2("png.png");
}

static float frameTimeElapsed = 0;

void MainLoopTask::run()
{
	BROFILER_FRAME("MainLoopTask");

	using namespace chrono;
		
	float frameTime = timer.update();
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

	Renderer::Viewport viewport = window->getViewport();

	BROFILER_CATEGORY("After input handling / Before rendering", Profiler::Color::AntiqueWhite);

	renderer->beginScene();

	PlaygroundShader* playgroundShader = dynamic_cast<PlaygroundShader*>
		(renderer->getShaderManager()->getShader(Playground));
	SimpleLightShader* simpleLightShader = dynamic_cast<SimpleLightShader*>
		(renderer->getShaderManager()->getShader(SimpleLight));

	LampShader* lampShader = dynamic_cast<LampShader*>
		(renderer->getShaderManager()->getShader(Lamp));

	renderer->getMeshManager()->loadMeshes();

	simpleLightShader->setLightColor({1.0f, 1.0f, 1.0f});
	simpleLightShader->setObjectColor({1.0f, 0.5f, 0.31f});
	Model model(TestMeshes::CUBE_POSITION_UV_NAME);
	Model simpleLitModel(TestMeshes::CUBE_POSITION_NAME);
	Model lampModel(TestMeshes::CUBE_POSITION_NAME);

	model.setPosition({ 0.0f, 0.0f, 0.0f });
	model.calcTrafo();

	simpleLitModel.setPosition({ 1.1f, 0.0f, 0.0f });
	simpleLitModel.calcTrafo();

	lampModel.setPosition({ 1.1f, 1.0f, 0.0f });
	lampModel.setScale({0.5f, 0.5f, 0.5f});
	//lampModel.setEulerXYZ({ 0.0f, 0.0f, radians(45.0f) });
	lampModel.calcTrafo();

	camera->calcView();
	mat4 view = camera->getView();
	mat4 projection = perspective(radians(camera->getFOV()), (float)viewport.width / (float) viewport.height, 0.1f, 100.0f);
	mat4 viewProj = projection * view;

	playgroundShader->setTextureMixValue(mixValue);
	playgroundShader->draw(model, viewProj * model.getTrafo());

	simpleLightShader->draw(simpleLitModel, viewProj * simpleLitModel.getTrafo());
	
	lampShader->draw(lampModel, viewProj * lampModel.getTrafo());

	renderer->endScene();
	renderer->present();
	BROFILER_CATEGORY("After rendering / before buffer swapping", Profiler::Color::Aqua);
	window->swapBuffers();
}

void MainLoopTask::updateCamera(Input* input, float deltaTime)
{
	if (window->hasFocus())
	{
		camera->update(input, deltaTime);
		
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
		std::stringstream ss; ss << originalTitle << " : FPS= " << fps;
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