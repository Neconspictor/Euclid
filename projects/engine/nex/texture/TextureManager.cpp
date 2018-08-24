#include <nex/texture/TextureManager.hpp>
#include <imgui/imgui.h>
#include <nex/gui/Util.hpp>
#include <nex/util/ExceptionHandling.hpp>

TextureManager_Configuration::TextureManager_Configuration(TextureManager* textureManager) : m_textureManager(textureManager)
{
}

void TextureManager_Configuration::drawSelf()
{

	float anisotropyMax = m_textureManager->getMaxAnisotropicFiltering();
	float anisotropy = m_textureManager->getAnisotropicFiltering();

	//float anisotropyBackup = anisotropy;

	// render configuration properties
	ImGui::PushID(m_id.c_str());
	if (ImGui::InputFloat("Anisotropic Filtering (read-only)", &anisotropy, 1.0f, 1.0f, "%.3f", ImGuiInputTextFlags_ReadOnly))
	{
	}
	ImGui::PopID();

	//throw_with_trace(std::runtime_error("Hello exception!"));
}