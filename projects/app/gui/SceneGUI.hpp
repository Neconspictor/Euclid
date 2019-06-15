#pragma once

#include <nex/gui/Drawable.hpp>
#include <nex/gui/Menu.hpp>
#include <nex/gui/ControllerStateMachine.hpp>
#include "nex/math/Ray.hpp"


namespace nex
{
	class Camera;
	class PerspectiveCamera;
	class Input;
	class Scene;
	class SceneNode;
}

namespace nex::gui
{
	class Picker;
	class Gizmo;

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

	class SceneNodeProperty : public nex::gui::Drawable
	{
	public:
		SceneNodeProperty();
		virtual ~SceneNodeProperty();
		void setPicker(Picker* picker);

		void update(Scene& scene, const PerspectiveCamera& camera, const Input& input);

	protected:
		void activate(Scene& scene, const Ray& ray, const Camera& camera);

		void drawSelf() override;

		Picker* mPicker;
		std::unique_ptr<gui::Gizmo> mGizmo;
	};
}