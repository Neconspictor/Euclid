#pragma once
#include <renderer/opengl/RendererOpenGL.hpp>
#include <texture/opengl/TextureGL.hpp>


class SMAA_GL : public SMAA
{
public:
	explicit SMAA_GL(RendererOpenGL* renderer);
	virtual ~SMAA_GL();

	virtual void antialias(RenderTarget* renderTarget) override;

	virtual void init() override;

	void reset() override;

	virtual void updateBuffers() override;

private:
	RendererOpenGL* renderer;
	RendererOpenGL::RenderTargetGL edgesTex;
	RendererOpenGL::RenderTargetGL blendTex;

	TextureGL* areaTex;
	TextureGL* searchTex;

	GLuint edgeDetectionPass;
	GLuint blendingWeightCalculationPass;
	GLuint neighborhoodBlendingPass;
};