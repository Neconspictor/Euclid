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

class PBR_Deferred_MainLoopTask
{
public:

	using EnginePtr = Engine*;
	using WindowSystemPtr = SubSystemProvider*;
	using WindowPtr = Window*;
	using RendererPtr = Renderer3D*;
	using GuiPtr = ImGUI_Impl*;
	typedef unsigned int uint;

	PBR_Deferred_MainLoopTask(WindowPtr window, 
								WindowSystemPtr windowSystem, 
								RendererPtr renderer,
								GuiPtr gui);

	SceneNode* createShadowScene();
	SceneNode* createCubeReflectionScene();
	bool getShowDepthMap() const;
	Window* getWindow();
	void init();
	void run(Camera* camera, float frameTime);
	void setShowDepthMap(bool showDepthMap);
	void setUI(SystemUI* ui);
	void setRunning(bool isRunning);
	bool isRunning() const;
	void updateRenderTargets(int width, int height);
	hbao::HBAO* getHBAO();

private:

	// Allow the UI mode classes accessing private members

	GaussianBlur* blurEffect;
	DirectionalLight globalLight;
	GuiPtr gui;
	bool m_isRunning;
	platform::LoggingClient logClient;
	float mixValue;
	std::list<SceneNode> nodes;
	Texture* panoramaSky;

	std::unique_ptr<PBR_Deferred> pbr_deferred;
	std::unique_ptr<PBR_GBuffer>  pbr_mrt;

	std::unique_ptr<SSAO_Deferred> ssao_deferred;
	std::unique_ptr<hbao::HBAO> hbao;

	RendererPtr renderer;
	RenderTarget* renderTargetSingleSampled;
	SceneNode* scene;
	Sprite screenSprite;
	DepthMap* shadowMap;
	bool showDepthMap;
	SystemUI* ui;

	std::list<Vob> vobs;

	WindowPtr window;
	WindowSystemPtr windowSystem;
};