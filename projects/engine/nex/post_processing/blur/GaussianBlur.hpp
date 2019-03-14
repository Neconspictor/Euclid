#pragma once
#include <nex/texture/Sprite.hpp>
#include "nex/shader/post_processing/blur/GaussianBlurShader.hpp"

namespace nex {
	class RenderTarget2D;

	class GaussianBlur {

	public:
		GaussianBlur();

		Texture2D* blur(Texture2D* texture, RenderTarget2D* out, RenderTarget2D* cache);

	protected:
		nex::Sprite sprite;

		std::unique_ptr<GaussianBlurHorizontalShader> mHorizontalPass;
		std::unique_ptr<GaussianBlurVerticalShader> mVerticalPass;
	};
}
