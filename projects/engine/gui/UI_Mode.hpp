#ifndef UI_MODE_HPP
#define UI_MODE_HPP

class UI_ModeStateMachine;

#include <gui/View.hpp>

class UI_Mode {

public:

	UI_Mode(std::vector<std::unique_ptr<View>> views) : m_views(std::move(views)){}
	virtual ~UI_Mode() = default;
	virtual void frameUpdate(UI_ModeStateMachine& stateMachine) = 0;
	virtual void init() = 0;

	virtual void drawGUI() {
		for (auto& view : m_views)
			view->drawGUI();
	};

	virtual void addView(std::unique_ptr<View> view) {
		m_views.emplace_back(move(view));
	};

protected:
	std::vector<std::unique_ptr<View>> m_views;
};

#endif