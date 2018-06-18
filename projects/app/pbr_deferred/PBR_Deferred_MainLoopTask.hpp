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
#include <platform/PlatformProvider.hpp>
#include <scene/SceneNode.hpp>
#include <light/Light.hpp>
#include <sprite/Sprite.hpp>
#include <post_processing/blur/GaussianBlur.hpp>
#include <shading_model/PBR_Deferred.hpp>
#include <post_processing/SSAO.hpp>
#include <post_processing/HBAO.hpp>
#include <platform/gui/ImGUI.hpp>
#include <ui_mode/UI_ModeStateMachine.hpp>

class SystemUI;
class GUI_Mode;
class CameraMode;
class BaseGUI_Mode;

class PBR_Deferred_MainLoopTask : public Task
{
public:

	using EnginePtr = Engine*;
	using WindowSystemPtr = SubSystemProvider*;
	using WindowPtr = Window*;
	using RendererPtr = Renderer3D*;
	using GuiPtr = ImGUI_Impl*;
	typedef unsigned int uint;

	PBR_Deferred_MainLoopTask(EnginePtr engine, 
								WindowPtr window, 
								WindowSystemPtr windowSystem, 
								RendererPtr renderer,
								GuiPtr gui,
		unsigned int flags = SINGLETHREADED_REPEATING);

	virtual ~PBR_Deferred_MainLoopTask();

	SceneNode* createShadowScene();
	SceneNode* createCubeReflectionScene();
	Camera* getCamera();
	bool getShowDepthMap() const;
	Timer* getTimer();
	Window* getWindow();
	void init();
	virtual void run() override;
	void setShowDepthMap(bool showDepthMap);
	void setUI(SystemUI* ui);

private:

	// Allow the UI mode classes accessing private members

	GaussianBlur* blurEffect;
	std::shared_ptr<Camera> camera;
	FPSCounter counter;
	EnginePtr engine;
	DirectionalLight globalLight;
	GuiPtr gui;
	bool isRunning;
	platform::LoggingClient logClient;
	float mixValue;
	std::list<SceneNode> nodes;
	std::string originalTitle;
	Texture* panoramaSky;

	std::unique_ptr<PBR_Deferred> pbr_deferred;
	std::unique_ptr<PBR_GBuffer>  pbr_mrt;

	std::unique_ptr<SSAO_Deferred> ssao_deferred;
	std::unique_ptr<hbao::HBAO_Deferred> hbao_deferred;

	RendererPtr renderer;
	RenderTarget* renderTargetSingleSampled;
	float runtime;
	SceneNode* scene;
	Sprite screenSprite;
	DepthMap* shadowMap;
	bool showDepthMap;
	Timer timer;
	SystemUI* ui;

	std::list<Vob> vobs;

	UI_ModeStateMachine uiModeStateMachine;

	WindowPtr window;
	WindowSystemPtr windowSystem;

	void drawScene(const glm::mat4& projection, const glm::mat4& view, Shaders shaderType = Shaders::Unknown);

	void updateWindowTitle(float frameTime, float fps);

	void setupCallbacks();

	void onWindowsFocus(Window * window, bool receivedFocus);
};

class BaseGUI_Mode : public UI_Mode {
public:
	BaseGUI_Mode(PBR_Deferred_MainLoopTask& mainTask, ImGUI_Impl& guiRenderer);
	virtual ~BaseGUI_Mode() = default;

	virtual void drawGUI() override;
	virtual void frameUpdate(UI_ModeStateMachine& stateMachine) override;
	virtual void init() override;

protected:
	void handleExitEvent();

protected:
	PBR_Deferred_MainLoopTask* mainTask;
	ImGUI_Impl* guiRenderer;
	platform::LoggingClient logClient;
};

class GUI_Mode : public BaseGUI_Mode {
public:
	GUI_Mode(PBR_Deferred_MainLoopTask& mainTask, ImGUI_Impl& guiRenderer);
	virtual ~GUI_Mode() = default;
	virtual void frameUpdate(UI_ModeStateMachine& stateMachine) override;
};

class CameraMode : public BaseGUI_Mode {
public:
	CameraMode(PBR_Deferred_MainLoopTask& mainTask, ImGUI_Impl& guiRenderer);
	virtual ~CameraMode() = default;
	virtual void frameUpdate(UI_ModeStateMachine& stateMachine) override;

private:
	void updateCamera(Input* input, float deltaTime);
};

