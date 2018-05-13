#include<post_processing/blur/GaussianBlurGL.hpp>

#include <texture/TextureGL.hpp>
#include <renderer/RendererOpenGL.hpp>
#include <shader/post_processing/blur/GaussianBlurShader.hpp>

GaussianBlurGL::GaussianBlurGL(RendererOpenGL* renderer) : GaussianBlur(), renderer(renderer)
{
	sprite.setPosition({ 0,0 });
	sprite.setHeight(1);
	sprite.setWidth(1);
}

GaussianBlurGL::~GaussianBlurGL()
{
}

void GaussianBlurGL::blur(RenderTarget* target, RenderTarget* cache)
{
	RenderTargetGL* glTarget = dynamic_cast<RenderTargetGL*>(target);
	assert(glTarget != nullptr);
	GLuint texture = glTarget->getTextureGL();
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	GaussianBlurHorizontalShader* horizontalShader = dynamic_cast<GaussianBlurHorizontalShader*>(
		renderer->getShaderManager()->getConfig(Shaders::GaussianBlurHorizontal));
	GaussianBlurVerticalShader* verticalShader = dynamic_cast<GaussianBlurVerticalShader*>(
		renderer->getShaderManager()->getConfig(Shaders::GaussianBlurVertical));


	//TODO do a blur pass

	renderer->useRenderTarget(cache);
	renderer->beginScene();


	// horizontal pass
	sprite.setTexture(glTarget->getTexture());
	horizontalShader->setTexture(sprite.getTexture());
	horizontalShader->setImageHeight(glTarget->getHeight());
	horizontalShader->setImageWidth(glTarget->getWidth());
	modelDrawer->draw(&sprite, Shaders::GaussianBlurHorizontal);
	renderer->blitRenderTargets(cache, glTarget);

	// vertical pass
	renderer->useRenderTarget(cache);
	renderer->beginScene();
	sprite.setTexture(glTarget->getTexture());
	verticalShader->setTexture(sprite.getTexture());
	verticalShader->setImageHeight(glTarget->getHeight());
	verticalShader->setImageWidth(glTarget->getWidth());
	modelDrawer->draw(&sprite, Shaders::GaussianBlurVertical);
	renderer->blitRenderTargets(cache, glTarget);
}

void GaussianBlurGL::init()
{
	/*if (tempTarget)
		renderer->destroyRenderTarget(tempTarget);

	Renderer3D::Viewport viewPort = renderer->getViewport();
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
