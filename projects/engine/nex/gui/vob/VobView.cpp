#include <nex/gui/vob/VobView.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nfd/nfd.h>
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <boxer/boxer.h>
#include <nex/scene/Vob.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/gui/Picker.hpp>

namespace nex::gui
{
	void VobView::draw(Vob* vob, Scene* scene, Picker* picker, bool doOneTimeChanges)
	{
		if (vob->isDeletable() && ImGui::Button("Delete Vob")) {
			scene->acquireLock();
			if (scene->deleteVobUnsafe(vob)) {
				picker->deselect(*scene);
				return;
			}
		}

		glm::vec3 position = vob->getPosition();
		nex::gui::Vector3D(&position, "Position");
		vob->setPosition(position);

		glm::quat rotation = vob->getRotation();
		nex::gui::Quat(&rotation, "Orientation (Quaternion) - Radians");
		rotation = normalize(rotation);
		glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));

		if (euler.x == -180.0f) euler.x = 180.0f;
		if (euler.z == -180.0f) euler.z = 180.0f;

		nex::gui::Vector3D(&euler, "Orientation (Euler X-Y-Z) - Degrees");

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
	}
}