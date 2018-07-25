#pragma once

#include <gui/View.hpp>
#include <gui/Menu.hpp>
#include <string>
#include <vector>
#include <functional>


namespace nex::engine::gui
{
	class SceneGUI : public View
	{
	public:

		SceneGUI();

		virtual ~SceneGUI() = default;

		Menu* getFileMenu() const;

		Menu* getOptionMenu() const;

	protected:

		void drawSelf() override;
		
		MainMenuBar m_menuBar;
		Menu* m_optionMenu;
		Menu* m_fileMenu;
	};
}