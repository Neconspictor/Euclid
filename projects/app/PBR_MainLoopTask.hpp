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
#include <nex/shading_model/PBR.hpp>

class SystemUI;

class PBR_MainLoopTask : public Task
{
public:

	using EnginePtr = Engine*;
	using WindowSystemPtr = SubSystemProvider*;
	using WindowPtr = Window*;
	using RendererPtr = RenderBackend*;
	typedef unsigned int uint;

	PBR_MainLoopTask(EnginePtr engine, WindowPtr window, WindowSystemPtr windowSystem, RendererPtr renderer,
		unsigned int flags = SINGLETHREADED_REPEATING);

	SceneNode* createShadowScene();

	SceneNode* createCubeReflectionScene();

	void init();

	void setUI(SystemUI* ui);

	virtual void run() override;

private:

	GaussianBlur* blurEffect;
	std::shared_ptr<Camera> camera;
	FPSCounter counter;
	EnginePtr engine;
	DirectionalLight globalLight;
	bool isRunning;
	nex::LoggingClient logClient;
	float mixValue;
	std::list<SceneNode> nodes;
	std::string originalTitle;
	Texture* panoramaSky;

	std::unique_ptr<PBR> pbr;

	RendererPtr renderer;
	RenderTarget* renderTargetMultisampled;
	RenderTarget* renderTargetSingleSampled;
	float runtime;
	SceneNode* scene;
	Sprite screenSprite;
	DepthMap* shadowMap;
	bool showDepthMap;
	Timer timer;
	SystemUI* ui;

	std::list<Vob> vobs;
	WindowPtr window;
	WindowSystemPtr windowSystem;

	void updateCamera(Input* input, float deltaTime);

	void handleInputEvents();

	void updateWindowTitle(float frameTime, float fps);

	void onWindowsFocus(Window* window, bool receivedFocus);
};