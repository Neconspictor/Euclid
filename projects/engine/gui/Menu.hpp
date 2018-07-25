#ifndef MENU_HPP
#define MENU_HPP

#include <gui/View.hpp>
#include <string>
#include <vector>
#include <functional>


namespace nex::engine::gui
{

	class Menu;
	class MenuItem;

	using MenuPtr = std::unique_ptr<Menu>;
	using MenuItemPtr = std::unique_ptr<MenuItem>;
	using Callback = std::function<void(MenuItem* menuItem)>;

	class MenuItem : public View
	{
	public:

		explicit MenuItem(Callback callback);

		virtual ~MenuItem() = default;

		virtual void drawGUIWithParent(Menu* parent);

	protected:

		void drawSelf() override;

		Callback m_callback;
	};

	class Menu : public View {

	public:
		explicit Menu(const char* name);

		virtual ~Menu() = default;

		void addMenuItem(MenuItemPtr menuItem);

		const std::vector<MenuItemPtr>& getMenuItems() const;

		const std::string& getName() const;

		/**
		* Checks whether the menu is currently selected (on the current frame).
		*/
		bool isSelected() const;

	protected:

		void drawSelf() override;

		std::vector<MenuItemPtr> m_menuItems;
		std::string m_name;
		bool m_isSelected;
	};

	class MainMenuBar : public View {
	public:
		void addMenu(MenuPtr menu);

	protected:

		void drawSelf() override;

		std::vector<MenuPtr> m_menus;
	};
}

#endif