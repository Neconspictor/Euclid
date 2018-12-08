#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/Menu.hpp>

namespace nex::gui
{
	class ConfigurationWindow : public nex::gui::Window
	{
	private:
		nex::gui::MainMenuBar* m_mainMenuBar;
		std::string m_menuTitle;
		nex::gui::TabBar* m_tabBar;

		inline static const char* GENERAL = "General";
		inline static const char* GRAPHICS_TECHNIQUES = "Graphics Techniques";
		inline static const char* CAMERA = "Camera";
		inline static const char* VIDEO = "Video";

	public:
		ConfigurationWindow(nex::gui::MainMenuBar* mainMenuBar, nex::gui::Menu* configurationMenu);

		void drawGUI() override;

		nex::gui::Tab* getGeneralTab();

		nex::gui::Tab* getGraphicsTechniquesTab();

		nex::gui::Tab* getCameraTab();

		nex::gui::Tab* getVideoTab();

	protected:
		bool hasVisibleChild() const;

		void drawSelf() override;
	};
}
