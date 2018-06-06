#ifndef UI_MODE_HPP
#define UI_MODE_HPP

class UI_ModeStateMachine;

class UI_Mode {

public:
	virtual ~UI_Mode() = default;
	virtual void frameUpdate(UI_ModeStateMachine& stateMachine) = 0;
};

#endif