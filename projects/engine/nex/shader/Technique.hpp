#pragma once

namespace nex
{
	class Pass;

	template<class T>
	class Selector
	{
	public:

		T* getActive() const
		{
			return mActive;
		}

	protected:
		T* mActive = nullptr;
	};


	class Technique : public Selector<Pass>
	{
	public:
		virtual ~Technique() = default;

		Pass* getActivePass() const
		{
			return mActive;
		}
	};

	/*class TechniqueSelector : public Selector<Technique>
	{
	public:
		virtual ~TechniqueSelector() = default;

		Technique* getActiveTechnique() const
		{
			return mActive;
		}

		Pass* getActivePass() const
		{
			return mActive->getActivePass();
		}
	};*/
}