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
		ImGUI_TextureDesc desc;
		desc.texture = TextureManager::get()->getImage("_intern/icon/icon_menu_symbol.png");

		auto fileMenu = std::make_unique<ImageMenu>(desc, "File", false);
		auto exitMenuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
		{
			if (ImGui::MenuItem("Exit", "Esc"))
			{
				mExitCallback();
			}
		});

		auto subMenuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem){});

		auto subMenu = std::make_unique<ImageMenu>(desc, "SubFile");
		auto exitSubMenuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
			{
				if (ImGui::MenuItem("Exit", "Esc"))
				{
					mExitCallback();
				}
			});

		subMenu->addMenuItem(std::move(exitSubMenuItem));

		subMenuItem->addChild(std::move(subMenu));

		fileMenu->addMenuItem(std::move(exitMenuItem));
		fileMenu->addMenuItem(std::move(subMenuItem));


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