#pragma once
#include <gui/View.hpp>
#include "gui/Menu.hpp"

namespace App
{
	class ConfigurationWindow : public nex::engine::gui::View
	{
	private:
		std::string m_name;
		nex::engine::gui::MainMenuBar* m_mainMenuBar;
	public:
		ConfigurationWindow(std::string name, nex::engine::gui::MainMenuBar* mainMenuBar);
	protected:
		bool hasVisibleChild() const;

		void drawSelf() override;

		void drawSelfAfterChildren() override;
	};
}
