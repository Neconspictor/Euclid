#pragma once

#include <nex/texture/Texture.hpp>

namespace nex
{
	class RenderBackend;
	class RenderTarget;

	class SMAA
	{
	public:
		explicit SMAA(RenderBackend* renderer);
		virtual ~SMAA();

		void antialias(RenderTarget* renderTarget);

		void init();

		void reset();

		void updateBuffers();

	private:
		RenderBackend* renderer;
		RenderTarget* edgesTex;
		RenderTarget* blendTex;

		Texture* areaTex;
		Texture* searchTex;

		unsigned edgeDetectionPass;
		unsigned blendingWeightCalculationPass;
		unsigned neighborhoodBlendingPass;

		bool initialized;
	};
}