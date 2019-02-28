#include<nex/post_processing/blur/GaussianBlur.hpp>

#include <nex/shader/post_processing/blur/GaussianBlurShader.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/texture/Texture.hpp>

namespace nex {


	GaussianBlur::GaussianBlur() :
		mHorizontalPass(std::make_unique< GaussianBlurHorizontalShader>()),
		mVerticalPass(std::make_unique< GaussianBlurVerticalShader>())
	{
		sprite.setPosition({ 0,0 });
		sprite.setHeight(1);
		sprite.setWidth(1);
	}

	void GaussianBlur::blur(RenderTarget2D* target, RenderTarget2D* cache)
	{

		//TODO do a blur pass
		cache->bind();
		cache->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);


		auto* renderResult = target->getColorAttachments()[0].texture.get();

		// horizontal pass
		sprite.setTexture(renderResult);
		mHorizontalPass->bind();
		mHorizontalPass->setTexture(sprite.getTexture());
		mHorizontalPass->setImageHeight((float)target->getHeight());
		mHorizontalPass->setImageWidth((float)target->getWidth());
		StaticMeshDrawer::draw(&sprite, mHorizontalPass.get());

		using r = RenderComponent;
		Dimension blitRegion = { 0,0, target->getWidth(), target->getHeight() };
		cache->blit(target, blitRegion, r::Color | r::Depth | r::Stencil);

		// vertical pass
		cache->bind();
		cache->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
		sprite.setTexture(renderResult);
		mVerticalPass->bind();
		mVerticalPass->setTexture(sprite.getTexture());
		mVerticalPass->setImageHeight((float)target->getHeight());
		mVerticalPass->setImageWidth((float)target->getWidth());
		StaticMeshDrawer::draw(&sprite, mVerticalPass.get());

		cache->blit(target, blitRegion, r::Color | r::Depth | r::Stencil);
	}
}