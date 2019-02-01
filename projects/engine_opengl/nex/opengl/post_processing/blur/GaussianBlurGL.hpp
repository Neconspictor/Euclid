#pragma once
#include <nex/texture/Sprite.hpp>

namespace nex {

	class RenderTarget2D;
	class RenderBackend;

	class GaussianBlurGL {

	public:
		GaussianBlurGL(RenderBackend* renderer);

		virtual ~GaussianBlurGL();

		void blur(RenderTarget2D* target, RenderTarget2D* cache);

		void init();

		void release();

	protected:
		RenderBackend* renderer;
		nex::Sprite sprite;
	};
}