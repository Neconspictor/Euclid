#pragma once
#include <pbr_deferred/PBR_Deferred_Renderer.hpp>
#include <gui/SceneGUI.hpp>
#include <nex/util/Timer.hpp>
#include <nex/util/FPSCounter.hpp>
#include <nex/config/Configuration.hpp>
#include <VideoConfig.hpp>
#include <nex/FileSystem.hpp>
#include "Globals.hpp"

namespace glm
{
	inline std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
	{
		os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
		return os;
	}
}

namespace nex
{

	class SubSystemProvider;
	class SubSystemProviderGLFW;
	class Cursor;
	class Window;

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

		void collectRenderCommands(RenderCommandQueue* queue, const Scene& scene);
		void pickingTest( const Scene& scene);
		std::unique_ptr<Mesh> createBoundingBoxMesh();
		std::unique_ptr<Mesh> createLineMesh();
		void createScene();
		Window* createWindow();
		void initLights();
		void initPbr();
		void initProbes();
		void initRenderBackend();
		void readConfig();
		void setupCallbacks();
		void setupGUI();
		void setupCamera();
		void updateWindowTitle(float frameTime, float fps);
	private:
		util::Globals mGlobals;
		nex::Logger mLogger;
		std::unique_ptr<PBR_Deferred_Renderer> mRenderer;
		std::unique_ptr<gui::ControllerStateMachine> mControllerSM;
		std::unique_ptr<PerspectiveCamera> mCamera;
		SubSystemProvider* mWindowSystem;
		std::unique_ptr<gui::ImGUI_Impl> mGui;
		Window* mWindow;
		std::unique_ptr<Cursor> mCursor;
		Input* mInput;
		std::string mBaseTitle;
		Timer mTimer;
		FPSCounter mCounter;
		Scene mScene;
		std::list<std::unique_ptr<StaticMeshContainer>> mModels;
		bool mIsRunning;

		Configuration mConfig;
		VideoConfig mVideo;
		std::string mConfigFileName;
		std::string mSystemLogLevelStr;
		nex::LogLevel mSystemLogLevel;

		std::unique_ptr<nex::FileSystem> mShaderFileSystem;

		std::unique_ptr<CascadedShadow> mCascadedShadow;
		std::unique_ptr<PbrProbe> mPbrProbe;
		std::unique_ptr<PbrTechnique> mPbrTechnique;

		AmbientLight mAmbientLight;
		DirectionalLight mSun;
		Texture* panoramaSky;

		SceneNode* mBoundingBoxNode;
		SceneNode* mLineNode;
	};
}
