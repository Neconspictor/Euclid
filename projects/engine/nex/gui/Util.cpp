#include <nex/gui/Util.hpp>
#include <imgui/imgui_internal.h>


void nex::gui::Separator(float thickness, bool vertical)
{
	ImGui::Separator();
}

bool nex::gui::Vector3D(glm::vec3* vec, const char* label, float speed)
{
	//std::stringstream ss;
	//ss << label << ":";
	//ImGui::TextUnformatted(ss.str().c_str());
	return ImGui::DragFloat3(label, (float*)vec);
}

void nex::gui::EulerRot(glm::vec3* vec, const char* label, float speed)
{
	std::stringstream ss;
	ss << label << ":";
	ImGui::TextUnformatted(ss.str().c_str());

	ImGui::PushID(label);
	ImGui::TextUnformatted("X: ");
	ImGui::SameLine();
	ImGui::DragFloat("###X", &vec->x, speed);

	ImGui::TextUnformatted("Y: ");
	ImGui::SameLine();
	ImGui::DragFloat("###Y", &vec->y, speed);
		
		
	ImGui::TextUnformatted("Z: ");
	ImGui::SameLine();
	ImGui::DragFloat("###Z", &vec->z, speed);
	ImGui::PopID();
}

void nex::gui::Quat(glm::quat* quat, const char* label)
{
	std::stringstream ss;
	ss << label << ":";
	ImGui::TextUnformatted(ss.str().c_str());

	ImGui::PushID(label);
	ImGui::TextUnformatted("X: ");
	ImGui::SameLine();
	ImGui::DragFloat("###X", &quat->x, 0.1f);
	ImGui::TextUnformatted("Y: ");
	ImGui::SameLine();
	ImGui::DragFloat("###Y", &quat->y, 0.1f);
	ImGui::TextUnformatted("Z: ");
	ImGui::SameLine();
	ImGui::DragFloat("###Z", &quat->z, 0.1f);
	ImGui::TextUnformatted("W: ");
	ImGui::SameLine();
	ImGui::DragFloat("###W", &quat->w, 0.1f);
	ImGui::PopID();
}

nex::gui::ID::ID(int id)
{
	ImGui::PushID(id);
}
nex::gui::ID::~ID()
{
	ImGui::PopID();
}

ImVec2 nex::gui::GetWindowContentPadding()
{
	const auto& style = GImGui->Style;
	return style.WindowPadding + style.FramePadding;
}

ImVec2 nex::gui::GetWindowContentEffectiveSize()
{
	const auto& style = GImGui->Style;
	return style.WindowPadding + style.FramePadding;
}
