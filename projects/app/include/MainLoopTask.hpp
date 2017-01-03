#pragma once
#include <platform/logging/LoggingClient.hpp>
#include <platform/Input.hpp>
#include <platform/event/Task.hpp>
#include <system/Engine.hpp>
#include <renderer/Renderer3D.hpp>
#include <platform/Window.hpp>
#include <util/Timer.hpp>
#include <util/FPSCounter.hpp>
#include <camera/Camera.hpp>
#include <model/Vob.hpp>
#include <platform/WindowSystem.hpp>


class SystemUI;

class MainLoopTask : public Task
{
public:

	using EnginePtr = Engine*;
	using WindowSystemPtr = WindowSystem*;
	using WindowPtr = Window*;
	using RendererPtr = Renderer3D*;
	typedef unsigned int uint;

	MainLoopTask(EnginePtr engine, WindowPtr window, WindowSystemPtr windowSystem, RendererPtr renderer,
		unsigned int flags = SINGLETHREADED_REPEATING);

	void init();

	void setUI(SystemUI* ui);

	virtual void run() override;

private:
	WindowSystemPtr windowSystem;
	WindowPtr window;
	RendererPtr renderer;
	EnginePtr engine;
	platform::LoggingClient logClient;
	Timer timer;
	FPSCounter counter;
	float runtime;
	std::string originalTitle;
	std::shared_ptr<Camera> camera;
	float mixValue;
	bool isRunning;
	Vob nanosuitModel;
	CubeMap* sky;
	Model* skyBox;
	SystemUI* ui;

	glm::mat4* asteriodTrafos;
	uint asteriodSize;

	void drawAsteriods(glm::mat4* asteriodTrafos, uint asteriodSize);

	void drawScene();

	void updateCamera(Input* input, float deltaTime);

	void handleInputEvents();

	void updateWindowTitle(float frameTime, float fps);

	void onWindowsFocus(Window* window, bool receivedFocus);
};
