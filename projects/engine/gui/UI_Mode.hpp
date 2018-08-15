#ifndef UI_MODE_HPP
#define UI_MODE_HPP

class UI_ModeStateMachine;

#include <gui/Drawable.hpp>
#include <utility>
#include <memory>
#include <vector>
#include <platform/gui/ImGUI.hpp>

class UI_Mode {

public:
	using View = nex::engine::gui::Drawable;
	using ViewPtr = nex::engine::gui::Drawable*;
	using ManagedViewPtr = std::unique_ptr<View>;

	UI_Mode(ManagedViewPtr view) : m_view(std::move(view)){}
	virtual ~UI_Mode() = default;
	virtual void frameUpdate(UI_ModeStateMachine& stateMachine) = 0;
	virtual void init() = 0;

	ViewPtr getView()const
	{
		return m_view.get();
	}

	virtual void render(ImGUI_Impl& guiRenderer)
	{
		guiRenderer.newFrame();
		m_view->drawGUI();
		ImGui::Render();
		guiRenderer.renderDrawData(ImGui::GetDrawData());
	}

	void setView(std::unique_ptr<nex::engine::gui::Drawable> view) {
		m_view = std::move(view);
	};

protected:
	std::unique_ptr<nex::engine::gui::Drawable> m_view;
};

#endif