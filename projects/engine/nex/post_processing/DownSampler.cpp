#include <nex/post_processing/DownSampler.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/material/Material.hpp>
#include "nex/drawing/StaticMeshDrawer.hpp"

class nex::DownSampler::DownSamplePass : public Pass
{
public:
	DownSamplePass()
	{
		mShader = Shader::create("post_processing/fullscreenPlane_vs.glsl", "post_processing/downsample_fs.glsl");
		mSourceUniform = { mShader->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D, 0};
		mShader->setBinding(mSourceUniform.location, mSourceUniform.bindingSlot);
	}

private:

	UniformTex mSourceUniform;
};


nex::DownSampler::DownSampler(unsigned width, unsigned height) : mDownSampleShader(std::make_unique<DownSamplePass>()),
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

nex::DownSampler::~DownSampler() = default;

nex::Texture2D* nex::DownSampler::downsampleHalfResolution(Texture2D* src)
{
	return downsample(src, mHalfResolution.get());
}

nex::Texture2D* nex::DownSampler::downsampleQuarterResolution(Texture2D* src)
{
	return downsample(src, mQuarterResolution.get());
	//return downsample(src, mQuarterResolution.get());
}

nex::Texture2D* nex::DownSampler::downsampleEigthResolution(Texture2D* src)
{
	return downsample(src, mEigthResolution.get());
	//return downsample(src, mEigthResolution.get());
}

nex::Texture2D* nex::DownSampler::downsampleSixteenthResolution(Texture2D* src)
{
	return downsample(src, mSixteenthResolution.get());
	//return downsample(src, mSixteenthResolution.get());
}

nex::Texture2D* nex::DownSampler::downsample(Texture2D* src, RenderTarget2D* dest)
{
	auto* renderBackend = RenderBackend::get();
	dest->bind();
	renderBackend->setViewPort(0, 0, dest->getWidth(), dest->getHeight());
	//dest->clear(Color);

	mDownSampleShader->bind();
	mDownSampleShader->getShader()->setTexture(src, mSampler.get(), 0);

	RenderState state = RenderState::createNoDepthTest();
	StaticMeshDrawer::drawFullscreenTriangle(state, mDownSampleShader.get());

	auto*  renderImage = static_cast<Texture2D*>(dest->getColorAttachmentTexture(0));
	//renderImage->generateMipMaps();

	return renderImage;
}

void nex::DownSampler::resize(unsigned width, unsigned height)
{
	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mHalfResolution = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);

	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mQuarterResolution = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);

	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mEigthResolution = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);

	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mSixteenthResolution = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);
}