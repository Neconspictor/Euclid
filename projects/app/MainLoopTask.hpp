#pragma once
#include <platform/logging/LoggingClient.hpp>
#include <platform/Input.hpp>
#include <platform/event/Task.hpp>
#include <system/Engine.hpp>
#include <renderer/RenderBackend.hpp>
#include <platform/Window.hpp>
#include <util/Timer.hpp>
#include <util/FPSCounter.hpp>
#include <camera/Camera.hpp>
#include <model/Vob.hpp>
#include <platform/SubSystemProvider.hpp>
#include <scene/SceneNode.hpp>
#include <light/Light.hpp>
#include <sprite/Sprite.hpp>
#include <post_processing/blur/GaussianBlur.hpp>

class SystemUI;

class MainLoopTask : public Task
{
public:

	using EnginePtr = Engine*;
	using WindowSystemPtr = SubSystemProvider*;
	using WindowPtr = Window*;
	using RendererPtr = RenderBackend*;
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
	GaussianBlur* blurEffect;
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
	PointLight pointLight;
	glm::vec3 pointLightPositions[4];
	CubeDepthMap* pointShadowMap;
	RendererPtr renderer;
	RenderTarget* renderTargetMultisampled;
	RenderTarget* renderTargetSingleSampled;
	float runtime;
	SceneNode* scene;
	Sprite screenSprite;
	DepthMap* shadowMap;
	bool showDepthMap;
	CubeMap* sky;
	Vob skyBox;
	Timer timer;
	SystemUI* ui;
	RenderTarget* vsMap;
	RenderTarget* vsMapCache;
	std::list<Vob> vobs;
	WindowPtr window;
	WindowSystemPtr windowSystem;

	void drawScene(const glm::mat4& projection, const glm::mat4& view, Shaders shaderType = Shaders::Unknown);

	void drawSky(const glm::mat4& projection, const glm::mat4& view);

	void updateCamera(Input* input, float deltaTime);

	void handleInputEvents();

	void updateWindowTitle(float frameTime, float fps);

	void onWindowsFocus(Window* window, bool receivedFocus);
};
