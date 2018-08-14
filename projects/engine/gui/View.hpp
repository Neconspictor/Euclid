#pragma once

#include <gui/Style.hpp>


class UI_ModeStateMachine;

namespace nex::engine::gui
{

	class View {
	public:

		using StyleClassPtr = std::shared_ptr<StyleClass>;

		virtual ~View() = default;
		
		virtual void drawGUI() final
		{
			// Apply style class changes
			if (m_style) m_style->pushStyleChanges();

			drawSelf();
			for (auto& child : m_childs)
				child->drawGUI();

			// Revert style class changes
			if (m_style) m_style->popStyleChanges();
		}

		void addChild(std::unique_ptr<View> child)
		{
			m_childs.emplace_back(std::move(child));
		}

		void useStyleClass(StyleClassPtr styleClass)
		{
			m_style = std::move(styleClass);
		}

	protected:
		virtual void drawSelf() = 0;

	protected:
		std::vector<std::unique_ptr<View>> m_childs;
		StyleClassPtr m_style;
	};

	class Container : public View
	{
	public:
		virtual ~Container() = default;

	protected:
		void drawSelf() override
		{
		}
	};
}