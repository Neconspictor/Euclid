#ifndef VIEW_HPP
#define VIEW_HPP

class UI_ModeStateMachine;

class View {

public:
	virtual ~View() = default;
	virtual void drawGUI() = 0;
};

#endif