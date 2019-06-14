#pragma once

#include <nex/gui/Controller.hpp>
#include <memory>

namespace nex::gui
{

	class ControllerStateMachine {

	public:

		using ControllerPtr = std::unique_ptr<Controller>;

		ControllerStateMachine(ControllerPtr controller);
		virtual ~ControllerStateMachine() = default;

		void frameUpdate(Real frameTime);

		Controller* getCurrentController();

		void init();

		void setCurrentController(ControllerPtr controller);

	private:
		ControllerPtr m_controller;
	};
}