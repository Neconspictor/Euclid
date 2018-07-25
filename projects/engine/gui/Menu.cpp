#include <gui/Menu.hpp>
#include <platform/gui/ImGUI.hpp>

namespace nex::engine::gui
{
	MenuItem::MenuItem(Callback callback) : m_callback(std::move(callback))
	{
	}

	void MenuItem::drawSelf()
	{
		m_callback(this);
	}

	void MenuItem::drawGUIWithParent(Menu* parent)
	{
		drawGUI();
	}

	Menu::Menu(const char* name) : View(), m_name(name), m_isSelected(false)
	{
	}

	void Menu::addMenuItem(MenuItemPtr menuItem)
	{
		m_menuItems.emplace_back(std::move(menuItem));
	}

	void Menu::drawSelf()
	{
		m_isSelected = ImGui::BeginMenu(m_name.c_str());
		if (m_isSelected)
		{
			for (auto& item : m_menuItems)
			{
				item->drawGUIWithParent(this);
			}
			ImGui::EndMenu();
		}
	}

	const std::vector<MenuItemPtr>& Menu::getMenuItems() const
	{
		return m_menuItems;
	}

	const std::string& Menu::getName() const
	{
		return m_name;
	}

	bool Menu::isSelected() const
	{
		return m_isSelected;
	}

	void MainMenuBar::drawSelf()
	{
		if (ImGui::BeginMainMenuBar())
		{
			for (auto& menu : m_menus)
			{
				menu->drawGUI();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void MainMenuBar::addMenu(MenuPtr menu)
	{
		m_menus.emplace_back(std::move(menu));
	}

}