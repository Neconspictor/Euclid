#pragma once

namespace nex
{
	class Pass;

	class Technique
	{
	public:
		virtual ~Technique() = default;
		Technique(const Technique&) = default;
		Technique(Technique&&) = default;
		Technique& operator=(Technique&&) = default;
		Technique& operator=(const Technique&) = default;

		/**
		 * provides the current active pass for rendering a submesh.
		 */
		Pass* getActiveSubMeshPass() const
		{
			return mActive;
		}

	protected:
		Pass* mActive = nullptr;
		
	};
}