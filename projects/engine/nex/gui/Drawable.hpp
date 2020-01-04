#pragma once

#include <nex/gui/Style.hpp>
#include <memory>
#include <vector>
#include <functional>
#include <nex/gui/ImGUI.hpp>

namespace nex::gui
{

	/**
	 * A Drawable represents anything ui-related that should be rendered on screen.
	 * In ImGui, the counterparts of a Drawable are only rendered by ImGui if they are part of a ImGuiWindow (as children) or 
	 * if they are ImGuiWindows themselves. See also the documentation of the Window class for more information. 
	 */
	class Drawable {
	public:

		using StyleClassPtr = std::shared_ptr<StyleClass>;

		Drawable(bool isVisibleDefaultState = true);

		virtual ~Drawable() = default;

		/**
		 * Draws the Gui of this Drawable and all of its children.
		 */
		virtual void drawGUI();

		/**
		 * Adds a child to this Drawable. The Drawable manages the lifetime of the child to be added.
		 */
		void addChild(std::unique_ptr<Drawable> child);

		/**
		 * Adds a child to this Drawable. The caller is responsible for the lifetime
		 * of the child to be added.
		 */
		void addChild(Drawable* child);

		std::vector<Drawable*>& getReferencedChilds();

		void useStyleClass(StyleClassPtr styleClass);

		virtual void setVisible(bool visible);

		virtual bool isVisible() const;

		virtual const char* getID() const;

		void onCanvasResize(unsigned width, unsigned height);

	protected:

		/**
		 * Draws the GUI of this Drawable.
		 */
		virtual void drawSelf();

		void drawChilds();
		void drawContent();

		virtual void onCanvasResizeSelf(unsigned width, unsigned height);

	protected:
		std::vector<std::unique_ptr<Drawable>> mChilds;
		std::vector<Drawable*> mReferencedChilds;
		StyleClassPtr mStyle;
		bool mIsVisible;
		std::string mId;
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

		int getImGuiFlags() const;
		void setImGuiFlags(int flags);

		void setExplicitContentSize(const ImVec2& size);

	protected:

		int mImGuiFlags;
		std::string mName;
		bool mUseCloseCross;
		ImVec2 mExplicitContentSize;
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
			return mName;
		}

	protected:

		std::string mName;
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

		std::string mName;
	};

	class Button : public Drawable {
	public:

		Button(std::string title);

		void setAction(std::function<void()> action);
		void enable(bool enabled);

		void drawSelf() override;

		bool drawImmediate();

	protected:

		class ButtonStyle : public StyleClass {

		protected:
			virtual void pushStyleChangesSelf() override;
			virtual void popStyleChangesSelf() override;
		};

		std::function<void()> mAction;
		bool mEnabled;
		std::string mTitle;

	};

	class ApplyButton : public Drawable {
	public:
		ApplyButton(std::function<void()> apply, std::function<void()> revert, std::function<bool()> condition = []() {return true; });

		void drawSelf() override;

	protected:
		std::function<void()> mApply;
		std::function<void()> mRevert;
		std::function<bool()> mCondition;
	};
}