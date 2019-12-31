#include <gui/SceneGUI.hpp>
#include <gui/Controller.hpp>
#include "nex/gui/Util.hpp"
#include <nex/platform/Input.hpp>
#include <nex/platform/Window.hpp>

namespace nex::gui
{
	SceneGUI::SceneGUI(nex::Window* window,
		Picker* picker, 
		Camera* camera,
		const std::function<void()> exitCallback) :
		mOptionMenu(nullptr), 
		mFileMenu(nullptr), 
		mToolsMenu(nullptr),
		mExitCallback(std::move(exitCallback)),
		mPicker(picker)
	{
		std::unique_ptr<Menu> fileMenu = std::make_unique<Menu>("File");
		std::unique_ptr<MenuItem> exitMenuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
		{
			if (ImGui::MenuItem("Exit", "Esc"))
			{
				mExitCallback();
			}
		});

		fileMenu->addMenuItem(std::move(exitMenuItem));


		std::unique_ptr<Menu> optionMenu = std::make_unique<Menu>("Options");
		std::unique_ptr<Menu> toolsMenu = std::make_unique<Menu>("Tools");

		mFileMenu = fileMenu.get();
		mOptionMenu = optionMenu.get();
		mToolsMenu = toolsMenu.get();

		mMenuBar.addMenu(std::move(fileMenu));
		mMenuBar.addMenu(std::move(optionMenu));
		mMenuBar.addMenu(std::move(toolsMenu));

		mVobEditor = std::make_unique<VobEditor>(window, mPicker, camera);
	}

	MainMenuBar* SceneGUI::getMainMenuBar()
	{
		return &mMenuBar;
	}

	Menu* SceneGUI::getFileMenu() const
	{
		return mFileMenu;
	}

	Menu* SceneGUI::getOptionMenu() const
	{
		return mOptionMenu;
	}

	Menu * SceneGUI::getToolsMenu() const
	{
		return mToolsMenu;
	}

	nex::gui::VobEditor* SceneGUI::getVobEditor()
	{
		return mVobEditor.get();
	}

	nex::gui::Picker* SceneGUI::getPicker()
	{
		return mPicker;
	}

	void SceneGUI::drawSelf()
	{
		mMenuBar.drawGUI();
	}
}