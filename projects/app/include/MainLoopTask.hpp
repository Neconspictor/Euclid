#ifndef APP_MAIN_LOOP_TASK_HPP
#define APP_MAIN_LOOP_TASK_HPP
#include <chrono>
#include <platform/logging/LoggingClient.hpp>
#include <platform/Input.hpp>
#include <platform/event/Task.hpp>
#include <system/Engine.hpp>
#include <renderer/Renderer3D.hpp>
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
	using RendererPtr = Renderer3D*;

	MainLoopTask(EnginePtr engine, WindowPtr window, RendererPtr renderer, 
		unsigned int flags = SINGLETHREADED_REPEATING);

	virtual void run() override;

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

	void doUserMovement(Input* input, double deltaTime);

	void handleInputEvents();

	void updateWindowTitle(long double frameTime, long double fps);

	void onWindowsFocus(Window* window, bool receivedFocus) const;
};

#endif