#pragma once

namespace nex
{
	class Texture;
	struct RenderState;

	class Blit {
	public:
		Blit();
		~Blit();
		void blitDepthStencilLuma(const Texture* color, const Texture* luminance, const Texture* deph, const Texture* stencil, const RenderState& state);
		void blit(const Texture* color, const RenderState& state);

	private:

		class BlitPass;

		std::unique_ptr<BlitPass> mBlitPass;
		std::unique_ptr<BlitPass> mBlitDepthStencilLumaPass;
	};
}