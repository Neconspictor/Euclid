#ifndef UI_MODE_HPP
#define UI_MODE_HPP

class ControllerStateMachine;

#include <gui/Drawable.hpp>
#include <utility>
#include <memory>
#include <platform/gui/ImGUI.hpp>

class Controller {

public:
	using Drawable = nex::engine::gui::Drawable;
	using DrawablePtr = nex::engine::gui::Drawable*;
	using ManagedDrawable = std::unique_ptr<Drawable>;

	Controller(ManagedDrawable drawable) : m_drawable(std::move(drawable)){}
	virtual ~Controller() = default;
	virtual void frameUpdate(ControllerStateMachine& stateMachine) = 0;
	virtual void init() = 0;

	DrawablePtr getView()const
	{
		return m_drawable.get();
	}

	virtual void render(ImGUI_Impl& guiRenderer)
	{
		guiRenderer.newFrame();
		m_drawable->drawGUI();
		ImGui::Render();
		guiRenderer.renderDrawData(ImGui::GetDrawData());
	}

	void setDrawable(std::unique_ptr<nex::engine::gui::Drawable> drawable) {
		m_drawable = std::move(drawable);
	};

protected:
	ManagedDrawable m_drawable;
};

#endif