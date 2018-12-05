#ifndef UI_MODE_HPP
#define UI_MODE_HPP

#include <nex/gui/Drawable.hpp>
#include <utility>
#include <memory>
#include <nex/gui/ImGUI.hpp>

class ControllerStateMachine;

namespace nex::gui
{

	class Controller {

	public:
		using Drawable = nex::gui::Drawable;
		using DrawablePtr = nex::gui::Drawable*;
		using ManagedDrawable = std::unique_ptr<Drawable>;

		Controller(ManagedDrawable drawable) : m_drawable(std::move(drawable)) {}
		virtual ~Controller() = default;
		virtual void frameUpdate(ControllerStateMachine& stateMachine, float frameTime) = 0;
		virtual void init() = 0;

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