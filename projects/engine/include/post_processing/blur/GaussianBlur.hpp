#pragma once

class RenderTarget;

class GaussianBlur {
public:

	GaussianBlur();

	virtual ~GaussianBlur();

	virtual void blur(RenderTarget* target, RenderTarget* cache) = 0;

	virtual void init() = 0;
};