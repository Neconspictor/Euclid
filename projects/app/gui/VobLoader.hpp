#pragma once

#include <nex/gui/MenuWindow.hpp>


namespace nex
{
	class Scene;
	class Window;
	class Vob;
}

namespace nex::gui
{
	class VobLoader : public nex::gui::MenuWindow
	{
	public:
		VobLoader(std::string title, nex::gui::MainMenuBar* menuBar, nex::gui::Menu* menu, nex::Scene* scene, nex::Window* widow);
		virtual ~VobLoader();
		void setScene(nex::Scene* scene);

	protected:

		void drawSelf() override;

		nex::Scene* mScene;
		nex::Window* mWindow;
	};
}