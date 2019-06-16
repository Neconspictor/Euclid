#include <nex/gui/ControllerStateMachine.hpp>

nex::gui::ControllerStateMachine::ControllerStateMachine(Input* input, ControllerPtr controller) : Controller(input), mActiveController(controller)
{
}

void nex::gui::ControllerStateMachine::frameUpdateSelf(float frameTime)
{
	if (mActiveController)
		mActiveController->frameUpdate(frameTime);
}

nex::gui::Controller * nex::gui::ControllerStateMachine::getActiveController()
{
	return mActiveController;
}

void nex::gui::ControllerStateMachine::activateSelf()
{
	if (mActiveController)
		mActiveController->activate();
}

void nex::gui::ControllerStateMachine::deactivateSelf()
{
	if (mActiveController)
		mActiveController->deactivateSelf();
}

void nex::gui::ControllerStateMachine::setActiveController(ControllerPtr controller)
{
	if (mActiveController != nullptr)
		mActiveController->deactivate();

	mActiveController = controller;
	mActiveController->activate();
}

bool nex::gui::ControllerStateMachine::isNotInterruptibleActionActiveSelf() const
{
	return mActiveController->isNotInterruptibleActionActiveSelf();
}