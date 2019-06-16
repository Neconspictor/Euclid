#include <nex/gui/ControllerStateMachine.hpp>

nex::gui::ControllerStateMachine::ControllerStateMachine(Input* input, ControllerPtr controller) : Controller(input), mActiveController(controller)
{
}

void nex::gui::ControllerStateMachine::frameUpdateSelf(float frameTime)
{
	mActiveController->frameUpdate(frameTime);
}

nex::gui::Controller * nex::gui::ControllerStateMachine::getActiveController()
{
	return mActiveController;
}

void nex::gui::ControllerStateMachine::activateSelf()
{
	mActiveController->activate();
}

void nex::gui::ControllerStateMachine::setActiveController(ControllerPtr controller)
{
	mActiveController = controller;
}

bool nex::gui::ControllerStateMachine::isNotInterruptibleActionActiveSelf() const
{
	return mActiveController->isNotInterruptibleActionActiveSelf();
}