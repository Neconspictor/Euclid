#pragma once

namespace nex
{
	class Texture;
	struct RenderState;
	class RenderTarget;

	class Blit {
	public:
		Blit();
		~Blit();
		void blitColor0Color1DepthUseStencilTest(const RenderTarget& source, const Texture* sourceStencil, const RenderTarget& dest, const RenderState& state);
		void blit(const Texture* color, const RenderState& state);

	private:

		class BlitPass;

		std::unique_ptr<BlitPass> mBlitPass;
		std::unique_ptr<BlitPass> mBlitDepthStencilLumaPass;
		std::unique_ptr<RenderTarget> mRenderTarget;
	};
}