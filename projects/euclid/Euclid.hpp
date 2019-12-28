#pragma once
#include <PBR_Deferred_Renderer.hpp>
#include <nex/util/Timer.hpp>
#include <nex/util/FPSCounter.hpp>
#include <nex/config/Configuration.hpp>
#include <VideoConfig.hpp>
#include <nex/resource/FileSystem.hpp>
#include "Globals.hpp"
#include "gui/Controller.hpp"
#include <nex/common/Future.hpp>
#include <nex/renderer/RenderEngine.hpp>

namespace nex
{

	class SubSystemProvider;
	class SubSystemProviderGLFW;
	class Cursor;
	class Window;
	class GlobalIllumination;
	class ResourceLoader;
	class ProbeGenerator;
	class ProbeCluster;
	class ShadowMap;
	class FlameShader;
	class ParticleManager;
	class ParticleShader;
	class VisualizationSphere;
	class OceanGPU;
	class PSSR;

	namespace gui
	{
		class Picker;
		class SceneGUI;
		class ProbeClusterView;
	}

	class Euclid : public RenderEngine
	{
	public:

		Euclid(SubSystemProvider* provider);
		virtual ~Euclid();

		nex::LogLevel getLogLevel() const;

		void init();

		void initScene();

		bool isRunning() const;

		void run();

		void setConfigFileName(const char* fileName);

		void setRunning(bool isRunning);

	protected:
		void createScene(nex::RenderEngine::CommandQueue*);
		Window* createWindow();
		void initLights();
		void initPbr();
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
		std::unique_ptr<gui::EngineController> mControllerSM;
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
		std::unique_ptr<VisualizationSphere> mVisualizationSphere;
		std::vector<std::unique_ptr<MeshGroup>> mMeshes;
		bool mIsRunning;

		Configuration mConfig;
		VideoConfig mVideo;
		std::string mConfigFileName;
		std::string mSystemLogLevelStr;
		nex::LogLevel mSystemLogLevel;

		std::unique_ptr<nex::FileSystem> mShaderFileSystem;

		std::unique_ptr<CascadedShadow> mCascadedShadow;
		std::unique_ptr<ShadowMap> mGiShadowMap;
		std::unique_ptr<GlobalIllumination> mGlobalIllumination;
		std::unique_ptr<PbrTechnique> mPbrTechnique;
		std::unique_ptr<ProbeGenerator> mProbeGenerator;
		std::unique_ptr<nex::gui::ProbeClusterView> mProbeClusterView;
		std::unique_ptr<ShaderStorageBuffer> mBoneTrafoBuffer;
		std::unique_ptr<FlameShader> mFlameShader;
		std::unique_ptr<ParticleShader> mParticleShader;
		RenderCommandQueue mRenderCommandQueue;

		DirLight mSun;

		std::unique_ptr<Ocean> mOcean;
		std::unique_ptr<OceanVob> mOceanVob;
		std::unique_ptr<PSSR> mPSSR;
	};
}