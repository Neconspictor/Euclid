#include <gui/SceneGUI.hpp>
#include <gui/Controller.hpp>
#include "nex/gui/ImGUI_Extension.hpp"
#include <nex/platform/Input.hpp>
#include <nex/platform/Window.hpp>
#include <nex/gui/Dockspace.hpp>

namespace nex::gui
{

	class CustomStyleClass : public StyleClass {
	public:
	protected:
		void pushStyleChangesSelf() override {

			ImGuiStyle& style = ImGui::GetStyle();
			ImGui::PushStyleColor(ImGuiCol_WindowBg, style.Colors[ImGuiCol_WindowBg]);
			ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0, 0, 0, 0));
			//ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
		}
		void popStyleChangesSelf() override {
			ImGui::PopStyleColor(2);
		}
	};

	//ImGuiStyle & style = ImGui::GetStyle();

	SceneGUI::SceneGUI(nex::Window* window,
		Picker* picker, 
		Scene* scene,
		Camera* camera,
		const std::function<void()> exitCallback) :
		mOptionMenu(nullptr), 
		mFileMenu(nullptr), 
		mToolsMenu(nullptr),
		mExitCallback(std::move(exitCallback)),
		mPicker(picker)
	{
		ImGUI_TextureDesc desc;
		desc.texture = TextureManager::get()->getImage("_intern/icon/icon_menu_symbol.png", false);

		auto fileMenu = std::make_unique<ImageMenu>(desc, "##File", false);
		auto exitMenuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
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

		mMenuBar.useStyleClass(std::make_shared<CustomStyleClass>());

		mVobEditor = std::make_unique<VobEditor>(window, mPicker, scene, camera);

		addChild(std::make_unique<Dockspace>());

		// Note: Ensure that main menu bar is called after dockspace!
		addChild(&mMenuBar);

		//Ensure that docking is enabled at the beginning
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
	}
}