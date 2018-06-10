#pragma once

#include <ui_mode/UI_Mode.hpp>
#include <memory>

class UI_ModeStateMachine {

public:
	UI_ModeStateMachine(std::unique_ptr<UI_Mode> mode);
	~UI_ModeStateMachine() = default;

	void drawGUI();
	void frameUpdate();
	
	UI_Mode* getUIMode();

	void init();

	void setUIMode(std::unique_ptr<UI_Mode> mode);

private:
	std::unique_ptr<UI_Mode> m_uiMode;
};