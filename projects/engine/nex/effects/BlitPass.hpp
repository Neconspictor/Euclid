#pragma once

#include <nex/shader/Pass.hpp>

namespace nex
{
	class BlitPass : public Pass
	{
	public:
		BlitPass();

		void setInput(Texture* texture);

	private:
		UniformTex mInput;
	};
}