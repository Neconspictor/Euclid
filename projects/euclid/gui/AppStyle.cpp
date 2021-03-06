#include <imgui/imgui.h>
#include <gui/AppStyle.hpp>

void nex::gui::AppStyle::apply()
{
	ImGuiStyle& style = ImGui::GetStyle();
	return;
	
	//style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.00f, 0.0f, 0.2f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.00f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.00f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.00f, 0.0f, 1.0f);
}

void nex::gui::ConfigurationStyle::pushStyleChangesSelf()
{
	return;
	float const alpha = 1.0f;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, alpha));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, alpha));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.0f, 0.0f, alpha));
	ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.0f, 0.0f, 0.0f, alpha));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
}

void nex::gui::ConfigurationStyle::popStyleChangesSelf()
{
	return;
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(5);
}

void nex::gui::ConfigurationStyle2::pushStyleChangesSelf()
{
	return;
	ImVec4(1.0f, 0.0f, 0.0f, 0.5f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
}

void nex::gui::ConfigurationStyle2::popStyleChangesSelf()
{
	return;
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
}