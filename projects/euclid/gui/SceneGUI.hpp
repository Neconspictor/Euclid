#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/gui/Menu.hpp>
#include <nex/gui/ControllerStateMachine.hpp>
#include "nex/gui/TextureView.hpp"


namespace nex
{
	class Camera;
	class PerspectiveCamera;
	class Input;
	class Scene;
	class SceneNode;
	class Window;
	class Vob;
}

namespace nex::gui
{
	class SceneGUI : public Drawable
	{
	public:

		SceneGUI(std::function<void()> exitCallback);

		virtual ~SceneGUI() = default;

		Menu* getFileMenu() const;
		MainMenuBar* getMainMenuBar();
		Menu* getOptionMenu() const;
		Menu* getToolsMenu() const;

	protected:

		void drawSelf() override;
		
		MainMenuBar mMenuBar;
		Menu* mOptionMenu;
		Menu* mFileMenu;
		std::function<void()> mExitCallback;
		Menu* mToolsMenu;
	};
}