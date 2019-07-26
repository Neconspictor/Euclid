#include <gui/NodeEditor.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"
#include "nex/gui/Picker.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "nex/pbr/PbrProbe.hpp"
#include "nex/texture/TextureManager.hpp"
#include <nfd/nfd.h>
#include <nex/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <boxer/boxer.h>

namespace nex::gui
{
	NodeEditor::NodeEditor(nex::Window* window) : mPicker(nullptr),
		mBrdfView({}, ImVec2(256, 256)),
		mConvolutedView({}, ImVec2(256, 256)),
		mPrefilteredView({}, ImVec2(256, 256)),
		mWindow(window),
		mScene(nullptr),
		mLastPicked(nullptr)
		//mTransparentView({}, ImVec2(256, 256))
	{
	}

	NodeEditor::~NodeEditor() = default;

	void NodeEditor::setPicker(Picker* picker)
	{
		mPicker = picker;
	}

	void NodeEditor::setScene(nex::Scene * scene)
	{
		mScene = scene;
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

	void nex::gui::NodeEditor::drawSelf()
	{
		ImGui::PushID(mId.c_str());
		nex::gui::Separator(2.0f);

		Vob* vob = nullptr;
		bool doOneTimeChanges = false;

		if (mPicker) {
			vob = mPicker->getPicked();
			doOneTimeChanges = vob != mLastPicked;
			mLastPicked = vob;
		}

		ImGui::Text("Selected scene node:");
		if (!mPicker || !vob) {
			ImGui::Text("No scene node selected.");
			ImGui::PopID();
			return;
		}



		ImGui::SameLine();
		if (auto* probeVob = dynamic_cast<ProbeVob*>(vob))
		{
			auto* probe = probeVob->getProbe();

			ImGui::Text("pbr probe vob");

			if (doOneTimeChanges) {
				auto& irradiance = mConvolutedView.getTexture();
				irradiance.level = probe->getArrayIndex();

				auto& probePrefiltered = mPrefilteredView.getTexture();
				probePrefiltered.level = probe->getArrayIndex();
			}

			if (ImGui::TreeNode("Brdf Lookup map"))
			{
				auto* texture = probe->getBrdfLookupTexture();
				auto& probePrefiltered = mBrdfView.getTexture();
				probePrefiltered.texture = texture;
				probePrefiltered.flipY = ImageFactory::isYFlipped();
				probePrefiltered.sampler = nullptr;


				mBrdfView.updateTexture(true);
				mBrdfView.drawGUI();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Convoluted map"))
			{
				auto* texture = probe->getIrradianceMaps();
				auto& irradiance = mConvolutedView.getTexture();
				irradiance.texture = texture;
				irradiance.flipY = ImageFactory::isYFlipped();
				irradiance.sampler = nullptr;

				mConvolutedView.updateTexture(true);
				mConvolutedView.drawGUI();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Prefiltered map"))
			{
				auto* texture = probe->getPrefilteredMaps();
				auto& probePrefiltered = mPrefilteredView.getTexture();
				probePrefiltered.texture = texture;
				probePrefiltered.flipY = ImageFactory::isYFlipped();
				probePrefiltered.sampler = nullptr;

				mPrefilteredView.updateTexture(true);
				mPrefilteredView.drawGUI();

				ImGui::TreePop();
			}
		}
		else
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

		vob->mDebugName.reserve(256);
		if (ImGui::InputText("debug name", vob->mDebugName.data(), vob->mDebugName.capacity())) {
		}


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
