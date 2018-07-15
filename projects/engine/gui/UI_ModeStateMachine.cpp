#include <gui/UI_ModeStateMachine.hpp>

UI_ModeStateMachine::UI_ModeStateMachine(std::unique_ptr<UI_Mode> mode) : m_uiMode(move(mode))
{
}

void UI_ModeStateMachine::drawGUI()
{
	m_uiMode->drawGUI();
}

void UI_ModeStateMachine::frameUpdate()
{
	m_uiMode->frameUpdate(*this);
}

UI_Mode * UI_ModeStateMachine::getUIMode()
{
	return m_uiMode.get();
}

void UI_ModeStateMachine::init()
{
	m_uiMode->init();
}

void UI_ModeStateMachine::setUIMode(std::unique_ptr<UI_Mode> mode)
{
	m_uiMode = std::move(mode);
}

void UI_ModeStateMachine::addView(std::unique_ptr<View> view)
{
	m_uiMode->addView(move(view));
}