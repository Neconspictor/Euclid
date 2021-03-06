#pragma once

#include <nex/gui/Drawable.hpp>
#include <utility>
#include <memory>
#include <nex/gui/ImGUI.hpp>

namespace nex
{
	class Input;
}

namespace nex::gui
{
	class Controller {

	public:
		Controller(Input* input);
		virtual ~Controller();

		virtual void frameUpdateSelf(float frameTime) = 0;
		
		virtual void frameUpdate(float frameTime);


		virtual void activateSelf() = 0;
		virtual void activate();

		virtual void deactivateSelf() = 0;
		virtual void deactivate();

		/**
		 * Checks if a not interruptible  action is active
		 */
		virtual bool isNotInterruptibleActionActiveSelf()const = 0;
		virtual bool isNotInterruptibleActionActive()const;

		bool allowsInputForUI() const;

		Drawable* getDrawable();

		void setDrawable(Drawable* drawable);;

		void addChild(Controller* controller);

	protected:
		Drawable* mDrawable;
		std::vector<Controller*> mChilds;
		Input* mInput;
		bool mIsActivated;
		bool mAllowInputForUi;
	};

}