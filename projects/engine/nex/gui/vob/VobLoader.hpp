#pragma once

#include <nex/gui/MenuWindow.hpp>
#include <glm/glm.hpp>
#include <nex/common/Future.hpp>
#include <nex/common/Cache.hpp>
#include <nex/renderer/RenderEngine.hpp>
#include <nex/scene/Vob.hpp>


namespace nex
{
	class PbrTechnique;
	class Scene;
	class Window;
	class MeshGroup;
	class Camera;
	class Resource;
	class AbstractMaterialLoader;
	class AbstractMeshLoader;
	class FileSystem;
}

namespace nex::gui
{
	class VobLoader : public nex::gui::MenuWindow
	{
	public:

		using VobBluePrints = nex::Cache<nex::FlexibleCacheItem<VobBluePrint>>;

		VobLoader(std::string title,
			nex::gui::MainMenuBar* menuBar,
			nex::gui::Menu* menu,
			nex::Scene* scene,
			VobBluePrints* vobBluePrints,
			nex::PbrTechnique* pbrTechnique,
			nex::Window* widow,
			Camera* camera);
		virtual ~VobLoader();
		void setScene(nex::Scene* scene);

	protected:

		void drawSelf() override;

		void selectAndLoadVob();

		std::unique_ptr<Vob> loadVob(const std::filesystem::path& p,
			nex::RenderEngine::CommandQueue* commandQueue,
			const AbstractMaterialLoader& materialLoader,
			const nex::FileSystem* fileSystem = nullptr);

		nex::Scene* mScene;
		nex::Window* mWindow;
		nex::PbrTechnique* mPbrTechnique;
		VobBluePrints* mBluePrints;
		Camera* mCamera;
		bool mUseRescale = false;
		float mDefaultScale = 1.0f;
		nex::Future<Vob*> mFuture;
	};
}