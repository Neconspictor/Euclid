#pragma once

struct RenderTarget;

class SMAA
{
public:

	SMAA() {}

	virtual ~SMAA() {}

	virtual void antialias(RenderTarget* renderTarget) = 0;

	virtual void init() = 0;

	virtual void reset() = 0;

	virtual void updateBuffers() = 0;
};