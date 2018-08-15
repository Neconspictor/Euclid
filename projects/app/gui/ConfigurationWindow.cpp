#include <gui/ConfigurationWindow.hpp>
#include <imgui/imgui.h>

App::ConfigurationWindow::ConfigurationWindow(std::string name, nex::engine::gui::MainMenuBar* mainMenuBar) :
Window(std::move(name), false),  m_mainMenuBar(mainMenuBar)
{
	m_imGuiFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize; //ImGuiWindowFlags_NoTitleBar
	//ImGuiWindowFlags_ResizeFromAnySide | ImGuiWindowFlags_HorizontalScrollbar
}

void App::ConfigurationWindow::drawGUI()
{
	if (hasVisibleChild())
	{
		Window::drawGUI();
	}
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
	const float mainbarHeight = m_mainMenuBar->getSize().y;
	const ImVec2 mainbarPos = m_mainMenuBar->getPosition();

	ImGui::SetNextWindowPos(ImVec2(mainbarPos.x, mainbarPos.y + mainbarHeight));
	ImGui::Begin(m_name.c_str(), &m_isVisible, m_imGuiFlags);
}