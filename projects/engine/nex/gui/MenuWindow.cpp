#include <nex/gui/MenuWindow.hpp>
#include <imgui/imgui.h>
#include <nex/gui/ImGUI_Extension.hpp>

nex::gui::MenuWindow::MenuWindow(std::string title, MainMenuBar* mainMenuBar, Menu* menu, int flags) :
	Window(std::move(title), true, flags),
	mMainMenuBar(mainMenuBar)
{
	setVisible(false);
	mUseCloseCross = true;

	MenuItemPtr menuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
	{
		std::string label = mName + "###" + mId;

		if (!mIsVisible) mSetDefaultPosition = true;


		if (ImGui::MenuItem(label.c_str())) {
			setVisible(true);
			for (auto& child : mChilds) {
				child->setVisible(true);
			}
		}
	});

	menu->addMenuItem(std::move(menuItem));
}

nex::gui::MenuWindow::MenuWindow(std::string title, MainMenuBar* menuBar, Menu* menu, std::function<void()> drawFunc, int flags) : 
	MenuWindow(std::move(title), menuBar, menu, flags)
{
	mDrawFunc = std::move(drawFunc);
}

void nex::gui::MenuWindow::drawGUI()
{
	if (!mIsVisible) return;

	const float mainbarHeight = mMainMenuBar->getSize().y;
	const ImVec2 mainbarPos = mMainMenuBar->getPosition();

	if (mSetDefaultPosition) {
		ImGui::SetNextWindowPos(ImVec2(mainbarPos.x, mainbarPos.y + mainbarHeight));
		mSetDefaultPosition = false;
	}

	Window::drawGUI();
}

void nex::gui::MenuWindow::drawSelf()
{
	if (mDrawFunc.has_value()) {
		mDrawFunc.value()();
	}
}

bool nex::gui::MenuWindow::hasVisibleChild() const
{
	for (auto &child : mChilds)
	{
		if (child->isVisible())
			return true;
	}

	return false;
}

nex::gui::MenuDrawable::MenuDrawable(const std::string& title, MainMenuBar* mainMenuBar, Menu* menu, bool useDefaultStartPos) : Drawable(),
mName(title), mMainMenuBar(mainMenuBar), mUseDefaultStartPosition(useDefaultStartPos)
{
	setVisible(false);

	MenuItemPtr menuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
		{
			std::string label = mName + "###" + mId;

			if (!mIsVisible) mSetDefaultPosition = true;


			if (ImGui::MenuItem(label.c_str())) {
				setVisible(true);
				for (auto& child : mChilds) {
					child->setVisible(true);
				}
			}
		});

	menu->addMenuItem(std::move(menuItem));
}

nex::gui::MenuDrawable::MenuDrawable(const std::string& title, MainMenuBar* mainMenuBar, Menu* menu, bool useDefaultStartPos, Drawer drawFunc) :
	MenuDrawable(title, mainMenuBar, menu, useDefaultStartPos)
{
	mDrawFunc = std::move(drawFunc);
}

void nex::gui::MenuDrawable::drawSelf()
{
	const float mainbarHeight = mMainMenuBar->getSize().y;
	const ImVec2 mainbarPos = mMainMenuBar->getPosition();

	if (mDrawFunc.has_value()) {

		bool visible = isVisible();

		if (mUseDefaultStartPosition && mInit) {
			ImGui::SetNextWindowPos(ImVec2(mainbarPos.x, mainbarPos.y + mainbarHeight));
			mInit = false;
		}

		mDrawFunc.value()(mainbarPos, mainbarHeight, visible);

		if (visible != mIsVisible) setVisible(visible);
	}
}