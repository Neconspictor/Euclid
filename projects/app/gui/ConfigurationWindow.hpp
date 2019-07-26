#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/Menu.hpp>

namespace nex::gui
{
	class ConfigurationWindow : public nex::gui::Window
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

		void drawGUI() override;

		nex::gui::Tab* getGeneralTab();

		nex::gui::Tab* getGraphicsTechniquesTab();

		nex::gui::Tab* getCameraTab();

		nex::gui::Tab* getVideoTab();

	protected:
		bool hasVisibleChild() const;

		void drawSelf() override;

		nex::gui::MainMenuBar* m_mainMenuBar;
		nex::gui::TabBar* m_tabBar;

		inline static const char* GENERAL = "General";
		inline static const char* GRAPHICS_TECHNIQUES = "Graphics Techniques";
		inline static const char* CAMERA = "Camera";
		inline static const char* VIDEO = "Video";
	};
}
