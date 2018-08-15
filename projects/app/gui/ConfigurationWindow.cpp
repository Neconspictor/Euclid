#include <gui/ConfigurationWindow.hpp>
#include <imgui/imgui.h>

App::ConfigurationWindow::ConfigurationWindow(std::string name, nex::engine::gui::MainMenuBar* mainMenuBar) : 
m_name(std::move(name)), m_mainMenuBar(mainMenuBar)
{
}

bool App::ConfigurationWindow::hasVisibleChild() const
{
	for (auto &child : m_childs)
	{
		if (child->isVisible())
			return true;
	}

	return false;
}

void App::ConfigurationWindow::drawSelf()
{
	// Only draw this window if it at least has one visible child 
	if (!hasVisibleChild()) return;


	float mainbarHeight = m_mainMenuBar->getSize().y;
	ImVec2 mainbarPos = m_mainMenuBar->getPosition();

	ImGui::SetNextWindowPos(ImVec2(mainbarPos.x, mainbarPos.y + mainbarHeight));
	ImGui::Begin(m_name.c_str(), nullptr, 
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize); //ImGuiWindowFlags_ResizeFromAnySide | ImGuiWindowFlags_HorizontalScrollbar
}

void App::ConfigurationWindow::drawSelfAfterChildren()
{
	// Only draw this window if it at least has one visible child 
	if (!hasVisibleChild()) return;
	ImGui::End();
}