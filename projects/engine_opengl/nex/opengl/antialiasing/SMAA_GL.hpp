#pragma once
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>


class SMAA_GL
{
public:
	explicit SMAA_GL(RendererOpenGL* renderer);
	virtual ~SMAA_GL();

	void antialias(RenderTargetGL* renderTarget);

	void init();

	void reset();

	void updateBuffers();

private:
	RendererOpenGL* renderer;
	RenderTargetGL* edgesTex;
	RenderTargetGL* blendTex;

	TextureGL* areaTex;
	TextureGL* searchTex;

	GLuint edgeDetectionPass;
	GLuint blendingWeightCalculationPass;
	GLuint neighborhoodBlendingPass;

	bool initialized;
};