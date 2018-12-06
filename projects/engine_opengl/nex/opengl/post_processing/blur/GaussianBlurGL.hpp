#pragma once
#include <nex/texture/Sprite.hpp>

namespace nex {

	class RenderTarget;
	class RendererOpenGL;

	class GaussianBlurGL {

	public:
		GaussianBlurGL(RendererOpenGL* renderer);

		virtual ~GaussianBlurGL();

		void blur(RenderTarget* target, RenderTarget* cache);

		void init();

		void release();

	protected:
		RendererOpenGL* renderer;
		nex::Sprite sprite;
	};
}