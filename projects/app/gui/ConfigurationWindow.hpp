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
	public:
		ConfigurationWindow(nex::engine::gui::MainMenuBar* mainMenuBar, nex::engine::gui::Menu* configurationMenu);

		void drawGUI() override;

	protected:
		bool hasVisibleChild() const;

		void drawSelf() override;
	};
}
