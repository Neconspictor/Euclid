#pragma once
#include <nex/gui/Drawable.hpp>
#include <nex/gui/Menu.hpp>

namespace nex::gui
{
    /**
     * An utility class for windows that should be able to open via a menu entry 
     * from a MainMenuBar.
     */ 
	class MenuWindow : public nex::gui::Window
	{
	public:

		static constexpr int DEFAULT_FLAGS = 
			//ImGuiWindowFlags_NoMove 
			ImGuiWindowFlags_AlwaysAutoResize 
			//| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_HorizontalScrollbar;

		static constexpr int COMMON_FLAGS =
			//ImGuiWindowFlags_NoMove 
			//| ImGuiWindowFlags_NoResize
			ImGuiWindowFlags_NoCollapse;


		MenuWindow(std::string title, 
			MainMenuBar* menuBar, 
			Menu* menu,
			int flags = DEFAULT_FLAGS);

		MenuWindow(std::string title,
			MainMenuBar* menuBar,
			Menu* menu,
			std::function<void()> drawFunc,
			int flags = DEFAULT_FLAGS);

		void drawGUI() override;

	protected:

		void drawSelf() override;
		bool hasVisibleChild() const;

		nex::gui::MainMenuBar* mMainMenuBar;
		bool mSetDefaultPosition;
		std::optional<std::function<void()>> mDrawFunc;
	};


	class MenuDrawable : public nex::gui::Drawable
	{
	public:

		using Drawer = std::function<void(const ImVec2 & menubarPos, float menuBarHeight, bool& isVisible)>;

		MenuDrawable(const std::string& title,
			MainMenuBar* menuBar,
			Menu* menu,
			bool useDefaultStartPosition);

		MenuDrawable(const std::string& title,
			MainMenuBar* menuBar,
			Menu* menu,
			bool useDefaultStartPosition,
			Drawer drawFunc);

	protected:

		void drawSelf() override;

		nex::gui::MainMenuBar* mMainMenuBar;
		bool mSetDefaultPosition;
		std::optional<Drawer> mDrawFunc;
		std::string mName;
		bool mInit = true;
		bool mUseDefaultStartPosition;
	};
}