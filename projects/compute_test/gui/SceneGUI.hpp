#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/gui/Menu.hpp>
#include <nex/gui/ControllerStateMachine.hpp>


namespace nex::gui
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
