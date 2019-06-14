#include <nex/gui/ControllerStateMachine.hpp>

nex::gui::ControllerStateMachine::ControllerStateMachine(ControllerPtr controller) : m_controller(move(controller))
{
}

void nex::gui::ControllerStateMachine::frameUpdate(Real frameTime)
{
	m_controller->frameUpdate(*this, frameTime);
}

nex::gui::Controller * nex::gui::ControllerStateMachine::getCurrentController()
{
	return m_controller.get();
}

void nex::gui::ControllerStateMachine::init()
{
	m_controller->init();
}

void nex::gui::ControllerStateMachine::setCurrentController(ControllerPtr controller)
{
	m_controller = std::move(controller);
}