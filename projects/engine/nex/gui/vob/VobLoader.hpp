#pragma once

#include <nex/gui/MenuWindow.hpp>


namespace nex
{
	class PbrTechnique;
	class Scene;
	class Window;
	class MeshGroup;
}

namespace nex::gui
{
	class VobLoader : public nex::gui::MenuWindow
	{
	public:
		VobLoader(std::string title,
			nex::gui::MainMenuBar* menuBar,
			nex::gui::Menu* menu,
			nex::Scene* scene,
			std::vector<std::unique_ptr<nex::MeshGroup>>* meshes,
			nex::PbrTechnique* pbrTechnique,
			nex::Window* widow);
		virtual ~VobLoader();
		void setScene(nex::Scene* scene);
		void setMeshes(std::vector<std::unique_ptr<nex::MeshGroup>>* meshes);

	protected:

		void drawSelf() override;

		nex::Scene* mScene;
		nex::Window* mWindow;
		nex::PbrTechnique* mPbrTechnique;
		std::vector<std::unique_ptr<nex::MeshGroup>>* mMeshes;
	};
}