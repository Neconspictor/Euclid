#include <nex/gui/ControllerStateMachine.hpp>

namespace nex::gui
{

	ControllerStateMachine::ControllerStateMachine(ControllerPtr controller) : m_controller(move(controller))
	{
	}

	void ControllerStateMachine::frameUpdate(float frameTime)
	{
		m_controller->frameUpdate(*this, frameTime);
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
}