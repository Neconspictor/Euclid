#pragma once

#include <nex/gui/ImGUI.hpp>
#include <nex/gui/Controller.hpp>
#include <nex/camera/Camera.hpp>
#include "pbr_deferred/PBR_Deferred_Renderer.hpp"
#include "nex/gui/ControllerStateMachine.hpp"
#include "SceneGUI.hpp"


namespace nex {
	class Input;
	class Window;
	class PBR_Deferred_Renderer;
	}

namespace nex::gui
{
	class Picker;
	class Gizmo;

	class BaseController : public Controller {
	public:
		BaseController(nex::Window* window, Input* input, PBR_Deferred_Renderer* mainTask);
		virtual ~BaseController() = default;
		
		void frameUpdateSelf(float frameTime) override;
		void activateSelf() override;
		void deactivateSelf() override;

		bool isNotInterruptibleActionActiveSelf()const override;

		void handleExitEvent();

	protected:
		nex::Window * m_window;
		nex::Input* m_input;
		PBR_Deferred_Renderer* m_mainTask;
		nex::Logger m_logger;
	};

	class EditMode : public Controller {
	public:

		class GizmoGUI : public Drawable
		{
		public:
			GizmoGUI(Gizmo* gizmo, Input* input);

		protected:
			void drawSelf() override;

			Gizmo* mGizmo;
			Input* mInput;
		};

		EditMode(nex::Window* window,
			Input* input,
			PerspectiveCamera* camera,
			Scene* scene,
			SceneGUI* sceneGUI);
		virtual ~EditMode();

		/**
		 * Stuff that should always be updated.
		 */
		void updateAlways();

		void frameUpdateSelf(float frameTime) override;
		void activateSelf() override;
		void deactivateSelf() override;

		bool isNotInterruptibleActionActiveSelf()const override;

		Picker* getPicker();

	private:

		void activate(const Ray& ray);

		nex::Window* mWindow;
		PerspectiveCamera * mCamera;
		Scene* mScene;
		SceneGUI* mSceneGUI;
		std::unique_ptr<Picker> mPicker;
		std::unique_ptr<Gizmo> mGizmo;
		GizmoGUI mGizmoGUI;
		
	};

	class CameraMode : public Controller {
	public:
		CameraMode(nex::Window* window,
			Input* input,
			Camera* camera);
		virtual ~CameraMode();
		void frameUpdateSelf(float frameTime) override;

		void activateSelf() override;
		void deactivateSelf() override;

		bool isNotInterruptibleActionActiveSelf()const override;

	private:
		void updateCamera(float deltaTime);

		nex::Window* mWindow;
		Camera* mCamera;
	};

	class EngineController : public ControllerStateMachine
	{
	public:
		EngineController(nex::Window* window,
			Input* input,
			PBR_Deferred_Renderer* mainTask,
			PerspectiveCamera* camera,
			Scene* scene,
			ImGUI_Impl* guiImpl);

		EngineController(const EngineController&) = delete;
		EngineController(EngineController&&) = delete;
		EngineController& operator=(const EngineController&) = delete;
		EngineController& operator=(EngineController&&) = delete;
		
		virtual ~EngineController();


		void frameUpdateSelf(float frameTime) override;

		SceneGUI* getSceneGUI();
		EditMode* getEditMode();
	private:
		BaseController mBaseController;
		SceneGUI mSceneGUI;
		EditMode mEditMode;
		CameraMode mCameraMode;
		ImGUI_Impl* mGuiImpl;
		
	};
}