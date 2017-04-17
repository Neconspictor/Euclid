#pragma once

#include <post_processing/blur/GaussianBlur.hpp>

class GaussianBlurGL : public GaussianBlur {

public:
	GaussianBlurGL(RendererOpenGL* renderer);

	virtual ~GaussianBlurGL();

	void blur(RenderTarget* target);

	void init();

protected:
	RendererOpenGL* renderer;
	RenderTargetGL tempTarget;
};