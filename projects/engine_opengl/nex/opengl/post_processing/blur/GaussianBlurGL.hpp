#pragma once
#include <nex/texture/Sprite.hpp>

namespace nex {

	class RendererOpenGL;
	class RenderTargetGL;

	class GaussianBlurGL {

	public:
		GaussianBlurGL(RendererOpenGL* renderer);

		virtual ~GaussianBlurGL();

		void blur(RenderTargetGL* target, RenderTargetGL* cache);

		void init();

		void release();

	protected:
		RendererOpenGL* renderer;
		nex::Sprite sprite;
	};
}