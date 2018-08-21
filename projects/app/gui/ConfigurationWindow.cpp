#include <gui/ConfigurationWindow.hpp>
#include <imgui/imgui.h>
#include "nex/gui/imgui_tabs.h"
#include "nex/gui/Util.hpp"

using namespace nex::engine::gui;

App::ConfigurationWindow::ConfigurationWindow(MainMenuBar* mainMenuBar, Menu* configurationMenu) :
Window("Graphics and Video Settings", false),  m_mainMenuBar(mainMenuBar), m_menuTitle("Graphics and Video Settings")
{
	m_imGuiFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize
	| ImGuiWindowFlags_NoCollapse;
	//ImGuiWindowFlags_ResizeFromAnySide | ImGuiWindowFlags_HorizontalScrollbar

	m_isVisible = false;

	MenuItemPtr menuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
	{
		std::string label = m_menuTitle + "###" + m_id;
		if (ImGui::Checkbox(label.c_str(), &m_isVisible)){}

	});

	configurationMenu->addMenuItem(std::move(menuItem));
}

void App::ConfigurationWindow::drawGUI()
{
		Window::drawGUI();
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

	//ImGui::BeginChild("child", ImVec2(-1, 100), false, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_ResizeFromAnySide);
	ImGui::BeginGroup();
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.15f, 0.15f, 1.0));
	ImGui::BeginTabBar("Settings#left_tabs_bar");

	// ericb 2017_07_21 : draw the tabs background BEFORE to fill it, to avoid a "colored overlay"
	//ImGui::DrawTabsBackground();
	
	if (ImGui::TabItem("General")) {

	}
	if (ImGui::TabItem("GUI###1")) {
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 2");
		ImGui::Text("Tab 20");
	}
	if (ImGui::TabItem("Tab Name###2")) {
		ImGui::Text("Tab 3");
	}
	if (ImGui::TabItem("Tab Name###3")) {
		ImGui::Text("Tab 4");
	}
	
	ImGui::EndTabBar();

	ImGui::Dummy(ImVec2(0, 20));
	nex::engine::gui::Separator(2.0f);

	//ImGui::PopStyleColor();
	ImGui::EndGroup();
	//ImGui::EndChild();

	//ImGui::Dummy(ImVec2(0, 100));

	//ImGui::Dummy(ImVec2(0, 100));

	ImGui::Text("After tab area");
}