#include<nex/opengl/post_processing/blur/GaussianBlurGL.hpp>

#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/opengl/shader/post_processing/blur/GaussianBlurShaderGL.hpp>

namespace nex {


	GaussianBlurGL::GaussianBlurGL(RenderBackend* renderer) : renderer(renderer)
	{
		sprite.setPosition({ 0,0 });
		sprite.setHeight(1);
		sprite.setWidth(1);
	}

	GaussianBlurGL::~GaussianBlurGL()
	{
	}

	void GaussianBlurGL::blur(RenderTarget2D* target, RenderTarget2D* cache)
	{
		StaticMeshDrawer* modelDrawer = renderer->getModelDrawer();
		GaussianBlurHorizontalShader* horizontalShader = dynamic_cast<GaussianBlurHorizontalShader*>(
			renderer->getShaderManager()->getShader(ShaderType::GaussianBlurHorizontal));
		GaussianBlurVerticalShader* verticalShader = dynamic_cast<GaussianBlurVerticalShader*>(
			renderer->getShaderManager()->getShader(ShaderType::GaussianBlurVertical));


		//TODO do a blur pass
		cache->bind();
		renderer->beginScene();


		// horizontal pass
		sprite.setTexture(target->getRenderResult());
		horizontalShader->bind();
		horizontalShader->setTexture(sprite.getTexture());
		horizontalShader->setImageHeight((float)target->getHeight());
		horizontalShader->setImageWidth((float)target->getWidth());
		modelDrawer->draw(&sprite, horizontalShader);

		using r = RenderComponent;
		Dimension blitRegion = { 0,0, target->getWidth(), target->getHeight() };
		cache->blit(target, blitRegion, r::Color | r::Depth | r::Stencil);

		// vertical pass
		cache->bind();
		renderer->beginScene();
		sprite.setTexture(target->getRenderResult());
		verticalShader->bind();
		verticalShader->setTexture(sprite.getTexture());
		verticalShader->setImageHeight((float)target->getHeight());
		verticalShader->setImageWidth((float)target->getWidth());
		modelDrawer->draw(&sprite, verticalShader);

		cache->blit(target, blitRegion, r::Color | r::Depth | r::Stencil);
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
}
