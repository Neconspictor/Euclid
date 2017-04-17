#include<post_processing/blur/opengl/GaussianBlurGL.hpp>

#include <texture/opengl/TextureGL.hpp>
#include <renderer/opengl/RendererOpenGL.hpp>

GaussianBlurGL::GaussianBlurGL(RendererOpenGL* renderer) : GaussianBlur(), renderer(renderer)
{
}

GaussianBlurGL::~GaussianBlurGL()
{
}

void GaussianBlurGL::blur(RenderTarget* target)
{
	RenderTargetGL* glTarget = dynamic_cast<RenderTargetGL*>(target);
	assert(glTarget != nullptr);
	GLuint texture = glTarget->getTextureGL();

	//TODO do a blur pass

	// At the end blit the rendered image to the render target 'target'
	renderer->blitRenderTargets(&tempTarget, glTarget);
}

void GaussianBlurGL::init()
{
	Renderer3D::Viewport viewPort = renderer->getViewport();
	int& width = viewPort.width;
	int& height = viewPort.height;

	renderer->destroyRenderTarget(&tempTarget);
	tempTarget = renderer->createRenderTarget(GL_RGBA8, width, height, 1, GL_NONE);
}