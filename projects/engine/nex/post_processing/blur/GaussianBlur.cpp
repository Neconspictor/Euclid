#include<nex/post_processing/blur/GaussianBlur.hpp>

#include <nex/shader/ShaderManager.hpp>
#include <nex/shader/post_processing/blur/GaussianBlurShader.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/texture/Texture.hpp>

namespace nex {


	GaussianBlur::GaussianBlur()
	{
		sprite.setPosition({ 0,0 });
		sprite.setHeight(1);
		sprite.setWidth(1);
	}

	void GaussianBlur::blur(RenderTarget2D* target, RenderTarget2D* cache)
	{
		GaussianBlurHorizontalShader* horizontalShader = dynamic_cast<GaussianBlurHorizontalShader*>(
			ShaderManager::get()->getShader(ShaderType::GaussianBlurHorizontal));
		GaussianBlurVerticalShader* verticalShader = dynamic_cast<GaussianBlurVerticalShader*>(
			ShaderManager::get()->getShader(ShaderType::GaussianBlurVertical));


		//TODO do a blur pass
		cache->bind();
		cache->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);


		auto* renderResult = target->getColorAttachments()[0].texture.get();

		// horizontal pass
		sprite.setTexture(renderResult);
		horizontalShader->bind();
		horizontalShader->setTexture(sprite.getTexture());
		horizontalShader->setImageHeight((float)target->getHeight());
		horizontalShader->setImageWidth((float)target->getWidth());
		StaticMeshDrawer::draw(&sprite, horizontalShader);

		using r = RenderComponent;
		Dimension blitRegion = { 0,0, target->getWidth(), target->getHeight() };
		cache->blit(target, blitRegion, r::Color | r::Depth | r::Stencil);

		// vertical pass
		cache->bind();
		cache->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
		sprite.setTexture(renderResult);
		verticalShader->bind();
		verticalShader->setTexture(sprite.getTexture());
		verticalShader->setImageHeight((float)target->getHeight());
		verticalShader->setImageWidth((float)target->getWidth());
		StaticMeshDrawer::draw(&sprite, verticalShader);

		cache->blit(target, blitRegion, r::Color | r::Depth | r::Stencil);
	}
}