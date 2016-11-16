#ifndef APP_MAIN_LOOP_TASK_HPP
#define APP_MAIN_LOOP_TASK_HPP
#include <chrono>
#include <platform/logging/LoggingClient.hpp>
#include <platform/Input.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <platform/event/Task.hpp>
#include <system/Engine.hpp>
#include <platform/Renderer.hpp>
#include <platform/Window.hpp>
#include <Brofiler.h>
#include <util/Timer.hpp>
#include <util/FPSCounter.hpp>
#include <glm/glm.hpp>
#include <camera/Camera.hpp>


class MainLoopTask : public Task
{
public:

	using EnginePtr = Engine*;
	using WindowPtr = Window*;
	using RendererPtr = Renderer*;

	MainLoopTask(EnginePtr engine, WindowPtr window, RendererPtr renderer, unsigned int flags = SINGLETHREADED_REPEATING) :
		Task(flags), logClient(platform::getLogServer()), runtime(0)
	{
		this->window = window;
		this->renderer = renderer;
		this->engine = engine;
		originalTitle = window->getTitle();
		logClient.add(platform::makeConsoleEndpoint());
		logClient.setPrefix("[MainLoop]");
		auto focusCallback = bind(&MainLoopTask::onWindowsFocus, this, std::placeholders::_1, std::placeholders::_2);
		auto scrollCallback = bind(&MainLoopTask::onScrolling, this, std::placeholders::_1);
		this->window->addWindowFocusCallback(focusCallback);
		this->window->getInputDevice()->addScrollCallback(scrollCallback);
		mixValue = 0.2f;
	}

	void setLogLevel(platform::LogLevel level)
	{
		logClient.setLogLevel(level);
	};


	void run() override {
		BROFILER_FRAME("MainLoopTask");
		using namespace std;
		using namespace chrono;
		using namespace platform;
		
		long double frameTime = timer.update();
		long double fps = counter.update(frameTime);

		updateWindowTitle(frameTime, fps);

		if (!window->isOpen())
		{
			engine->stop();
			return;
		}

		handleInputEvents();

		BROFILER_CATEGORY("After input handling / Before rendering", Profiler::Color::AntiqueWhite);

		renderer->beginScene();
		renderer->endScene();
		renderer->present();
		BROFILER_CATEGORY("After rendering / before buffer swapping", Profiler::Color::Aqua);
		window->swapBuffers();
	};

private:
	WindowPtr window;
	RendererPtr renderer;
	EnginePtr engine;
	platform::LoggingClient logClient;
	Timer timer;
	FPSCounter counter;
	long double runtime;
	std::string originalTitle;
	Camera camera;
	float mixValue;

	void doUserMovement(Input* input, double deltaTime)
	{
		using namespace glm;
		// camera movements
		float cameraSpeed = 5.0f * deltaTime;
		vec3 cameraPos = camera.getPosition();
		vec3 cameraLook = camera.getLookDirection();
		vec3 cameraUp = camera.getUpDirection();
		vec3 cameraRight = normalize(cross(cameraLook, cameraUp));

		if (input->isDown(Input::KeyW))
			cameraPos += cameraSpeed * cameraLook;

		if (input->isDown(Input::KeyS))
			cameraPos -= cameraSpeed * cameraLook;

		if (input->isDown(Input::KeyD))
			cameraPos += cameraSpeed * cameraRight;

		if (input->isDown(Input::KeyA))
			cameraPos -= cameraSpeed * cameraRight;

		camera.setPosition(cameraPos);
		camera.setLookDirection(cameraLook);
		camera.setUpDirection(cameraUp);
	}

	void handleInputEvents()
	{
		using namespace platform;
		BROFILER_CATEGORY("Before input handling", Profiler::Color::AliceBlue);
		window->pollEvents();
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
	};

	void updateWindowTitle(long double frameTime, long double fps)
	{
		runtime += frameTime;
		if (runtime > 1)
		{
			std::stringstream ss; ss << originalTitle << " : FPS= " << (int)fps;
			window->setTitle(ss.str());
			runtime -= 1;
		}
	};

	void onScrolling(float scrollAmount)
	{
		LOG(logClient, platform::Debug) << "user is scrolling with strength: " << scrollAmount;
	}

	 void onWindowsFocus(Window* window, bool receivedFocus)
	{
		if (receivedFocus)
			LOG(logClient, platform::Debug) << "received focus!";
		else
			LOG(logClient, platform::Debug) << "lost focus!";
	}
};

#endif