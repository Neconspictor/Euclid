#pragma once

#include <nex/gui/Controller.hpp>
#include <memory>

namespace nex::gui
{

	class ControllerStateMachine : public Controller {

	public:

		using ControllerPtr = Controller*;

		ControllerStateMachine(Input* input, ControllerPtr controller);
		virtual ~ControllerStateMachine() = default;

		void frameUpdateSelf(float frameTime) override;

		Controller* getActiveController();

		void activateSelf() override;

		void setActiveController(ControllerPtr controller);

		bool isNotInterruptibleActionActiveSelf()const override;

	protected:
		ControllerPtr mActiveController;
	};
}