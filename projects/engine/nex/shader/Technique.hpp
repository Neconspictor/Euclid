#pragma once

namespace nex
{
	class Camera;
	class Pass;


	template<class T>
	class BaseSelector
	{
	public:

		BaseSelector(T* selected) : mSelected(selected) {}

		virtual ~BaseSelector() = default;

		// BaseSelector should not be movable or copyable
		BaseSelector(const BaseSelector&) = delete;
		BaseSelector(BaseSelector&&) = delete;
		BaseSelector& operator=(BaseSelector&&) = delete;
		BaseSelector& operator=(const BaseSelector&) = delete;

		T* getSelected() const
		{
			return mSelected;
		}

		void setSelected(T* newSelected)
		{
			mSelected = newSelected;
		}

	protected:
		T* mSelected;
	};

	class Technique : public BaseSelector<Pass>
	{
	public:
		
		Technique(Pass* active) : BaseSelector(active) {}

		virtual ~Technique() = default;

		// Techniques should not be movable or copyable
		Technique(const Technique&) = delete;
		Technique(Technique&&) = delete;
		Technique& operator=(Technique&&) = delete;
		Technique& operator=(const Technique&) = delete;

		/**
		 *
		 */
		virtual void configureSubMeshPass(Camera* camera) = 0;

		/**
		 * provides the current active pass for rendering a submesh.
		 */
		Pass* getActiveSubMeshPass() const
		{
			return getSelected();
		}
	};

	class TechniqueSelector : public BaseSelector<Technique>
	{
	public:

		//using Selectables = std::vector<std::unique_ptr<Technique>>;

		TechniqueSelector(Technique* active) : BaseSelector(active) {}

		virtual ~TechniqueSelector() = default;

		// Techniques should not be movable or copyable
		TechniqueSelector(const TechniqueSelector&) = delete;
		TechniqueSelector(TechniqueSelector&&) = delete;
		TechniqueSelector& operator=(TechniqueSelector&&) = delete;
		TechniqueSelector& operator=(const TechniqueSelector&) = delete;

		/*void addSelectable(std::unique_ptr<Technique> selectable)
		{
			mSelectables.emplace_back(std::move(selectable));
		}

		const Selectables& getSelectables() const
		{
			return mSelectables;
		}*/

		/**
		 * provides the current active pass for rendering a submesh.
		 */
		Pass* getActiveSubMeshPass() const
		{
			if (getSelected() == nullptr) return nullptr;
			return getSelected()->getActiveSubMeshPass();
		}

	//protected:
	//	Selectables mSelectables;
	};
}