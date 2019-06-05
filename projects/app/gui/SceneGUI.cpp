#include <gui/SceneGUI.hpp>
#include <imgui/imgui.h>
#include <gui/Controller.hpp>
#include "nex/gui/Util.hpp"
#include "nex/gui/Picker.hpp"

namespace nex::gui
{
	SceneGUI::SceneGUI(ControllerStateMachine* controllerSM) : m_optionMenu(nullptr), m_controllerSM(controllerSM)
	{
		std::unique_ptr<Menu> fileMenu = std::make_unique<Menu>("File");
		std::unique_ptr<MenuItem> exitMenuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
		{
			if (ImGui::MenuItem("Exit", "Esc"))
			{
				//handleExitEvent();
				nex::gui::BaseController* controller = dynamic_cast<nex::gui::BaseController*>(m_controllerSM->getCurrentController());

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

	SceneNodeProperty::SceneNodeProperty() :  mPicker(nullptr)
	{
	}

	void SceneNodeProperty::setPicker(Picker* picker)
	{
		mPicker = picker;
	}

	void SceneNodeProperty::drawSelf()
	{
		ImGui::PushID(m_id.c_str());
		nex::gui::Separator(2.0f);
		ImGui::Text("Selected scene node:");
		if (!mPicker || !mPicker->getPicked()) {
			ImGui::Text("No scene node selected.");
			ImGui::PopID();
			return;
		}

		auto* node = mPicker->getPicked();

		auto trafo = node->getLocalTrafo();
		glm::vec3 position = trafo[3];
		nex::gui::Vector3D(&position, "Position");

		trafo[3].x = position.x;
		trafo[3].y = position.y;
		trafo[3].z = position.z;

		node->setLocalTrafo(trafo);
		node->updateWorldTrafoHierarchy();
		mPicker->updateBoundingBoxTrafo();

		ImGui::PopID();
	}
}
