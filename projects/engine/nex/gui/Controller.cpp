#include <nex/gui/Controller.hpp>

nex::gui::Controller::Controller(Input* input): mInput(input)
{
}

nex::gui::Controller::~Controller() = default;

void nex::gui::Controller::frameUpdate(float frameTime)
{
	frameUpdateSelf(frameTime);
	for (auto* child : mChilds)
	{
		child->frameUpdate(frameTime);
	}
}

void nex::gui::Controller::activate()
{
	for (auto* child : mChilds)
	{
		child->activate();
	}
}

bool nex::gui::Controller::isNotInterruptibleActionActive() const
{
	if (isNotInterruptibleActionActiveSelf()) return true;

	for (auto* child : mChilds)
	{
		if (child->isNotInterruptibleActionActive()) return true;
	}

	return false;
}

nex::gui::Drawable* nex::gui::Controller::getDrawable()
{
	return mDrawable;
}

void nex::gui::Controller::setDrawable(nex::gui::Drawable* drawable)
{
	mDrawable = drawable;
}

void nex::gui::Controller::addChild(nex::gui::Controller* controller)
{
	mChilds.push_back(controller);
}