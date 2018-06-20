#pragma once

#include <gui/UI_Mode.hpp>
#include <gui/View.hpp>
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

	void addView(std::unique_ptr<View> view);

private:
	std::unique_ptr<UI_Mode> m_uiMode;
};