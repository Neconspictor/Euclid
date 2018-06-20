#ifndef MENU_HPP
#define MENU_HPP

#include <gui/View.hpp>
#include <string>
#include <vector>
#include <platform/gui/ImGUI.hpp>

using MenuItem = std::function<void()>;

class Menu {

public:
	Menu(const char* name) : m_name(name) {};
	virtual ~Menu() = default;

	void addMenuItem(MenuItem menuItem) {
		m_menuItems.push_back(menuItem);
	}

	const std::vector<MenuItem>& getMenuItems() const {
		return m_menuItems;
	}

	const std::string& getName() const {
		return m_name;
	}

protected:
	std::vector<MenuItem> m_menuItems;
	std::string m_name;
};

class MenuBar : public View {
public:
	virtual ~MenuBar() = default;

	virtual void addMenuItem(MenuItem item, const char* menuName) {
		Menu& menu = getMenu(menuName);
		menu.addMenuItem(item);
	}
	virtual void drawGUI() override {
		if (ImGui::BeginMainMenuBar())
		{
			for (auto& menu : m_menus) {
				if (ImGui::BeginMenu(menu.getName().c_str()))
				{
					for (auto& item : menu.getMenuItems()) {
						item();
					}
					ImGui::EndMenu();
				}
			}
			ImGui::EndMainMenuBar();
		}
	}

protected:
	virtual Menu& getMenu(const char* name) {
		for (auto& menu : m_menus) {
			if (menu.getName().compare(name) == 0)
				return menu;
		}

		m_menus.emplace_back(Menu(name));
		return m_menus.back();
	}
protected:
	std::vector<Menu> m_menus;
};

#endif