#pragma once

#include <gui/Drawable.hpp>
#include <gui/Menu.hpp>
#include <gui/ControllerStateMachine.hpp>


namespace nex::engine::gui
{
	class SceneGUI : public Drawable
	{
	public:

		SceneGUI(ControllerStateMachine* controllerSM);

		virtual ~SceneGUI() = default;

		Menu* getFileMenu() const;

		Menu* getOptionMenu() const;

		MainMenuBar* getMainMenuBar();

	protected:

		void drawSelf() override;
		
		MainMenuBar m_menuBar;
		Menu* m_optionMenu;
		Menu* m_fileMenu;
		ControllerStateMachine* m_controllerSM;
	};
}
