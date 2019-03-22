#include<nex/post_processing/blur/GaussianBlur.hpp>

#include <nex/shader/post_processing/blur/GaussianBlurShader.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/texture/Sampler.hpp>
#include "nex/texture/Sprite.hpp"

namespace nex {


	GaussianBlur::GaussianBlur(unsigned width, unsigned height) :
		mHorizontalPass(std::make_unique< GaussianBlurHorizontalShader>()),
		mVerticalPass(std::make_unique< GaussianBlurVerticalShader>()),
		mSampler(std::make_unique<Sampler>(SamplerDesc()))
	{
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
		mHorizontalPass->bind();
		mHorizontalPass->setTexture(texture);
		mHorizontalPass->setImageHeight((float)cache->getHeight());
		mHorizontalPass->setImageWidth((float)cache->getWidth());
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), mHorizontalPass.get());

		// vertical pass
		out->bind();
		RenderBackend::get()->setViewPort(0, 0, out->getWidth(), out->getHeight());
		mVerticalPass->bind();
		mVerticalPass->setTexture(cache->getColorAttachmentTexture(0));
		mVerticalPass->setImageHeight((float)out->getHeight());
		mVerticalPass->setImageWidth((float)out->getWidth());
		StaticMeshDrawer::draw(Sprite::getScreenSprite(), mVerticalPass.get());

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
