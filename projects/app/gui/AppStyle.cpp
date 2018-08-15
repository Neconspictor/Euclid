#include <imgui/imgui.h>
#include <gui/AppStyle.hpp>

void App::AppStyle::apply()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.00f, 0.0f, 0.2f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.00f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.00f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.00f, 0.0f, 1.0f);
}

void App::ConfigurationStyle::pushStyleChangesSelf()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
}

void App::ConfigurationStyle::popStyleChangesSelf()
{
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
}

void App::ConfigurationStyle2::pushStyleChangesSelf()
{
	ImVec4(1.0f, 0.0f, 0.0f, 0.5f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
}

void App::ConfigurationStyle2::popStyleChangesSelf()
{
	ImGui::PopStyleVar(1);
	ImGui::PopStyleColor(2);
}