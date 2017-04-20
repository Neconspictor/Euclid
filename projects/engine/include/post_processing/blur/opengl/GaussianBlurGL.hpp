#pragma once

#include <post_processing/blur/GaussianBlur.hpp>
#include <sprite/Sprite.hpp>

class RendererOpenGL;
class RenderTargetGL;

class GaussianBlurGL : public GaussianBlur {

public:
	GaussianBlurGL(RendererOpenGL* renderer);

	virtual ~GaussianBlurGL();

	virtual void blur(RenderTarget* target, RenderTarget* cache) override;

	void init();

	void release();

protected:
	RendererOpenGL* renderer;
	Sprite sprite;
};