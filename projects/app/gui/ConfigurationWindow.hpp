#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/MenuWindow.hpp>

namespace nex::gui
{
	class ConfigurationWindow : public nex::gui::MenuWindow
	{
	public:

		ConfigurationWindow(std::string title, 
			MainMenuBar* menuBar, 
			Menu* menu,
			int flags = MenuWindow::DEFAULT_FLAGS);

		nex::gui::Tab* getGeneralTab();

		nex::gui::Tab* getGraphicsTechniquesTab();

		nex::gui::Tab* getCameraTab();

		nex::gui::Tab* getVideoTab();

	protected:

		nex::gui::TabBar* mTabBar;

		static constexpr const char* GENERAL = "General";
		static constexpr const char* GRAPHICS_TECHNIQUES = "Graphics Techniques";
		static constexpr const char* CAMERA = "Camera";
		static constexpr const char* VIDEO = "Video";
	};
}