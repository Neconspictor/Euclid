#pragma once
#include <nex/system/Engine.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <pbr_deferred/PBR_Deferred_MainLoopTask.hpp>
#include <gui/SceneGUI.hpp>
#include <nex/system/Video.hpp>
#include <nex/util/Timer.hpp>
#include <nex/util/FPSCounter.hpp>

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
	nex::LoggingClient m_logClient;
	std::unique_ptr<RenderBackend> m_renderer;
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
