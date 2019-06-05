#include <gui/SceneGUI.hpp>
#include <imgui/imgui.h>
#include <gui/Controller.hpp>
#include "nex/gui/Util.hpp"
#include "nex/gui/Picker.hpp"
#include <glm/gtc/matrix_transform.hpp>

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



	// Calculates rotation matrix given euler angles.
	glm::mat3 eulerAnglesToRotationMatrix(const glm::vec3& theta)
	{
		// Calculate rotation about x axis
		glm::mat3 R_x = {
			1, 0, 0,
			0, cos(theta[0]), -sin(theta[0]),
			0, sin(theta[0]), cos(theta[0])
		};

		// Calculate rotation about y axis
		glm::mat3 R_y = {
			cos(theta[1]), 0, sin(theta[1]),
			0, 1, 0,
			-sin(theta[1]), 0, cos(theta[1])
		};

		// Calculate rotation about z axis
		glm::mat3 R_z = {
			cos(theta[2]), -sin(theta[2]), 0,
			sin(theta[2]), cos(theta[2]), 0,
			0, 0, 1 };


		// Combined rotation matrix
		return transpose(R_z) * transpose(R_y) * transpose(R_x);
	}

	// Calculates rotation matrix to euler angles
// The result is the same as MATLAB except the order
// of the euler angles ( x and z are swapped ).
	glm::vec3 rotationMatrixToEulerAngles(const glm::mat4 &R)
	{
		float sy = sqrt(R[0][0] * R[0][0] + R[0][1] * R[0][1]);

		bool singular = sy < 1e-6; // If

		float x, y, z;
		if (!singular)
		{
			x = atan2(R[1][2], R[2][2]);
			y = atan2(-R[0][2], sy);
			z = atan2(R[0][1], R[0][0]);
		}
		else
		{
			x = atan2(-R[2][1], R[1][1]);
			y = atan2(-R[0][2], sy);
			z = 0;
		}
		return { x, y, z };
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

		glm::vec3 euler(0.0f);//glm::degrees(rotationMatrixToEulerAngles(trafo));

		glm::vec3 eulerCopy = euler;
		nex::gui::Vector3D(&eulerCopy, "Rotation (Euler (X-Y-Z))");


		glm::mat4 rotation = glm::mat3(trafo);
		rotation[3].w = 1.0f;

		if (eulerCopy.x != euler.x)
		{
			rotation = glm::rotate(rotation, glm::radians(eulerCopy.x - euler.x), glm::vec3(1, 0, 0));
		}

		if (eulerCopy.y != euler.y)
		{
			rotation = glm::rotate(rotation, glm::radians(eulerCopy.y - euler.y), glm::vec3(0, -1, 0));
		}

		if (eulerCopy.z != euler.z)
		{
			rotation = glm::rotate(rotation, glm::radians(eulerCopy.z - euler.z), glm::vec3(0, 0, 1));
		}
			



		rotation[3].x = position.x;
		rotation[3].y = position.y;
		rotation[3].z = position.z;
		rotation[3].w = 1.0f;

		node->setLocalTrafo(rotation);
		node->updateWorldTrafoHierarchy();
		mPicker->updateBoundingBoxTrafo();

		ImGui::PopID();
	}
}
