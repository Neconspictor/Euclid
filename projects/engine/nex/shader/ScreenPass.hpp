#pragma once
#include <nex/shader/Pass.hpp>

namespace nex
{
	class ScreenPass : public Pass
	{
	public:
		ScreenPass();

		void useTexture(const Texture* texture);
	protected:
		UniformTex mTexture;
	};
}