#include <gui/ConfigurationWindow.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"

using namespace nex::gui;

nex::gui::ConfigurationWindow::ConfigurationWindow(std::string title, MainMenuBar* mainMenuBar, Menu* menu, int flags) :
	MenuWindow(std::move(title), mainMenuBar, menu, flags),
	mTabBar(nullptr)
{
	auto tabBar = std::make_unique<TabBar>("###tabBar");

	mTabBar = tabBar.get();

	mChilds.emplace_back(std::move(tabBar));

	mTabBar->newTab(GRAPHICS_TECHNIQUES);
	mTabBar->newTab(GENERAL);
	mTabBar->newTab(VIDEO);
	mTabBar->newTab(CAMERA);
}

Tab* nex::gui::ConfigurationWindow::getGeneralTab()
{
	return mTabBar->getTab(GENERAL);
}

Tab* nex::gui::ConfigurationWindow::getGraphicsTechniquesTab()
{
	return mTabBar->getTab(GRAPHICS_TECHNIQUES);
}

Tab* nex::gui::ConfigurationWindow::getCameraTab()
{
	return mTabBar->getTab(CAMERA);
}

Tab* nex::gui::ConfigurationWindow::getVideoTab()
{
	return mTabBar->getTab(VIDEO);
}