#ifndef MENU_HPP
#define MENU_HPP

#include <nex/gui/Drawable.hpp>
#include <string>
#include <vector>
#include <functional>
#include <imgui/imgui.h>


namespace nex::gui
{

	class Menu;
	class MenuItem;

	using MenuPtr = std::unique_ptr<Menu>;
	using MenuItemPtr = std::unique_ptr<MenuItem>;
	using Callback = std::function<void(MenuItem* menuItem)>;

	class GuiNotRenderedException : public std::runtime_error
	{
	public:
		GuiNotRenderedException(const char* msg) : std::runtime_error(msg) {}
	};

	class MenuItem : public Drawable
	{
	public:

		explicit MenuItem(Callback callback);

		virtual ~MenuItem() = default;

		virtual void drawGUIWithParent(Menu* parent);

	protected:

		void drawSelf() override;

		Callback m_callback;
	};

	class Menu : public Drawable {

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

	class MainMenuBar : public Drawable {
	public:
		void addMenu(MenuPtr menu);

		/**
		 * Provides the current position of the rendered main menu bar.
		 * This function can only be called if the main menu bar was rendered at least one time.
		 * @throws GuiNotRenderedException: If the main menu bar wasn't rendered at least one time.
		 */
		ImVec2 getPosition() const;

		/**
		* Provides the current size of the rendered main menu bar.
		* This function can only be called if the main menu bar was rendered at least one time.
		* @throws GuiNotRenderedException: If the main menu bar wasn't rendered at least one time.
		*/
		ImVec2 getSize() const;

	protected:

		void drawSelf() override;

		std::vector<MenuPtr> m_menus;
	};
}

#endif