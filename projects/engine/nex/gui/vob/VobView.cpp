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
#include <nex/camera/Camera.hpp>
#include <list>

namespace nex::gui
{
	bool VobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
	{
		ImGui::Text("Type: "); ImGui::SameLine();
		ImGui::Text(vob->getTypeName().c_str());

		if (ImGui::Button("Jump to vob")) {
			camera->setPosition(glm::vec3(vob->getWorldTrafo()[3]), true);
		}

		if (vob->isDeletable() && ImGui::Button("Delete Vob")) {
			scene->acquireLock();
			
			if (scene->deleteVobUnsafe(vob)) {
				picker->deselect(*scene);
				return false;
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

		vob->getName().reserve(256);
		if (ImGui::InputText("Name", vob->getName().data(), vob->getName().capacity())) {
		
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

		return true;
	}
}