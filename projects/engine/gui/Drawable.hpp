#pragma once

#include <gui/Style.hpp>
#include <imgui/imgui.h>

namespace nex::engine::gui
{

	/**
	 * A Drawable represents anything ui-related that should be rendered on screen.
	 * In ImGui, the counterparts of a Drawable are only rendered by ImGui if they are part of a ImGuiWindow (as children) or 
	 * if they are ImGuiWindows themselves. See also the documentation of the Window class for more information. 
	 */
	class Drawable {
	public:

		using StyleClassPtr = std::shared_ptr<StyleClass>;

		Drawable() : m_isVisible(true)
		{
			std::stringstream ss;
			ss << std::hex << reinterpret_cast<long long>(this);
			m_id = ss.str();
		}

		virtual ~Drawable() = default;

		/**
		 * Draws the Gui of this Drawable and all of its children.
		 */
		virtual void drawGUI()
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

			// Revert style class changes
			if (m_style) m_style->popStyleChanges();
		}

		void addChild(std::unique_ptr<Drawable> child)
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

		virtual bool isVisible() const
		{
			return m_isVisible;
		}

		virtual const char* getID() const
		{
			return m_id.c_str();
		}

	protected:

		/**
		 * Draws the GUI of this Drawable.
		 */
		virtual void drawSelf() = 0;

	protected:
		std::vector<std::unique_ptr<Drawable>> m_childs;
		StyleClassPtr m_style;
		bool m_isVisible;
		std::string m_id;
	};

	/**
	 * A Drawable that can be rendered independently from other Drawables is called a Window.
	 * Every Drawable has either be part (child) of a Window or has to be a Window itself in order to get rendered.
	 * This class is a counterpart to the ImGuiWindow class.
	 */
	class Window : public Drawable
	{
	public:
		Window(std::string name, bool useCloseCross) : Drawable(), 
			m_imGuiFlags(0), m_name(std::move(name)), m_useCloseCross(useCloseCross)
		{
			m_name += "###" + m_id;
		}
		Window(std::string name, bool useCloseCross, int imGuiFlags) : Drawable(), 
			m_imGuiFlags(imGuiFlags), m_name(std::move(name)), m_useCloseCross(useCloseCross)
		{
			m_name += "###" + m_id;
		}

		virtual ~Window() = default;

		void drawGUI() override
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
			ImGui::End();

			// Revert style class changes
			if (m_style) m_style->popStyleChanges();
		}

	protected:

		void drawSelf() override
		{
			if (m_useCloseCross)
				ImGui::Begin(m_name.c_str(), &m_isVisible, m_imGuiFlags);
			else 
				ImGui::Begin(m_name.c_str(), nullptr, m_imGuiFlags);
		}

		int m_imGuiFlags;
		std::string m_name;
		bool m_useCloseCross;
	};

	class Container : public Drawable
	{
	public:
		virtual ~Container() = default;

	protected:
		void drawSelf() override
		{
		}
	};
}