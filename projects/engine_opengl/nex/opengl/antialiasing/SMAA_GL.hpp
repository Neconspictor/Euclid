#pragma once
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/antialiasing/SMAA.hpp>


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
	RenderTargetGL* edgesTex;
	RenderTargetGL* blendTex;

	TextureGL* areaTex;
	TextureGL* searchTex;

	GLuint edgeDetectionPass;
	GLuint blendingWeightCalculationPass;
	GLuint neighborhoodBlendingPass;

	bool initialized;
};