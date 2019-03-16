#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/RenderBackend.hpp"
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Shader.hpp>
#include "nex/texture/TextureManager.hpp"
#include "nex/texture/Sampler.hpp"
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/post_processing/DownSampler.hpp>


class nex::PostProcessor::PostProcessShader : public nex::Shader
{
public:
	PostProcessShader() : mSampler({})
	{
		mProgram = nex::ShaderProgram::create("fullscreenPlane_vs.glsl", "post_processing/postProcess_fs.glsl");
		sourceTextureUniform = { mProgram->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D, 0 };
		bloomHalfth = { mProgram->getUniformLocation("bloomHalfth"), UniformType::TEXTURE2D, 1 };
		bloomQuarter = { mProgram->getUniformLocation("bloomQuarter"), UniformType::TEXTURE2D, 2 };
		bloomEigth = { mProgram->getUniformLocation("bloomEigth"), UniformType::TEXTURE2D, 3 };
		bloomSixteenth = { mProgram->getUniformLocation("bloomSixteenth"), UniformType::TEXTURE2D, 4 };

		mSampler.setAnisotropy(0.0f);
		mSampler.setMinFilter(TextureFilter::Linear);
		mSampler.setMagFilter(TextureFilter::Linear);
	}

	UniformTex sourceTextureUniform;
	UniformTex bloomHalfth;
	UniformTex bloomQuarter;
	UniformTex bloomEigth;
	UniformTex bloomSixteenth;
	Sampler mSampler;
};

nex::PostProcessor::PostProcessor(unsigned width, unsigned height, DownSampler* downSampler, GaussianBlur* gaussianBlur) :
mPostprocessPass(std::make_unique<PostProcessShader>()), mDownSampler(downSampler), mGaussianBlur(gaussianBlur)
{
	mFullscreenPlane = StaticMeshManager::get()->getNDCFullscreenPlane();
	
	resize(width, height);
}

nex::PostProcessor::~PostProcessor() = default;

void nex::PostProcessor::doPostProcessing(Texture2D* source, Texture2D* glowTexture, RenderTarget2D* output)
{
	// Bloom
	auto* glowHalfth = mDownSampler->downsampleHalfResolution(glowTexture);
	auto* glowQuarter = mDownSampler->downsampleQuarterResolution(glowHalfth);
	auto* glowEigth = mDownSampler->downsampleEigthResolution(glowQuarter);
	auto* glowSixteenth = mDownSampler->downsampleSixteenthResolution(glowEigth);

	glowHalfth = mGaussianBlur->blurHalfResolution(glowHalfth, mBloomHalfth.get());
	glowQuarter = mGaussianBlur->blurQuarterResolution(glowQuarter, mBloomQuarter.get());
	glowEigth = mGaussianBlur->blurEigthResolution(glowEigth, mBloomEigth.get());
	glowSixteenth = mGaussianBlur->blurSixteenthResolution(glowSixteenth, mBloomSixteenth.get());

	
	// Post process
	output->bind();
	RenderBackend::get()->setViewPort(0, 0, output->getWidth(), output->getHeight());
	mPostprocessPass->bind();
	//TextureManager::get()->getDefaultImageSampler()->bind(0);
	mPostprocessPass->mSampler.bind(0);
	setPostProcessTexture(source);
	setGlowTextures(glowHalfth, glowQuarter, glowEigth, glowSixteenth);

	mFullscreenPlane->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);
	mPostprocessPass->mSampler.unbind(0);
}

void nex::PostProcessor::resize(unsigned width, unsigned height)
{
	mBloomHalfth = std::make_unique<RenderTarget2D>(width / 2, height / 2, TextureData::createRenderTargetRGBAHDR());
	mBloomQuarter = std::make_unique<RenderTarget2D>(width / 4, height / 4, TextureData::createRenderTargetRGBAHDR());
	mBloomEigth = std::make_unique<RenderTarget2D>(width / 8, height / 8, TextureData::createRenderTargetRGBAHDR());
	mBloomSixteenth = std::make_unique<RenderTarget2D>(width / 16, height / 16, TextureData::createRenderTargetRGBAHDR());
}

void nex::PostProcessor::setPostProcessTexture(Texture* texture)
{
	auto& uniform = mPostprocessPass->sourceTextureUniform;
	mPostprocessPass->getProgram()->setTexture(uniform.location, texture, uniform.bindingSlot);
}

void nex::PostProcessor::setGlowTextures(Texture* halfth, nex::Texture* quarter, nex::Texture* eigth, nex::Texture* sixteenth)
{
	mPostprocessPass->getProgram()->setTexture(mPostprocessPass->bloomHalfth.location, halfth, mPostprocessPass->bloomHalfth.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(mPostprocessPass->bloomQuarter.location, quarter, mPostprocessPass->bloomQuarter.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(mPostprocessPass->bloomEigth.location, eigth, mPostprocessPass->bloomEigth.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(mPostprocessPass->bloomSixteenth.location, sixteenth, mPostprocessPass->bloomSixteenth.bindingSlot);
}