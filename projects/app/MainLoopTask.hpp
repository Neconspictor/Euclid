#pragma once
#include <nex/logging/LoggingClient.hpp>
#include <nex/Input.hpp>
#include <nex/event/Task.hpp>
#include <nex/system/Engine.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/Window.hpp>
#include <nex/util/Timer.hpp>
#include <nex/util/FPSCounter.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/model/Vob.hpp>
#include <nex/SubSystemProvider.hpp>
#include <nex/scene/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/sprite/Sprite.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>

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
	nex::LoggingClient logClient;
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
