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
#include <light/Light.hpp>

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
	uint asteriodSize;
	glm::mat4* asteriodTrafos;
	std::shared_ptr<Camera> camera;
	FPSCounter counter;
	EnginePtr engine;
	DirectionalLight globalLight;
	bool isRunning;
	platform::LoggingClient logClient;
	float mixValue;
	Vob nanosuitModel;
	std::list<SceneNode> nodes;
	std::string originalTitle;
	Texture* panoramaSky;
	glm::vec3 pointLightPositions[4];
	RendererPtr renderer;
	float runtime;
	SceneNode* scene;
	DepthMap* shadowMap;
	CubeMap* sky;
	Vob skyBox;
	Timer timer;
	SystemUI* ui;
	std::list<Vob> vobs;
	WindowPtr window;
	WindowSystemPtr windowSystem;

	void drawScene(Projectional* projectional, ProjectionMode mode, Shader* shader = nullptr);

	void drawScene(Projectional* projectional, ProjectionMode mode, ShaderEnum shaderType);

	void drawSky(Projectional* projectional, ProjectionMode mode);

	void updateCamera(Input* input, float deltaTime);

	void handleInputEvents();

	void updateWindowTitle(float frameTime, float fps);

	void onWindowsFocus(Window* window, bool receivedFocus);
};
