#pragma once
#include <system/Engine.hpp>
#include <renderer/RendererOpenGL.hpp>
#include <PBR_MainLoopTask.hpp>
#include <pbr_deferred/PBR_Deferred_MainLoopTask.hpp>
#include <MainLoopTask.hpp>
#include <gui/SceneGUI.hpp>
#include "system/Video.hpp"

class Main
{
public:

	Main();
	virtual ~Main();

	void run();

protected:
	SceneNode * createScene();
	void readConfig();
	void setupCallbacks();
	void setupGUI();
	void setupCamera();
	void updateWindowTitle(float frameTime, float fps);
private:
	platform::LoggingClient m_logClient;
	std::unique_ptr<Renderer3D> m_renderer;
	std::unique_ptr<PBR_Deferred_MainLoopTask> m_task;
	std::unique_ptr<ControllerStateMachine> m_controllerSM;
	std::unique_ptr<Camera> m_camera;
	SubSystemProvider* m_windowSystem;
	std::unique_ptr<Engine> m_engine;
	std::unique_ptr<ImGUI_Impl> m_gui;
	std::shared_ptr<Video> m_video;
	Window* m_window;
	Input* m_input;
	std::string m_baseTitle;
	Timer m_timer;
	FPSCounter m_counter;
	std::list<SceneNode> m_nodes;
	std::list<Vob> m_vobs;
	SceneNode* m_scene;
};