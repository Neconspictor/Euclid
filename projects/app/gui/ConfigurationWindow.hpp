#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/MenuWindow.hpp>

namespace nex::gui
{
	class ConfigurationWindow : public nex::gui::MenuWindow
	{
	public:

		static constexpr int DEFAULT_WINDOW_FLAGS = ImGuiWindowFlags_NoMove 
			| ImGuiWindowFlags_AlwaysAutoResize 
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoCollapse;

		ConfigurationWindow(std::string title, 
			MainMenuBar* menuBar, 
			Menu* menu,
			int flags = DEFAULT_WINDOW_FLAGS);

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