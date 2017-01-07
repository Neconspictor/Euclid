#include <antialiasing/opengl/SMAA_GL.hpp>

SMAA_GL::SMAA_GL(RendererOpenGL* renderer) : SMAA(), renderer(renderer)
{
}

SMAA_GL::~SMAA_GL()
{
	renderer->destroyRenderTarget(&edgesTex);
	renderer->destroyRenderTarget(&blendTex);
}

void SMAA_GL::antialiase(RenderTarget* renderTarget)
{
}

void SMAA_GL::init()
{
	Renderer3D::Viewport viewPort = renderer->getViewport();
	int& width = viewPort.width;
	int& height = viewPort.height;

	edgesTex = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
	blendTex = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
}

void SMAA_GL::reset()
{
	renderer->clearFrameBuffer(blendTex.frameBuffer, { 0,0,0,1 }, 1.0f, 0);
	renderer->clearFrameBuffer(edgesTex.frameBuffer, { 0,0,0,1 }, 1.0f, 0);
}

void SMAA_GL::updateBuffers()
{
	renderer->destroyRenderTarget(&edgesTex);
	renderer->destroyRenderTarget(&blendTex);

	Renderer3D::Viewport viewPort = renderer->getViewport();
	int& width = viewPort.width;
	int& height = viewPort.height;

	edgesTex = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
	edgesTex = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
}