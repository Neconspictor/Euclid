#include <gui/SceneGUI.hpp>
#include <imgui/imgui.h>

namespace nex::engine::gui
{
	SceneGUI::SceneGUI() : m_optionMenu(nullptr)
	{
		std::unique_ptr<Menu> fileMenu = std::make_unique<Menu>("File");
		std::unique_ptr<MenuItem> exitMenuItem = std::make_unique<MenuItem>([](MenuItem* menuItem)
		{
			if (ImGui::MenuItem("Exit", "Esc"))
			{
				//handleExitEvent();
			}
		});


		std::unique_ptr<Menu> optionMenu = std::make_unique<Menu>("Options");
		
		m_fileMenu = fileMenu.get();
		m_optionMenu = optionMenu.get();

		m_fileMenu->addMenuItem(std::move(exitMenuItem));

		m_menuBar.addMenu(std::move(fileMenu));
		m_menuBar.addMenu(std::move(optionMenu));
	}

	Menu* SceneGUI::getFileMenu() const
	{
		return m_fileMenu;
	}

	Menu* SceneGUI::getOptionMenu() const
	{
		return m_optionMenu;
	}

	void SceneGUI::drawSelf()
	{
		m_menuBar.drawGUI();
	}
}