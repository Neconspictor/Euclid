#pragma once

namespace nex
{
	class Camera;
	class Pass;
	class TransformPass;


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

	class Technique : public BaseSelector<TransformPass>
	{
	public:
		
		Technique(TransformPass* active) : BaseSelector(active) {}

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
		TransformPass* getActiveSubMeshPass() const
		{
			return getSelected();
		}
	};
}