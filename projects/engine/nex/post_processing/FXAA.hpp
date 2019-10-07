#pragma once

#include <memory>

namespace nex
{
	class Texture;

	class FXAA
	{
	public:
		FXAA();
		~FXAA();

		/**
		 * Renders an antialiased version of the specified source texture into the currently bound rendertarget (color attachment 0).
		 */
		void antialias(Texture* source, bool sourceIsInGammaSpace);

	private:

		class FxaaPass;

		std::unique_ptr<FxaaPass> mFxaaPassGamma;
		std::unique_ptr<FxaaPass> mFxaaPassLinear;
	};
}