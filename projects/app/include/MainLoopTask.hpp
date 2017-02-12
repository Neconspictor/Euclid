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
#include <scene/SceneNode.hpp>

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

	SceneNode* createAsteriodField();
	SceneNode* createShadowScene();
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
	Camera directionalLightCamera;
	float mixValue;
	bool isRunning;
	Vob nanosuitModel;
	CubeMap* sky;
	Texture* panoramaSky;
	Vob skyBox;
	std::list<SceneNode> nodes;
	std::list<Vob> vobs;
	glm::vec3 pointLightPositions[4];
	SceneNode* scene;
	SystemUI* ui;

	glm::mat4* asteriodTrafos;
	uint asteriodSize;

	void drawScene();

	void updateCamera(Input* input, float deltaTime);

	void handleInputEvents();

	void updateWindowTitle(float frameTime, float fps);

	void onWindowsFocus(Window* window, bool receivedFocus);
};
