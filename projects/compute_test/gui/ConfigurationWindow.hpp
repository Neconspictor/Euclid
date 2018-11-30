#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/Menu.hpp>

namespace App
{
	class ConfigurationWindow : public nex::engine::gui::Window
	{
	private:
		nex::engine::gui::MainMenuBar* m_mainMenuBar;
		std::string m_menuTitle;
		nex::engine::gui::TabBar* m_tabBar;

		inline static const char* GENERAL = "General";
		inline static const char* GRAPHICS_TECHNIQUES = "Graphics Techniques";
		inline static const char* CAMERA = "Camera";
		inline static const char* VIDEO = "Video";

	public:
		ConfigurationWindow(nex::engine::gui::MainMenuBar* mainMenuBar, nex::engine::gui::Menu* configurationMenu);

		void drawGUI() override;

		nex::engine::gui::Tab* getGeneralTab();

		nex::engine::gui::Tab* getGraphicsTechniquesTab();

		nex::engine::gui::Tab* getCameraTab();

		nex::engine::gui::Tab* getVideoTab();

	protected:
		bool hasVisibleChild() const;

		void drawSelf() override;
	};
}
