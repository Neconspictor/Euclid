#include<nex/post_processing/blur/GaussianBlur.hpp>

#include <nex/shader/post_processing/blur/GaussianBlurShader.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>

namespace nex {


	GaussianBlur::GaussianBlur() :
		mHorizontalPass(std::make_unique< GaussianBlurHorizontalShader>()),
		mVerticalPass(std::make_unique< GaussianBlurVerticalShader>())
	{
		sprite.setPosition({ 0,0 });
		sprite.setHeight(1);
		sprite.setWidth(1);
	}

	Texture2D* GaussianBlur::blur(Texture2D* texture, RenderTarget2D* out, RenderTarget2D* cache)
	{
		cache->bind();

		// horizontal pass
		sprite.setTexture(texture);
		mHorizontalPass->bind();
		mHorizontalPass->setTexture(sprite.getTexture());
		mHorizontalPass->setImageHeight((float)cache->getHeight());
		mHorizontalPass->setImageWidth((float)cache->getWidth());
		StaticMeshDrawer::draw(&sprite, mHorizontalPass.get());

		// vertical pass
		out->bind();

		sprite.setTexture(cache->getColorAttachmentTexture(0));
		mVerticalPass->bind();
		mVerticalPass->setTexture(sprite.getTexture());
		mVerticalPass->setImageHeight((float)out->getHeight());
		mVerticalPass->setImageWidth((float)out->getWidth());
		StaticMeshDrawer::draw(&sprite, mVerticalPass.get());

		return static_cast<Texture2D*>(out->getColorAttachmentTexture(0));
	}
}