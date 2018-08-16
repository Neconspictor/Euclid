#include <gui/ControllerStateMachine.hpp>

ControllerStateMachine::ControllerStateMachine(ControllerPtr controller) : m_controller(move(controller))
{
}

void ControllerStateMachine::frameUpdate()
{
	m_controller->frameUpdate(*this);
}

Controller * ControllerStateMachine::getCurrentController()
{
	return m_controller.get();
}

void ControllerStateMachine::init()
{
	m_controller->init();
}

void ControllerStateMachine::setCurrentController(ControllerPtr controller)
{
	m_controller = std::move(controller);
}