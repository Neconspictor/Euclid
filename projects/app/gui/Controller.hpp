#pragma once

#include <nex/gui/ImGUI.hpp>
#include <nex/logging/LoggingClient.hpp>
#include <nex/gui/Controller.hpp>
#include <nex/camera/Camera.hpp>


class Input;
class PBR_Deferred_Renderer;

namespace App
{
	class BaseController : public Controller {
	public:
		BaseController(Window* window, Input* input, PBR_Deferred_Renderer* mainTask, ImGUI_Impl* guiRenderer, std::unique_ptr<nex::engine::gui::Drawable> drawable);
		virtual ~BaseController() = default;

		void frameUpdate(ControllerStateMachine& stateMachine, float frameTime) override;
		void init() override;

		virtual void handleExitEvent();

	protected:
		Window * m_window;
		Input* m_input;
		ImGUI_Impl* guiRenderer;
		PBR_Deferred_Renderer* m_mainTask;
		nex::LoggingClient logClient;
	};

	class EditMode : public BaseController {
	public:
		EditMode(Window* window, 
			Input* input, 
			PBR_Deferred_Renderer* mainTask, 
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
			PBR_Deferred_Renderer* mainTask, 
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