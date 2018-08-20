#include <gui/SceneGUI.hpp>
#include <imgui/imgui.h>
#include <iostream>
#include <gui/Controller.hpp>

namespace nex::engine::gui
{
	SceneGUI::SceneGUI(ControllerStateMachine* controllerSM) : m_optionMenu(nullptr), m_controllerSM(controllerSM)
	{
		std::unique_ptr<Menu> fileMenu = std::make_unique<Menu>("File");
		std::unique_ptr<MenuItem> exitMenuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
		{
			if (ImGui::MenuItem("Exit", "Esc"))
			{
				//handleExitEvent();
				App::BaseController* controller = dynamic_cast<App::BaseController*>(m_controllerSM->getCurrentController());

				if (controller != nullptr)
					controller->handleExitEvent();
			}
		});

		fileMenu->addMenuItem(std::move(exitMenuItem));


		std::unique_ptr<Menu> optionMenu = std::make_unique<Menu>("Options");
		
		m_fileMenu = fileMenu.get();
		m_optionMenu = optionMenu.get();

		m_menuBar.addMenu(std::move(fileMenu));
		m_menuBar.addMenu(std::move(optionMenu));
	}

	MainMenuBar* SceneGUI::getMainMenuBar()
	{
		return &m_menuBar;
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
