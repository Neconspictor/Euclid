#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/post_processing/DownSampler.hpp>
#include <nex/post_processing//SMAA.hpp>
#include "AmbientOcclusion.hpp"


class nex::PostProcessor::PostProcessShader : public nex::Pass
{
public:
	PostProcessShader()
	{
		mShader = nex::Shader::create("fullscreenPlane_vs.glsl", "post_processing/postProcess_fs.glsl");
		sourceTextureUniform = { mShader->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D, 0 };
		bloomHalfth = { mShader->getUniformLocation("bloomHalfth"), UniformType::TEXTURE2D, 1 };
		bloomQuarter = { mShader->getUniformLocation("bloomQuarter"), UniformType::TEXTURE2D, 2 };
		bloomEigth = { mShader->getUniformLocation("bloomEigth"), UniformType::TEXTURE2D, 3 };
		bloomSixteenth = { mShader->getUniformLocation("bloomSixteenth"), UniformType::TEXTURE2D, 4 };

		aoMap = { mShader->getUniformLocation("aoMap"), UniformType::TEXTURE2D, 5 };

		mShader->setBinding(sourceTextureUniform.location, sourceTextureUniform.bindingSlot);
		mShader->setBinding(bloomHalfth.location, bloomHalfth.bindingSlot);
		mShader->setBinding(bloomQuarter.location, bloomQuarter.bindingSlot);
		mShader->setBinding(bloomEigth.location, bloomEigth.bindingSlot);
		mShader->setBinding(bloomSixteenth.location, bloomSixteenth.bindingSlot);
		mShader->setBinding(aoMap.location, aoMap.bindingSlot);

		mSampler = &(Pass::mSampler);
	}

	UniformTex sourceTextureUniform;
	UniformTex bloomHalfth;
	UniformTex bloomQuarter;
	UniformTex bloomEigth;
	UniformTex bloomSixteenth;
	UniformTex aoMap;
	Sampler* mSampler;
};

nex::PostProcessor::PostProcessor(unsigned width, unsigned height, DownSampler* downSampler, GaussianBlur* gaussianBlur) :
mPostprocessPass(std::make_unique<PostProcessShader>()), mDownSampler(downSampler), mGaussianBlur(gaussianBlur),
mAoSelector(std::make_unique<AmbientOcclusionSelector>(width, height))
{
	mFullscreenPlane = StaticMeshManager::get()->getNDCFullscreenPlane();
	
	resize(width, height);
}

nex::PostProcessor::~PostProcessor() = default;

nex::Texture2D* nex::PostProcessor::doPostProcessing(Texture2D* source, Texture2D* glowTexture, Texture2D* aoMap, RenderTarget2D* output)
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
	//output->clear(Color | Depth | Stencil);
	output->clear(Depth);
	RenderBackend::get()->setViewPort(0, 0, output->getWidth(), output->getHeight());
	mPostprocessPass->bind();
	//TextureManager::get()->getDefaultImageSampler()->bind(0);
	setPostProcessTexture(source);
	setGlowTextures(glowHalfth, glowQuarter, glowEigth, glowSixteenth);
	//setGlowTextures(glowTexture, glowHalfth, glowHalfth, glowHalfth);
	setAoMap(aoMap);

	mFullscreenPlane->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);
	return output->getColor0AttachmentTexture();
}

void nex::PostProcessor::antialias(Texture2D * source, RenderTarget2D * output)
{
	//Do SMAA antialising after texture is in sRGB (gamma space)
	//But for best results the input read for the color/luma edge detection should *NOT* be sRGB !
	mSmaa->reset();
	auto* edgeTex = mSmaa->renderEdgeDetectionPass(source);
	auto* blendTex = mSmaa->renderBlendingWeigthCalculationPass(edgeTex);
	mSmaa->renderNeighborhoodBlendingPass(blendTex, source, output);
}

nex::SMAA* nex::PostProcessor::getSMAA()
{
	return mSmaa.get();
}

void nex::PostProcessor::resize(unsigned width, unsigned height)
{
	mBloomHalfth = std::make_unique<RenderTarget2D>(width / 2, height / 2, TextureData::createRenderTargetRGBAHDR());
	mBloomQuarter = std::make_unique<RenderTarget2D>(width / 4, height / 4, TextureData::createRenderTargetRGBAHDR());
	mBloomEigth = std::make_unique<RenderTarget2D>(width / 8, height / 8, TextureData::createRenderTargetRGBAHDR());
	mBloomSixteenth = std::make_unique<RenderTarget2D>(width / 16, height / 16, TextureData::createRenderTargetRGBAHDR());

	// Avoid double resizing of SMAA class.
	if (!mSmaa)
	{
		mSmaa = std::make_unique<SMAA>(width, height);
	} else
	{
		mSmaa->resize(width, height);
	}

	mAoSelector->onSizeChange(width, height);
}

nex::AmbientOcclusionSelector* nex::PostProcessor::getAOSelector()
{
	return mAoSelector.get();
}

void nex::PostProcessor::setAoMap(Texture2D* aoMap)
{
	auto& uniform = mPostprocessPass->aoMap;
	mPostprocessPass->getShader()->setTexture(aoMap, mPostprocessPass->mSampler, uniform.bindingSlot);
}

void nex::PostProcessor::setPostProcessTexture(Texture* texture)
{
	auto& uniform = mPostprocessPass->sourceTextureUniform;
	mPostprocessPass->getShader()->setTexture(texture, mPostprocessPass->mSampler, uniform.bindingSlot);
}

void nex::PostProcessor::setGlowTextures(Texture* halfth, nex::Texture* quarter, nex::Texture* eigth, nex::Texture* sixteenth)
{
	mPostprocessPass->getShader()->setTexture(halfth, mPostprocessPass->mSampler, mPostprocessPass->bloomHalfth.bindingSlot);
	mPostprocessPass->getShader()->setTexture(quarter, mPostprocessPass->mSampler, mPostprocessPass->bloomQuarter.bindingSlot);
	mPostprocessPass->getShader()->setTexture(eigth, mPostprocessPass->mSampler, mPostprocessPass->bloomEigth.bindingSlot);
	mPostprocessPass->getShader()->setTexture(sixteenth, mPostprocessPass->mSampler, mPostprocessPass->bloomSixteenth.bindingSlot);
}