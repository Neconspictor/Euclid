#pragma once

#include <platform/gui/ImGUI.hpp>
#include <platform/logging/LoggingClient.hpp>
#include "../../engine/gui/Controller.hpp"


class Input;
class PBR_Deferred_MainLoopTask;

namespace App
{
	class BaseController : public Controller {
	public:
		BaseController(PBR_Deferred_MainLoopTask& mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::Drawable> drawable);
		virtual ~BaseController() = default;

		void frameUpdate(ControllerStateMachine& stateMachine) override;
		void init() override;

		virtual void handleExitEvent();

	protected:
		PBR_Deferred_MainLoopTask * mainTask;
		ImGUI_Impl* guiRenderer;
		platform::LoggingClient logClient;
	};

	class EditMode : public BaseController {
	public:
		EditMode(PBR_Deferred_MainLoopTask& mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::Drawable> drawable);
		virtual ~EditMode() = default;
		void frameUpdate(ControllerStateMachine& stateMachine) override;
	};

	class CameraMode : public BaseController {
	public:
		CameraMode(PBR_Deferred_MainLoopTask& mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::Drawable> drawable);
		virtual ~CameraMode() = default;
		void frameUpdate(ControllerStateMachine& stateMachine) override;

	private:
		void updateCamera(Input* input, float deltaTime);
	};

}
