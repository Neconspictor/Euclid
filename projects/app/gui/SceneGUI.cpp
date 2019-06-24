#include <gui/SceneGUI.hpp>
#include <imgui/imgui.h>
#include <gui/Controller.hpp>
#include "nex/gui/Util.hpp"
#include "nex/gui/Picker.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <nex/gui/Gizmo.hpp>
#include <nex/Input.hpp>
#include "NeXEngine.hpp"
#include "nex/pbr/PbrProbe.hpp"
#include "nex/texture/TextureManager.hpp"

namespace nex::gui
{
	SceneGUI::SceneGUI(const std::function<void()> exitCallback) : m_optionMenu(nullptr), mExitCallback(std::move(exitCallback))
	{
		std::unique_ptr<Menu> fileMenu = std::make_unique<Menu>("File");
		std::unique_ptr<MenuItem> exitMenuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
		{
			if (ImGui::MenuItem("Exit", "Esc"))
			{
				mExitCallback();
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

	SceneNodeProperty::SceneNodeProperty() : mPicker(nullptr), 
	mBrdfView({}, ImVec2(256, 256)),
	mConvolutedView({}, ImVec2(256, 256)),
	mPrefilteredView({}, ImVec2(256, 256))
	//mTransparentView({}, ImVec2(256, 256))
	{
	}

	SceneNodeProperty::~SceneNodeProperty() = default;

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

	void nex::gui::SceneNodeProperty::drawSelf()
	{
		ImGui::PushID(m_id.c_str());
		nex::gui::Separator(2.0f);
		ImGui::Text("Selected scene node:");
		if (!mPicker || !mPicker->getPicked()) {
			ImGui::Text("No scene node selected.");
			ImGui::PopID();
			return;
		} 

		auto* vob = mPicker->getPicked();

		ImGui::SameLine();
		if (auto* probeVob = dynamic_cast<ProbeVob*>(vob))
		{
			auto* probe = probeVob->getProbe();
			ImGui::Text("pbr probe vob");

			if (ImGui::TreeNode("Brdf Lookup map"))
			{
				auto* texture = probe->getBrdfLookupTexture();
				auto& probePrefiltered = mBrdfView.getTexture();
				probePrefiltered.texture = texture;
				probePrefiltered.sampler = nullptr;

				mBrdfView.updateTexture(true);
				mBrdfView.drawGUI();
				
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Convoluted map"))
			{
				auto* texture = probe->getConvolutedEnvironmentMap();
				auto& probePrefiltered = mConvolutedView.getTexture();
				probePrefiltered.texture = texture;
				probePrefiltered.sampler = nullptr;

				mConvolutedView.updateTexture(true);
				mConvolutedView.drawGUI();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Prefiltered map"))
			{
				auto* texture = probe->getPrefilteredEnvironmentMap();
				auto& probePrefiltered = mPrefilteredView.getTexture();
				probePrefiltered.texture = texture;
				probePrefiltered.sampler = nullptr;

				mPrefilteredView.updateTexture(true);
				mPrefilteredView.drawGUI();

				ImGui::TreePop();
			}

			/*if (ImGui::TreeNode("transparent texture"))
			{
				auto* texture = TextureManager::get()->getImage("transparent/blending_transparent_window.png");
				auto& desc = mTransparentView.getTexture();
				desc.texture = texture;
				desc.sampler = nullptr;

				mTransparentView.updateTexture(true);
				mTransparentView.drawGUI();

				ImGui::TreePop();
			}*/

		} else
		{
			ImGui::Text("normal vob");
		}

		glm::vec3 position = vob->getPosition();
		nex::gui::Vector3D(&position, "Position");
		vob->setPosition(position);

		//glm::degrees(rotationMatrixToEulerAngles(trafo));
		glm::quat rotation = vob->getRotation();
		nex::gui::Quat(&rotation, "Orientation (Quaternion) - Radians");
		rotation = normalize(rotation);
		glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));

		if (euler.x == -180.0f) euler.x = 180.0f;
		if (euler.z == -180.0f) euler.z = 180.0f;

		nex::gui::Vector3D(&euler, "Orientation (Euler X-Y-Z) - Degrees");

		//eulerCopy.x = fmod((eulerCopy.x + 180.0f),360.0f) - 180.0f;
		//eulerCopy.z = fmod((eulerCopy.z + 180.0f), 360.0f) - 180.0f;
		//eulerCopy.y = fmod((eulerCopy.y + 90.0f), 180.0f) - 90.0f;

		euler.y = std::clamp(euler.y, -89.0f, 89.0f);

		vob->setOrientation(radians(euler));


		euler = glm::vec3(0.0f);
		nex::gui::Vector3D(&euler, "Rotate (Euler X-Y-Z) - Local - Degrees");
		vob->rotateLocal(radians(euler));

		euler = glm::vec3(0.0f);
		nex::gui::Vector3D(&euler, "Rotate (Euler X-Y-Z) - Global - Degrees");
		vob->rotateGlobal(radians(euler));


		glm::vec3 scale = vob->getScale();
		nex::gui::Vector3D(&scale, "Scale", 0.1f);
		scale = maxVec(scale, glm::vec3(0.0f));
		vob->setScale(scale);

		vob->updateTrafo();
		mPicker->updateBoundingBoxTrafo();

		ImGui::PopID();
	}
}
