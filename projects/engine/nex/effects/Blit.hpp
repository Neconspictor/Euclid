#pragma once

namespace nex
{
	class Texture;
	struct RenderState;

	class Blit {
	public:
		Blit();
		~Blit();
		void blitStencil(Texture* color, Texture* luminance, Texture* deph, Texture* stencil, const RenderState& state);
		void blit(Texture* color, Texture* deph, const RenderState& state);

	private:

		class BlitPass;

		std::unique_ptr<BlitPass> mBlitPass;
		std::unique_ptr<BlitPass> mBlitStencilPass;
	};
}