#pragma once
#include <gui/Drawable.hpp>
#include "gui/Menu.hpp"

namespace App
{
	class ConfigurationWindow : public nex::engine::gui::Window
	{
	private:
		nex::engine::gui::MainMenuBar* m_mainMenuBar;
	public:
		ConfigurationWindow(std::string name, nex::engine::gui::MainMenuBar* mainMenuBar);

		void drawGUI() override;

	protected:
		bool hasVisibleChild() const;

		void drawSelf() override;
	};
}
