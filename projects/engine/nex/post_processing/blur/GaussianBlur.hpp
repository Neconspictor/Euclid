#pragma once
#include <nex/texture/Sprite.hpp>

namespace nex {

	class RenderTarget2D;

	class GaussianBlur {

	public:
		GaussianBlur();

		void blur(RenderTarget2D* target, RenderTarget2D* cache);

	protected:
		nex::Sprite sprite;
	};
}