#pragma once

#include <platform/gui/ImGUI.hpp>
#include <platform/logging/LoggingClient.hpp>
#include "../../engine/gui/Controller.hpp"
#include <camera/Camera.hpp>


class Input;
class PBR_Deferred_MainLoopTask;

namespace App
{
	class BaseController : public Controller {
	public:
		BaseController(Window* window, Input* input, PBR_Deferred_MainLoopTask* mainTask, ImGUI_Impl* guiRenderer, std::unique_ptr<nex::engine::gui::Drawable> drawable);
		virtual ~BaseController() = default;

		void frameUpdate(ControllerStateMachine& stateMachine, float frameTime) override;
		void init() override;

		virtual void handleExitEvent();

	protected:
		Window * m_window;
		Input* m_input;
		ImGUI_Impl* guiRenderer;
		PBR_Deferred_MainLoopTask* m_mainTask;
		platform::LoggingClient logClient;
	};

	class EditMode : public BaseController {
	public:
		EditMode(Window* window, 
			Input* input, 
			PBR_Deferred_MainLoopTask* mainTask, 
			Camera* camera, 
			ImGUI_Impl* guiRenderer, 
			std::unique_ptr<nex::engine::gui::Drawable> drawable);
		virtual ~EditMode() = default;
		void frameUpdate(ControllerStateMachine& stateMachine, float frameTime) override;

	private:
		Camera * m_camera;
	};

	class CameraMode : public BaseController {
	public:
		CameraMode(Window* window, 
			Input* input, 
			PBR_Deferred_MainLoopTask* mainTask, 
			Camera* camera, 
			ImGUI_Impl* guiRenderer, 
			std::unique_ptr<nex::engine::gui::Drawable> drawable);
		virtual ~CameraMode() = default;
		void frameUpdate(ControllerStateMachine& stateMachine, float frameTime) override;

	private:
		void updateCamera(Input* input, float deltaTime);

		Window* m_window;
		Camera* m_camera;
	};

}