#include <nex/gui/vob/RiggedVobView.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/anim/Rig.hpp>
#include <nex/anim/BoneAnimation.hpp>

bool nex::gui::RiggedVobView::draw(Vob* vob, Scene* scene, Picker* picker, Camera* camera, bool doOneTimeChanges)
{
	if (!VobView::draw(vob, scene, picker, camera, doOneTimeChanges)) {
		return false;
	}

	auto* riggedVob = dynamic_cast<RiggedVob*>(vob);

	if (!riggedVob) {
		return true;
	}

	auto* rig = riggedVob->getRig();

	ImGui::Separator();

	ImGui::Text("Rig ID: %s", rig->getID().c_str());

	static std::string editableText = "editable text";

	if (ImGui::InputText("Input field", &editableText, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
		LOG(Logger("RiggedVobView"), Info) << "input field returned true";
	}
	

	rig->getBones();

	return true;
}
