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
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoCollapse;

		MenuWindow(std::string title, 
			MainMenuBar* menuBar, 
			Menu* menu,
			int flags = DEFAULT_FLAGS);

		void drawGUI() override;

	protected:
		bool hasVisibleChild() const;

		void drawSelf() override;

		nex::gui::MainMenuBar* mMainMenuBar;
		bool mSetDefaultPosition;
	};
}