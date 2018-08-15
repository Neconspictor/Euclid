#pragma once 

namespace nex::engine::gui
{
	class Style
	{
	public:
		virtual ~Style() = default;

		virtual void apply() = 0;
	};

	class StyleClass
	{
	public:

		using StyleClassPtr = std::shared_ptr<StyleClass>;

		virtual ~StyleClass() = default;

		/**
		 * Pushes ImGUI style changes for this style class.
		 * 
		 * An entity that wishes to use this style class has to call this function
		 * before it draws itself.
		 */
		virtual void pushStyleChanges()
		{
			pushStyleChangesSelf();
			for (auto& child : m_childs)
				child->pushStyleChanges();
		};

		/**
		* Pops ImGUI style changes for this style class. This function has to be called
		* after StyleClass::pushStyleChanges() has been called.
		* 
		* An entity that wishes to use this style class has to call this function
		* after it has drawn itself.
		*/
		virtual void popStyleChanges()
		{
			popStyleChangesSelf();
			for (auto& child : m_childs)
				child->popStyleChangesSelf();
		}

	protected:

		virtual void pushStyleChangesSelf() = 0;
		virtual void popStyleChangesSelf() = 0;

		std::vector<StyleClassPtr> m_childs;
	};
}