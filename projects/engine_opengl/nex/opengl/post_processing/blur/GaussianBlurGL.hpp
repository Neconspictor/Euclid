#pragma once
#include <nex/texture/Sprite.hpp>

namespace nex {

	class RenderTarget2D;
	class RendererOpenGL;

	class GaussianBlurGL {

	public:
		GaussianBlurGL(RendererOpenGL* renderer);

		virtual ~GaussianBlurGL();

		void blur(RenderTarget2D* target, RenderTarget2D* cache);

		void init();

		void release();

	protected:
		RendererOpenGL* renderer;
		nex::Sprite sprite;
	};
}