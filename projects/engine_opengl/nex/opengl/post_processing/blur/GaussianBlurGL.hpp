#pragma once
#include <nex/opengl/texture/Sprite.hpp>

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
	Sprite sprite;
};