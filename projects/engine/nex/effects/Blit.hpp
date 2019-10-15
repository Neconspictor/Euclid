#pragma once

namespace nex
{
	class Texture;

	class Blit {
	public:
		Blit();
		~Blit();
		void blitStencil(Texture* color, Texture* stencil);
		void blit(Texture* color);

	private:

		class BlitPass;

		std::unique_ptr<BlitPass> mBlitPass;
		std::unique_ptr<BlitPass> mBlitStencilPass;
	};
}