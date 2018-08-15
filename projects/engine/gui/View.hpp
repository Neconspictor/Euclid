#pragma once

#include <gui/Style.hpp>


class UI_ModeStateMachine;

namespace nex::engine::gui
{

	class View {
	public:

		using StyleClassPtr = std::shared_ptr<StyleClass>;

		View() : m_isVisible(true){}

		virtual ~View() = default;

		virtual void drawGUI() final
		{
			// Do not draw gui if this view is invisible!
			if (!m_isVisible) return;

			// Apply style class changes
			if (m_style) m_style->pushStyleChanges();

			drawSelf();
			for (auto& child : m_childs)
			{
				if (child->isVisible())
					child->drawGUI();
			}

			drawSelfAfterChildren();

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

		void setVisible(bool visible)
		{
			m_isVisible = visible;
		}

		bool isVisible() const
		{
			return m_isVisible;
		}

	protected:
		virtual void drawSelf() = 0;

		virtual void drawSelfAfterChildren() {}

	protected:
		std::vector<std::unique_ptr<View>> m_childs;
		StyleClassPtr m_style;
		bool m_isVisible;
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