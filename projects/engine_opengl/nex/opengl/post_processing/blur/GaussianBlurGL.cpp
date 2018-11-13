#include<nex/opengl/post_processing/blur/GaussianBlurGL.hpp>

#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/opengl/shader/post_processing/blur/GaussianBlurShaderGL.hpp>

GaussianBlurGL::GaussianBlurGL(RendererOpenGL* renderer) : renderer(renderer)
{
	sprite.setPosition({ 0,0 });
	sprite.setHeight(1);
	sprite.setWidth(1);
}

GaussianBlurGL::~GaussianBlurGL()
{
}

void GaussianBlurGL::blur(RenderTargetGL* target, RenderTargetGL* cache)
{
	RenderTargetGL* glTarget = dynamic_cast<RenderTargetGL*>(target);
	assert(glTarget != nullptr);
	GLuint texture = glTarget->getTextureGL();
	ModelDrawerGL* modelDrawer = renderer->getModelDrawer();
	GaussianBlurHorizontalShaderGL* horizontalShader = dynamic_cast<GaussianBlurHorizontalShaderGL*>(
		renderer->getShaderManager()->getConfig(Shaders::GaussianBlurHorizontal));
	GaussianBlurVerticalShaderGL* verticalShader = dynamic_cast<GaussianBlurVerticalShaderGL*>(
		renderer->getShaderManager()->getConfig(Shaders::GaussianBlurVertical));


	//TODO do a blur pass

	renderer->useBaseRenderTarget(cache);
	renderer->beginScene();


	// horizontal pass
	sprite.setTexture(glTarget->getTexture());
	horizontalShader->setTexture(sprite.getTexture());
	horizontalShader->setImageHeight((float)glTarget->getHeight());
	horizontalShader->setImageWidth((float)glTarget->getWidth());
	modelDrawer->draw(&sprite, Shaders::GaussianBlurHorizontal);

	using r = RenderComponent;
	Dimension blitRegion = { 0,0, glTarget->getWidth(), glTarget->getHeight() };
	renderer->blitRenderTargets(cache, glTarget, blitRegion, r::Color | r::Depth | r::Stencil);

	// vertical pass
	renderer->useBaseRenderTarget(cache);
	renderer->beginScene();
	sprite.setTexture(glTarget->getTexture());
	verticalShader->setTexture(sprite.getTexture());
	verticalShader->setImageHeight((float)glTarget->getHeight());
	verticalShader->setImageWidth((float)glTarget->getWidth());
	modelDrawer->draw(&sprite, Shaders::GaussianBlurVertical);


	renderer->blitRenderTargets(cache, glTarget, blitRegion, r::Color | r::Depth | r::Stencil);
}

void GaussianBlurGL::init()
{
	/*if (tempTarget)
		renderer->destroyRenderTarget(tempTarget);

	RenderBackend::Viewport viewPort = renderer->getViewport();
	int& width = viewPort.width;
	int& height = viewPort.height;
	tempTarget = renderer->createRenderTargetGL(GL_RGBA8, width, height, 1, GL_DEPTH_STENCIL);
	//tempTarget = reinterpret_cast<RenderTargetGL*>(renderer->createRenderTarget(1));*/
}

void GaussianBlurGL::release()
{
	/*if (tempTarget)
		renderer->destroyRenderTarget(tempTarget);
	tempTarget = nullptr;*/
}
