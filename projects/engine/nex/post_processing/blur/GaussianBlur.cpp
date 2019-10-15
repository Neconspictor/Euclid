#include<nex/post_processing/blur/GaussianBlur.hpp>

#include <nex/post_processing/blur/GaussianBlurPass.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/texture/Sampler.hpp>
#include "nex/texture/Sprite.hpp"
#include <nex/material/Material.hpp>

namespace nex {


	GaussianBlur::GaussianBlur(unsigned width, unsigned height) :
		mHorizontalPass(std::make_unique< GaussianBlurHorizontalShader>()),
		mVerticalPass(std::make_unique< GaussianBlurVerticalShader>()),
		mSampler(std::make_unique<Sampler>(SamplerDesc()))
	{
		mSampler->setAnisotropy(1.0f);
		mSampler->setMinFilter(TextureFilter::Linear);
		mSampler->setMagFilter(TextureFilter::Linear);
		mSampler->setWrapR(TextureUVTechnique::ClampToEdge);
		mSampler->setWrapS(TextureUVTechnique::ClampToEdge);
		mSampler->setWrapT(TextureUVTechnique::ClampToEdge);
		resize(width, height);
	}

	GaussianBlur::~GaussianBlur() = default;

	void GaussianBlur::resize(unsigned width, unsigned height)
	{
		mHalfBlur = std::make_unique<RenderTarget2D>(width / 2, height / 2, TextureDesc::createRenderTargetRGBAHDR());
		mQuarterBlur = std::make_unique<RenderTarget2D>(width / 4, height / 4, TextureDesc::createRenderTargetRGBAHDR());
		mEigthBlur = std::make_unique<RenderTarget2D>(width / 8, height / 8, TextureDesc::createRenderTargetRGBAHDR());
		mSixteenthBlur = std::make_unique<RenderTarget2D>(width / 16, height / 16, TextureDesc::createRenderTargetRGBAHDR());
	}

	Texture2D* GaussianBlur::blur(Texture2D* texture, RenderTarget* out, RenderTarget* cache)
	{
		const auto& state = RenderState::getNoDepthTest();

		mSampler->bind(0);
		cache->bind();
		RenderBackend::get()->setViewPort(0, 0, texture->getWidth(), texture->getHeight());
		//cache->clear(Color | Depth | Stencil);

		// horizontal pass
		mHorizontalPass->bind();
		mHorizontalPass->setTexture(texture);
		mHorizontalPass->setImageHeight((float)texture->getHeight());
		mHorizontalPass->setImageWidth((float)texture->getWidth());
		StaticMeshDrawer::drawFullscreenTriangle(state, mHorizontalPass.get());

		// vertical pass
		out->bind();
		RenderBackend::get()->setViewPort(0, 0, texture->getWidth(), texture->getHeight());
		//out->clear(Color | Depth | Stencil);
		mVerticalPass->bind();
		mVerticalPass->setTexture(cache->getColorAttachmentTexture(0));
		mVerticalPass->setImageHeight((float)texture->getHeight());
		mVerticalPass->setImageWidth((float)texture->getWidth());
		StaticMeshDrawer::drawFullscreenTriangle(state, mVerticalPass.get());

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
