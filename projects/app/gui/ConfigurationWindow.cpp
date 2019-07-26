#include <gui/ConfigurationWindow.hpp>
#include <imgui/imgui.h>
#include "nex/gui/imgui_tabs.h"
#include "nex/gui/Util.hpp"

using namespace nex::gui;

nex::gui::ConfigurationWindow::ConfigurationWindow(std::string title, MainMenuBar* mainMenuBar, Menu* configurationMenu, int flags) :
	Window(std::move(title), true, flags),
	m_mainMenuBar(mainMenuBar),
	m_tabBar(nullptr)
{
	mIsVisible = false;
	mUseCloseCross = true;

	MenuItemPtr menuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
	{
		std::string label = mName + "###" + mId;
		if (ImGui::Checkbox(label.c_str(), &mIsVisible))
		{
		}
	});

	configurationMenu->addMenuItem(std::move(menuItem));

	auto tabBar = std::make_unique<TabBar>("###tabBar");

	m_tabBar = tabBar.get();

	mChilds.emplace_back(std::move(tabBar));

	m_tabBar->newTab(GRAPHICS_TECHNIQUES);
	m_tabBar->newTab(GENERAL);
	m_tabBar->newTab(VIDEO);
	m_tabBar->newTab(CAMERA);
}

void nex::gui::ConfigurationWindow::drawGUI()
{
		Window::drawGUI();
}

Tab* nex::gui::ConfigurationWindow::getGeneralTab()
{
	return m_tabBar->getTab(GENERAL);
}

Tab* nex::gui::ConfigurationWindow::getGraphicsTechniquesTab()
{
	return m_tabBar->getTab(GRAPHICS_TECHNIQUES);
}

Tab* nex::gui::ConfigurationWindow::getCameraTab()
{
	return m_tabBar->getTab(CAMERA);
}

Tab* nex::gui::ConfigurationWindow::getVideoTab()
{
	return m_tabBar->getTab(VIDEO);
}

bool nex::gui::ConfigurationWindow::hasVisibleChild() const
{
	for (auto &child : mChilds)
	{
		if (child->isVisible())
			return true;
	}

	for (auto &child : mReferencedChilds)
	{
		if (child->isVisible())
			return true;
	}

	return false;
}

void nex::gui::ConfigurationWindow::drawSelf()
{
	const float mainbarHeight = m_mainMenuBar->getSize().y;
	const ImVec2 mainbarPos = m_mainMenuBar->getPosition();

	ImGui::SetNextWindowPos(ImVec2(mainbarPos.x, mainbarPos.y + mainbarHeight));
	Window::drawSelf();
}