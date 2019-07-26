#include <nex/gui/MenuWindow.hpp>
#include <imgui/imgui.h>
#include <nex/gui/imgui_tabs.h>
#include <nex/gui/Util.hpp>

nex::gui::MenuWindow::MenuWindow(std::string title, MainMenuBar* mainMenuBar, Menu* menu, int flags) :
	Window(std::move(title), true, flags),
	mMainMenuBar(mainMenuBar)
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

	menu->addMenuItem(std::move(menuItem));
}

void nex::gui::MenuWindow::drawGUI()
{
		Window::drawGUI();
}

bool nex::gui::MenuWindow::hasVisibleChild() const
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

void nex::gui::MenuWindow::drawSelf()
{
	const float mainbarHeight = mMainMenuBar->getSize().y;
	const ImVec2 mainbarPos = mMainMenuBar->getPosition();

	ImGui::SetNextWindowPos(ImVec2(mainbarPos.x, mainbarPos.y + mainbarHeight));
	Window::drawSelf();
}