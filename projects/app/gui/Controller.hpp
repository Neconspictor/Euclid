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
		EditMode(nex::Window* window,
			Input* input,
			Camera* camera);
		virtual ~EditMode();
		void frameUpdateSelf(float frameTime) override;
		void activateSelf() override;
		bool isNotInterruptibleActionActiveSelf()const override;

	private:
		nex::Window* mWindow;
		Camera * mCamera;
		std::unique_ptr<Picker> mPicker;
		std::unique_ptr<Gizmo> mGizmo;
	};

	class CameraMode : public Controller {
	public:
		CameraMode(nex::Window* window,
			Input* input,
			Camera* camera);
		virtual ~CameraMode();
		void frameUpdateSelf(float frameTime) override;

		void activateSelf() override;
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
			Camera* camera);

		EngineController(const EngineController&) = delete;
		EngineController(EngineController&&) = delete;
		EngineController& operator=(const EngineController&) = delete;
		EngineController& operator=(EngineController&&) = delete;
		
		virtual ~EngineController();


		void frameUpdateSelf(float frameTime) override;

		SceneGUI* getSceneGUI();
	private:
		BaseController mBaseController;
		EditMode mEditMode;
		CameraMode mCameraMode;
		SceneGUI mSceneGUI;
	};
}