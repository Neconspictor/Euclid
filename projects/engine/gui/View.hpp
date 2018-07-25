#pragma once


class UI_ModeStateMachine;

namespace nex::engine::gui
{
	class View {
	public:
		virtual ~View() = default;
		
		virtual void drawGUI() final
		{
			drawSelf();
			for (auto& child : m_childs)
				child->drawGUI();
		}

		void addChild(std::unique_ptr<View> child)
		{
			m_childs.emplace_back(std::move(child));
		}

	protected:
		virtual void drawSelf() = 0;

	protected:
		std::vector<std::unique_ptr<View>> m_childs;
	};

	class Container : public View
	{
	public:
		Container() : View(){}

		virtual ~Container() = default;

	protected:
		void drawSelf() override
		{
		}


	};
}