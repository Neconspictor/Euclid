#include <nex/gui/Menu.hpp>
#include <nex/gui/ImGUI.hpp>
#include <imgui/imgui_internal.h>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/gui/Util.hpp>

namespace nex::gui
{
	MenuItem::MenuItem(Callback callback) : mCallback(std::move(callback))
	{
	}

	void MenuItem::drawSelf()
	{
		mCallback(this);
	}

	void MenuItem::drawGUIWithParent(Menu* parent)
	{
		drawGUI();
	}

	Menu::Menu(const char* name) : Drawable(), mName(name), mIsSelected(false)
	{
		mName += "###" + mId;
	}

	void Menu::addMenuItem(MenuItemPtr menuItem)
	{
		mMenuItems.emplace_back(std::move(menuItem));
	}

	void Menu::drawSelf()
	{
		mIsSelected = ImGui::BeginMenu(mName.c_str());
		if (mIsSelected)
		{
			for (auto& item : mMenuItems)
			{
				item->drawGUIWithParent(this);
			}
			ImGui::EndMenu();
		}
	}

	const std::vector<MenuItemPtr>& Menu::getMenuItems() const
	{
		return mMenuItems;
	}

	const std::string& Menu::getName() const
	{
		return mName;
	}

	bool Menu::isSelected() const
	{
		return mIsSelected;
	}

	void MainMenuBar::drawSelf()
	{
		if (ImGui::BeginMainMenuBar())
		{
			for (auto& menu : mMenus)
			{
				menu->drawGUI();
			}
			ImGui::EndMainMenuBar();
		}
	}

	ImVec2 MainMenuBar::getPosition() const
	{
		ImGuiWindow* window = ImGui::FindWindowByName("##MainMenuBar");
		if (window == nullptr) throw_with_trace(GuiNotRenderedException("Main menu bar has to be rendered once before calling MainMenuBar::getPosition()"));
		return window->Pos;
	}

	ImVec2 MainMenuBar::getSize() const
	{
		ImGuiWindow* window = ImGui::FindWindowByName("##MainMenuBar");
		if (window == nullptr) throw_with_trace(GuiNotRenderedException("Main menu bar has to be rendered once before calling MainMenuBar::getSize()"));
		return window->Size;
	}

	void MainMenuBar::addMenu(MenuPtr menu)
	{
		mMenus.emplace_back(std::move(menu));
	}

	ImageMenu::ImageMenu(const ImGUI_TextureDesc& textureDesc, const char* name) : 
		Menu(name), mTextureDesc(textureDesc)
	{
	}
	void ImageMenu::drawSelf()
	{
		

		//ImGui::BeginGroup();
		auto* window = ImGui::GetCurrentWindow();
		auto width = window->DC.CurrLineSize.y - window->WindowPadding.y;
		//ImGui::Selectable("##dummy", &mIsSelected, 0, ImVec2(width, 0));



		mIsSelected = nex::gui::BeginMenuCustom("##dummy", ImVec2(width, 0));
		//ImGui::SameLine(-16, 16.0f);
		//ImGui::Image((void*)&mTextureDesc, ImVec2(width, width));

		
		

		
		
		//ImGui::EndGroup();

		if (mIsSelected)
		{
			//if (ImGui::BeginPopup("##dummy2")) {
				
				for (auto& item : mMenuItems)
				{
					item->drawGUIWithParent(this);
				}
				//ImGui::EndPopup();
			//}

			
			ImGui::EndMenu();
			
		}

	}
}