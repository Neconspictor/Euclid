#include <nex/post_processing/DownSampler.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Shader.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sampler.hpp>
#include <nex/material/Material.hpp>
#include "nex/drawing/StaticMeshDrawer.hpp"
#include <nex/texture/TextureManager.hpp>

class nex::DownSampler::DownSamplePass : public Shader
{
public:
	DownSamplePass()
	{
		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/downsample_fs.glsl");
		mSourceUniform = { mProgram->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D, 0};
		mProgram->setBinding(mSourceUniform.location, mSourceUniform.bindingSlot);
	}

private:

	UniformTex mSourceUniform;
};

class nex::DownSampler::DepthDownSamplePass : public Shader
{
public:
	DepthDownSamplePass()
	{
		mProgram = ShaderProgram::create("screen_space_vs.glsl", "post_processing/downsample_depth_fs.glsl");
		mSourceUniform = mProgram->createTextureUniform("sourceTexture", UniformType::TEXTURE2D, 0);

	}

	void setSource(Texture* texture) {
		mProgram->setTexture(texture, Sampler::getPoint(), mSourceUniform.bindingSlot);
	}

private:

	UniformTex mSourceUniform;
};


nex::DownSampler::DownSampler(unsigned width, unsigned height) : mDownSampleShader(std::make_unique<DownSamplePass>()),
mDepthDownSampleShader(std::make_unique<DepthDownSamplePass>())
{
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

nex::Texture2D* nex::DownSampler::downsampleEighthResolution(Texture2D* src)
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
	mDownSampleShader->getShader()->setTexture(src, Sampler::getLinear(), 0);

	const auto& state = RenderState::getNoDepthTest();
	StaticMeshDrawer::drawFullscreenTriangle(state, mDownSampleShader.get());

	auto*  renderImage = static_cast<Texture2D*>(dest->getColorAttachmentTexture(0));
	//renderImage->generateMipMaps();

	return renderImage;
}

nex::Texture* nex::DownSampler::downsampleDepthHalf(Texture2D* src, RenderTarget* dest)
{
	auto* renderBackend = RenderBackend::get();
	dest->bind();
	renderBackend->setViewPort(0, 0, src->getWidth() / 2, src->getHeight() / 2);
	//dest->clear(Color);

	mDepthDownSampleShader->bind();
	mDepthDownSampleShader->setSource(src);

	const auto& state = RenderState::getNoDepthTest();
	StaticMeshDrawer::drawFullscreenTriangle(state, mDepthDownSampleShader.get());

	auto* renderImage = dest->getColorAttachmentTexture(0);

	return renderImage;
}

void nex::DownSampler::resize(unsigned width, unsigned height)
{
	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mHalfResolution = std::make_unique<RenderTarget2D>(width, height, TextureDesc::createRenderTargetRGBAHDR(), 1);

	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mQuarterResolution = std::make_unique<RenderTarget2D>(width, height, TextureDesc::createRenderTargetRGBAHDR(), 1);

	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mEigthResolution = std::make_unique<RenderTarget2D>(width, height, TextureDesc::createRenderTargetRGBAHDR(), 1);

	width = static_cast<unsigned>(width * 0.5);
	height = static_cast<unsigned>(height * 0.5);
	mSixteenthResolution = std::make_unique<RenderTarget2D>(width, height, TextureDesc::createRenderTargetRGBAHDR(), 1);
}