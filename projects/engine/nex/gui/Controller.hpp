#ifndef UI_MODE_HPP
#define UI_MODE_HPP

#include <nex/gui/Drawable.hpp>
#include <utility>
#include <memory>
#include <nex/gui/ImGUI.hpp>

namespace nex::gui
{
	class ControllerStateMachine;

	class Controller {

	public:
		using Drawable = nex::gui::Drawable;
		using DrawablePtr = nex::gui::Drawable*;
		using ManagedDrawable = std::unique_ptr<Drawable>;

		Controller(ManagedDrawable drawable) : m_drawable(std::move(drawable)) {}
		virtual ~Controller() = default;
		virtual void frameUpdate(nex::gui::ControllerStateMachine& stateMachine, Real frameTime) = 0;
		virtual void init() = 0;

		/**
		 * Checks if a not interruptible  action is active
		 */
		virtual bool isNotInterruptibleActionActive()const = 0;

		DrawablePtr getDrawable()const
		{
			return m_drawable.get();
		}

		void setDrawable(std::unique_ptr<nex::gui::Drawable> drawable) {
			m_drawable = std::move(drawable);
		};

	protected:
		ManagedDrawable m_drawable;
	};

}

#endif