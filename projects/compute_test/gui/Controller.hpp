#pragma once

#include <nex/gui/ImGUI.hpp>
#include <nex/gui/Controller.hpp>
#include <nex/camera/Camera.hpp>


namespace nex {
	class Renderer;
}

class Input;

namespace nex::gui
{
	class BaseController : public Controller {
	public:
		BaseController(nex::Window* window, Input* input, Renderer* mainTask, ImGUI_Impl* guiRenderer, std::unique_ptr<nex::gui::Drawable> drawable);
		virtual ~BaseController() = default;

		void frameUpdate(ControllerStateMachine& stateMachine, float frameTime) override;
		void init() override;

		virtual void handleExitEvent();

	protected:
		nex::Window * m_window;
		nex::Input* m_input;
		ImGUI_Impl* guiRenderer;
		Renderer* m_mainTask;
		nex::Logger m_logger;
	};

	class EditMode : public BaseController {
	public:
		EditMode(nex::Window* window,
			Input* input, 
			Renderer* mainTask, 
			Camera* camera, 
			ImGUI_Impl* guiRenderer, 
			std::unique_ptr<nex::gui::Drawable> drawable);
		virtual ~EditMode() = default;
		void frameUpdate(ControllerStateMachine& stateMachine, float frameTime) override;

	private:
		Camera * m_camera;
	};

	class CameraMode : public BaseController {
	public:
		CameraMode(nex::Window* window,
			Input* input, 
			Renderer* mainTask, 
			Camera* camera, 
			ImGUI_Impl* guiRenderer, 
			std::unique_ptr<nex::gui::Drawable> drawable);
		virtual ~CameraMode() = default;
		void frameUpdate(ControllerStateMachine& stateMachine, float frameTime) override;

	private:
		void updateCamera(Input* input, float deltaTime);

		nex::Window* m_window;
		Camera* m_camera;
	};

}