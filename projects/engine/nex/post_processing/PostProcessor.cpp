#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/post_processing/DownSampler.hpp>
#include <nex/post_processing//SMAA.hpp>
#include "AmbientOcclusion.hpp"


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

		aoMap = { mProgram->getUniformLocation("aoMap"), UniformType::TEXTURE2D, 5 };

		mSampler.setAnisotropy(1.0f);
		mSampler.setMinFilter(TextureFilter::Linear);
		mSampler.setMagFilter(TextureFilter::Linear);


		mProgram->setBinding(sourceTextureUniform.location, sourceTextureUniform.bindingSlot);
		mProgram->setBinding(bloomHalfth.location, bloomHalfth.bindingSlot);
		mProgram->setBinding(bloomQuarter.location, bloomQuarter.bindingSlot);
		mProgram->setBinding(bloomEigth.location, bloomEigth.bindingSlot);
		mProgram->setBinding(bloomSixteenth.location, bloomSixteenth.bindingSlot);
		mProgram->setBinding(aoMap.location, aoMap.bindingSlot);
	}

	UniformTex sourceTextureUniform;
	UniformTex bloomHalfth;
	UniformTex bloomQuarter;
	UniformTex bloomEigth;
	UniformTex bloomSixteenth;
	UniformTex aoMap;
	Sampler mSampler;
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
	output->clear(Color | Depth | Stencil);
	RenderBackend::get()->setViewPort(0, 0, output->getWidth(), output->getHeight());
	mPostprocessPass->bind();
	//TextureManager::get()->getDefaultImageSampler()->bind(0);
	setPostProcessTexture(source);
	setGlowTextures(glowHalfth, glowQuarter, glowEigth, glowSixteenth);
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
	mPostprocessPass->mSampler.bind(uniform.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(aoMap, uniform.bindingSlot);
}

void nex::PostProcessor::setPostProcessTexture(Texture* texture)
{
	auto& uniform = mPostprocessPass->sourceTextureUniform;
	mPostprocessPass->mSampler.bind(uniform.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(texture, uniform.bindingSlot);
}

void nex::PostProcessor::setGlowTextures(Texture* halfth, nex::Texture* quarter, nex::Texture* eigth, nex::Texture* sixteenth)
{
	mPostprocessPass->mSampler.bind(mPostprocessPass->bloomHalfth.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(halfth, mPostprocessPass->bloomHalfth.bindingSlot);
	mPostprocessPass->mSampler.bind(mPostprocessPass->bloomQuarter.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(quarter, mPostprocessPass->bloomQuarter.bindingSlot);
	mPostprocessPass->mSampler.bind(mPostprocessPass->bloomEigth.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(eigth, mPostprocessPass->bloomEigth.bindingSlot);
	mPostprocessPass->mSampler.bind(mPostprocessPass->bloomSixteenth.bindingSlot);
	mPostprocessPass->getProgram()->setTexture(sixteenth, mPostprocessPass->bloomSixteenth.bindingSlot);
}