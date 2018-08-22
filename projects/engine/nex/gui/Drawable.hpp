#pragma once

#include <nex/gui/Style.hpp>
#include <memory>
#include <vector>

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

		Drawable();

		virtual ~Drawable() = default;

		/**
		 * Draws the Gui of this Drawable and all of its children.
		 */
		virtual void drawGUI();

		void addChild(std::unique_ptr<Drawable> child);

		void useStyleClass(StyleClassPtr styleClass);

		void setVisible(bool visible);

		virtual bool isVisible() const;

		virtual const char* getID() const;

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
		Window(std::string name, bool useCloseCross);

		Window(std::string name, bool useCloseCross, int imGuiFlags);

		virtual ~Window() = default;

		void drawGUI() override;

	protected:

		void drawSelf() override;

		int m_imGuiFlags;
		std::string m_name;
		bool m_useCloseCross;
	};

	class TabBar;

	class Tab : public Drawable
	{

	protected:

		//friend TabBar;
		friend std::unique_ptr<Tab> std::make_unique<Tab>(std::string&&);

		/**
		* Note: A Tab has to be child of a Tab container.
		* For this the constructor of this class is protected and only
		* subclasses and the Tab container class can access it.
		*/
		Tab(std::string name);

	public:

		virtual ~Tab() = default;

		void drawGUI() override;

		const std::string& getName() const
		{
			return m_name;
		}

	protected:

		// not needed
		void drawSelf() override;

		std::string m_name;
	};

	class TabBar : public Drawable
	{
	public:

		TabBar(std::string name);

		virtual ~TabBar() = default;


		Tab* newTab(std::string tabName);

		Tab* getTab(const char* tabName);

		void drawGUI() override;

	protected:

		void drawSelf() override;

		std::string m_name;
	};

	class Container : public Drawable
	{
	public:
		virtual ~Container() = default;

	protected:
		void drawSelf() override;
	};
}