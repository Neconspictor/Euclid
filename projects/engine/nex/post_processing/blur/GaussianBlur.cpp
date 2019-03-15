#include<nex/post_processing/blur/GaussianBlur.hpp>

#include <nex/shader/post_processing/blur/GaussianBlurShader.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/texture/Sampler.hpp>

namespace nex {


	GaussianBlur::GaussianBlur(unsigned width, unsigned height) :
		mHorizontalPass(std::make_unique< GaussianBlurHorizontalShader>()),
		mVerticalPass(std::make_unique< GaussianBlurVerticalShader>()),
		mSampler(std::make_unique<Sampler>(SamplerDesc()))
	{
		sprite.setPosition({ 0,0 });
		sprite.setHeight(1);
		sprite.setWidth(1);

		mSampler->setAnisotropy(0.0f);
		mSampler->setMinFilter(TextureFilter::Linear);
		mSampler->setMagFilter(TextureFilter::Linear);
		mSampler->setWrapR(TextureUVTechnique::ClampToEdge);
		mSampler->setWrapS(TextureUVTechnique::ClampToEdge);
		mSampler->setWrapT(TextureUVTechnique::ClampToEdge);

		resize(width, height);
	}

	void GaussianBlur::resize(unsigned width, unsigned height)
	{
		mHalfBlur = std::make_unique<RenderTarget2D>(width / 2, height / 2, TextureData::createRenderTargetRGBAHDR());
		mQuarterBlur = std::make_unique<RenderTarget2D>(width / 4, height / 4, TextureData::createRenderTargetRGBAHDR());
		mEigthBlur = std::make_unique<RenderTarget2D>(width / 8, height / 8, TextureData::createRenderTargetRGBAHDR());
		mSixteenthBlur = std::make_unique<RenderTarget2D>(width / 16, height / 16, TextureData::createRenderTargetRGBAHDR());
	}

	Texture2D* GaussianBlur::blur(Texture2D* texture, RenderTarget2D* out, RenderTarget2D* cache)
	{
		mSampler->bind(0);
		cache->bind();
		RenderBackend::get()->setViewPort(0, 0, cache->getWidth(), cache->getHeight());

		// horizontal pass
		sprite.setTexture(texture);
		mHorizontalPass->bind();
		mHorizontalPass->setTexture(sprite.getTexture());
		mHorizontalPass->setImageHeight((float)cache->getHeight());
		mHorizontalPass->setImageWidth((float)cache->getWidth());
		StaticMeshDrawer::draw(&sprite, mHorizontalPass.get());

		// vertical pass
		out->bind();
		RenderBackend::get()->setViewPort(0, 0, out->getWidth(), out->getHeight());
		sprite.setTexture(cache->getColorAttachmentTexture(0));
		mVerticalPass->bind();
		mVerticalPass->setTexture(sprite.getTexture());
		mVerticalPass->setImageHeight((float)out->getHeight());
		mVerticalPass->setImageWidth((float)out->getWidth());
		StaticMeshDrawer::draw(&sprite, mVerticalPass.get());

		mSampler->unbind(0);

		return static_cast<Texture2D*>(out->getColorAttachmentTexture(0));
	}

	Texture2D* GaussianBlur::blurHalfResolution(Texture2D* texture, RenderTarget2D* out)
	{
		return blur(texture, out, mHalfBlur.get());
	}

	Texture2D* GaussianBlur::blurQuarterResolution(Texture2D* texture, RenderTarget2D* out)
	{
		return blur(texture, out, mQuarterBlur.get());
	}

	Texture2D* GaussianBlur::blurEigthResolution(Texture2D* texture, RenderTarget2D* out)
	{
		return blur(texture, out, mEigthBlur.get());
	}

	Texture2D* GaussianBlur::blurSixteenthResolution(Texture2D* texture, RenderTarget2D* out)
	{
		//return blur(blurEigthResolution(texture, out), out, mQuarterBlur.get());
		return blur(texture, out, mSixteenthBlur.get());
	}
}