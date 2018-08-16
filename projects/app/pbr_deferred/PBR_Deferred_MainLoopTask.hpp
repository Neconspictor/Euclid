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
#include <gui/ControllerStateMachine.hpp>
#include "gui/Style.hpp"

class SystemUI;

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
	void run() override;
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
	std::unique_ptr<nex::engine::gui::Style> style;
	bool isRunning;
	platform::LoggingClient logClient;
	float mixValue;
	std::list<SceneNode> nodes;
	std::string originalTitle;
	Texture* panoramaSky;

	std::unique_ptr<PBR_Deferred> pbr_deferred;
	std::unique_ptr<PBR_GBuffer>  pbr_mrt;

	std::unique_ptr<SSAO_Deferred> ssao_deferred;
	std::unique_ptr<hbao::HBAO> hbao;

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

	ControllerStateMachine uiModeStateMachine;

	WindowPtr window;
	WindowSystemPtr windowSystem;

	void drawScene(const glm::mat4& projection, const glm::mat4& view, Shaders shaderType = Shaders::Unknown);

	void updateWindowTitle(float frameTime, float fps);

	void setupCallbacks();

	void onWindowsFocus(Window * window, bool receivedFocus);
};