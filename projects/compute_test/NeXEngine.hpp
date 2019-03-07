#pragma once
#include <renderer/ComputeTest_Renderer.hpp>
#include <gui/SceneGUI.hpp>
#include <nex/util/Timer.hpp>
#include <nex/util/FPSCounter.hpp>
#include <nex/config/Configuration.hpp>
#include <VideoConfig.hpp>
#include <nex/RenderBackend.hpp>
#include "nex/FileSystem.hpp"

namespace nex
{
	class SubSystemProvider;
	class SubSystemProviderGLFW;


	class NeXEngine
	{
	public:

		NeXEngine(SubSystemProvider* provider);
		virtual ~NeXEngine();

		nex::LogLevel getLogLevel() const;

		void init();

		bool isRunning() const;

		void run();

		void setConfigFileName(const char* fileName);

		void setRunning(bool isRunning);

	protected:

		SceneNode * createScene();
		Window* createWindow();
		void initRenderBackend();
		void readConfig();
		void setupCallbacks();
		void setupGUI();
		void setupCamera();
		void updateWindowTitle(float frameTime, float fps);
	private:
		nex::Logger m_logger;
		std::unique_ptr<ComputeTest_Renderer> m_renderer;
		std::unique_ptr<gui::ControllerStateMachine> m_controllerSM;
		std::unique_ptr<Camera> m_camera;
		SubSystemProvider* m_windowSystem;
		std::unique_ptr<gui::ImGUI_Impl> m_gui;
		Window* m_window;
		Input* m_input;
		std::string m_baseTitle;
		Timer m_timer;
		FPSCounter m_counter;
		std::list<SceneNode> m_nodes;
		std::list<Vob> m_vobs;
		SceneNode* m_scene;
		bool m_isRunning;

		Configuration m_config;
		VideoConfig m_video;
		std::string m_configFileName;
		std::string m_systemLogLevelStr;
		nex::LogLevel m_systemLogLevel;

		FileSystem mMeshFileSystem;
		FileSystem mShaderFileSystem;
		FileSystem mTextureFileSystem;
	};
}
