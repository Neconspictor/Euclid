#pragma once

#include <gui/Drawable.hpp>
#include <gui/Menu.hpp>
#include <string>
#include <vector>
#include <functional>


namespace nex::engine::gui
{
	class SceneGUI : public Drawable
	{
	public:

		SceneGUI();

		virtual ~SceneGUI() = default;

		Menu* getFileMenu() const;

		Menu* getOptionMenu() const;

		MainMenuBar* getMainMenuBar();

	protected:

		void drawSelf() override;
		
		MainMenuBar m_menuBar;
		Menu* m_optionMenu;
		Menu* m_fileMenu;
	};
}