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

class MainLoopTask : public Task
{
public:

	using EnginePtr = Engine*;
	using WindowPtr = Window*;
	using RendererPtr = Renderer*;

	MainLoopTask(EnginePtr engine, WindowPtr window, RendererPtr renderer, unsigned int flags = SINGLETHREADED_REPEATING) :
		Task(flags), logClient(platform::getLogServer()), runtime(0), FPScounter(0)
	{
		this->window = window;
		this->renderer = renderer;
		this->engine = engine;
		logClient.add(platform::makeConsoleEndpoint());
		logClient.setPrefix("[MainLoop]");
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
		auto currentFrameTime = timer.now();
		if (!lastFrameTime.time_since_epoch().count())
		{
			lastFrameTime = currentFrameTime;
			FPScounter = 0;
			runtime = 0;
		}

		auto frameDiff = duration_cast<duration<double>>(currentFrameTime - lastFrameTime);
		lastFrameTime = currentFrameTime;
		runtime += frameDiff.count();
		FPScounter += 1;

		if (runtime > 1)
		{
			LOG(logClient, Debug) << "current fps: " << FPScounter;
			runtime -= 1;
			FPScounter = 0;
		}

		if (!window->isOpen())
		{
			engine->stop();
			return;
		}

		BROFILER_CATEGORY("Before input handling", Profiler::Color::AliceBlue);
		window->pollEvents();

		Input* input = window->getInputDevice();

		if (input->isPressed(Input::KeyEscape))
		{
			LOG(logClient, platform::Warning) << "Escape is pressed!";
		}

		if (input->isPressed(Input::KeyKpAdd))
		{
			LOG(logClient, platform::Warning) << "Numpad add key is pressed!";
		}

		if (input->isPressed(Input::KeyEnter))
		{
			LOG(logClient, Warning) << "Enter is pressed!";
		}


		if (input->isPressed(Input::KeyEscape))
		{
			window->close();
		}

		BROFILER_CATEGORY("After input handling / Before rendering", Profiler::Color::AntiqueWhite);

		renderer->beginScene();
		renderer->endScene();
		renderer->present();
		BROFILER_CATEGORY("After rendering / before buffer swapping", Profiler::Color::Aqua);
		window->swapBuffers();
		BROFILER_CATEGORY("After buffer swapping", Profiler::Color::Aquamarine);
	};

private:
	WindowPtr window;
	RendererPtr renderer;
	EnginePtr engine;
	platform::LoggingClient logClient;
	std::chrono::high_resolution_clock timer;
	std::chrono::time_point<std::chrono::steady_clock> lastFrameTime;
	double runtime;
	int FPScounter;
};

#endif