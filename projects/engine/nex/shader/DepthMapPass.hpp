#pragma once
#include <nex/shader/Pass.hpp>
#include <nex/texture/Sampler.hpp>

namespace nex
{
	class DepthMapPass : public Pass
	{
	public:
		DepthMapPass();

		virtual ~DepthMapPass() = default;

		void useDepthMapTexture(const Texture* texture);
	private:
		UniformTex mDephTexture;
		Sampler mSampler;
	};
}
