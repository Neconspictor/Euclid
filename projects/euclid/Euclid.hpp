#pragma once
#include <EuclidRenderer.hpp>
#include <nex/util/Timer.hpp>
#include <nex/util/FPSCounter.hpp>
#include <nex/config/Configuration.hpp>
#include <VideoConfig.hpp>
#include <nex/resource/FileSystem.hpp>
#include "Globals.hpp"
#include "gui/Controller.hpp"
#include <nex/common/Future.hpp>
#include <nex/renderer/RenderEngine.hpp>
#include <nex/platform/Input.hpp>
#include <interface/buffers.h>
#include <nex/renderer/RenderContext.hpp>
#include <nex/common/Cache.hpp>

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
	class PSSR;
	class UniformBuffer;
	class AtmosphericScattering;
	class AbstractMeshLoader;


	namespace gui
	{
		class Picker;
		class SceneGUI;
		class ProbeClusterView;
		class FontManager;
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

		void collectCommands();
		void createScene(nex::RenderEngine::CommandQueue*);
		Window* createWindow();

		void executeTasks();

		void initLights();
		void initPbr();
		void initRenderBackend();
		void readConfig();

		void createVoxels();

		void renderFrame(float frameTime);

		void setupCallbacks();
		void setupGUI();
		void setupCamera();

		void updateRenderContext(float frameTime);
		void updateShaderConstants();
		void updateVoxelTexture();
		void updateWindowTitle(float frameTime, float fps);
		nex::Texture* visualizeVoxels();

		std::unique_ptr<Vob> loadVob(const std::filesystem::path& p, 
			nex::RenderEngine::CommandQueue* commandQueue, 
			const AbstractMaterialLoader& materialLoader,
			const nex::FileSystem* fileSystem = nullptr);
		

	private:
		util::Globals mGlobals;
		nex::Logger mLogger;
		std::unique_ptr<EuclidRenderer> mRenderer;
		std::unique_ptr<gui::EngineController> mControllerSM;
		std::unique_ptr<PerspectiveCamera> mCamera;
		SubSystemProvider* mWindowSystem;
		Window* mWindow;
		std::unique_ptr<Cursor> mCursor;
		Input* mInput;
		std::string mBaseTitle;
		Timer mTimer;
		FPSCounter mCounter;
		Scene mScene;
		std::unique_ptr<VisualizationSphere> mVisualizationSphere;
		nex::Cache<nex::FlexibleCacheItem<VobBluePrint>> mVobBluePrintCache;
		bool mIsRunning;

		Configuration mConfig;
		VideoConfig mVideo;
		std::string mConfigFileName;
		std::string mSystemLogLevelStr;
		nex::LogLevel mSystemLogLevel;
		std::string mKeyMapLanguageStr;
		nex::KeyMapLanguage mKeyMapLanguage;

		std::unique_ptr<nex::FileSystem> mShaderFileSystem;

		std::unique_ptr<CascadedShadow> mCascadedShadow;
		std::unique_ptr<ShadowMap> mGiShadowMap;
		std::unique_ptr<GlobalIllumination> mGlobalIllumination;
		std::unique_ptr<PbrTechnique> mPbrTechnique;
		std::unique_ptr<ProbeGenerator> mProbeGenerator;
		std::unique_ptr<nex::gui::ProbeClusterView> mProbeClusterView;
		std::unique_ptr<FlameShader> mFlameShader;
		std::unique_ptr<ParticleShader> mParticleShader;
		RenderCommandQueue mRenderCommandQueue;
		float mRenderScale = 1.0f;

		DirLight mSun;
		glm::vec4 mCurrentSunDir;

		std::unique_ptr<PSSR> mPSSR;

		std::unique_ptr<nex::gui::Picker> mPicker;
		std::unique_ptr<nex::gui::FontManager> mFontManager;	

		std::unique_ptr<AtmosphericScattering> mAtmosphericScattering;

		RenderContext mContext;
	};
}