#ifndef MENU_HPP
#define MENU_HPP

#include <gui/View.hpp>
#include <string>
#include <vector>
#include <functional>
//#include <optional>

using MenuItem = std::function<void()>;

class Menu {

public:
	explicit Menu(const char* name);

	void addMenuItem(MenuItem menuItem);

	void addSubMenu(Menu subMenu);

	const std::vector<MenuItem>& getMenuItems() const;

	const std::string& getName() const;

protected:
	std::vector<MenuItem> m_menuItems;
	std::string m_name;
	//std::optional<Menu> m_subMenu;
};

class MenuBar : public View {
public:
	virtual void addMenuItem(MenuItem item, const char* menuName);

	virtual void drawGUI() override;

protected:

	virtual Menu& getMenu(const char* name);
protected:
	std::vector<Menu> m_menus;
};

#endif