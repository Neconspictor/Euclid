#include <gui/Menu.hpp>
#include <platform/gui/ImGUI.hpp>

Menu::Menu(const char* name): m_name(name)
{
}

void Menu::addMenuItem(MenuItem menuItem)
{
	m_menuItems.emplace_back(std::move(menuItem));
}

void Menu::addSubMenu(Menu subMenu)
{
	//m_subMenu = std::move(subMenu);
}

const std::vector<MenuItem>& Menu::getMenuItems() const
{
	return m_menuItems;
}

const std::string& Menu::getName() const
{
	return m_name;
}

void MenuBar::addMenuItem(MenuItem item, const char* menuName)
{
	auto& menu = getMenu(menuName);
	menu.addMenuItem(std::move(item));
}

void MenuBar::drawGUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		for (auto& menu : m_menus)
		{
			if (ImGui::BeginMenu(menu.getName().c_str()))
			{
				for (auto& item : menu.getMenuItems())
				{
					item();
				}
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}
}

Menu& MenuBar::getMenu(const char* name)
{
	for (auto& menu : m_menus)
	{
		if (menu.getName() == name)
			return menu;
	}

	m_menus.emplace_back(Menu(name));
	return m_menus.back();
}
