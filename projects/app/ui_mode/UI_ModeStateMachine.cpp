#include <ui_mode/UI_ModeStateMachine.hpp>

UI_ModeStateMachine::UI_ModeStateMachine(std::unique_ptr<UI_Mode> mode) : m_uiMode(move(mode))
{
}

void UI_ModeStateMachine::frameUpdate()
{
	m_uiMode->frameUpdate(*this);
}

UI_Mode * UI_ModeStateMachine::getUIMode()
{
	return m_uiMode.get();
}

void UI_ModeStateMachine::setUIMode(std::unique_ptr<UI_Mode> mode)
{
	m_uiMode = std::move(mode);
}