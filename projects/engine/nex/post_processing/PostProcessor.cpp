#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/MeshManager.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/post_processing/DownSampler.hpp>
#include <nex/post_processing//SMAA.hpp>
#include "AmbientOcclusion.hpp"
#include "nex/drawing/StaticMeshDrawer.hpp"
#include <nex/post_processing/FXAA.hpp>
#include <nex/post_processing/TAA.hpp>
#include <nex/post_processing/SSR.hpp>


class nex::PostProcessor::PostProcessPass : public nex::Shader
{
public:
	PostProcessPass()
	{
		mProgram = nex::ShaderProgram::create("screen_space_vs.glsl", "post_processing/postProcess_fs.glsl");
		sourceTextureUniform = { mProgram->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D, 0 };
		bloomHalfth = { mProgram->getUniformLocation("bloomHalfth"), UniformType::TEXTURE2D, 1 };
		bloomQuarter = { mProgram->getUniformLocation("bloomQuarter"), UniformType::TEXTURE2D, 2 };
		bloomEigth = { mProgram->getUniformLocation("bloomEigth"), UniformType::TEXTURE2D, 3 };
		bloomSixteenth = { mProgram->getUniformLocation("bloomSixteenth"), UniformType::TEXTURE2D, 4 };

		aoMap = { mProgram->getUniformLocation("aoMap"), UniformType::TEXTURE2D, 5 };

		motionMap = { mProgram->getUniformLocation("motionMap"), UniformType::TEXTURE2D, 6 };

		mProgram->setBinding(sourceTextureUniform.location, sourceTextureUniform.bindingSlot);
		mProgram->setBinding(bloomHalfth.location, bloomHalfth.bindingSlot);
		mProgram->setBinding(bloomQuarter.location, bloomQuarter.bindingSlot);
		mProgram->setBinding(bloomEigth.location, bloomEigth.bindingSlot);
		mProgram->setBinding(bloomSixteenth.location, bloomSixteenth.bindingSlot);
		mProgram->setBinding(aoMap.location, aoMap.bindingSlot);
		mProgram->setBinding(motionMap.location, motionMap.bindingSlot);


		
	}


	UniformTex sourceTextureUniform;
	UniformTex bloomHalfth;
	UniformTex bloomQuarter;
	UniformTex bloomEigth;
	UniformTex bloomSixteenth;
	UniformTex aoMap;
	UniformTex motionMap;
	
};


class nex::PostProcessor::AoPass : public nex::Shader
{
public:
	AoPass()
	{
		mProgram = nex::ShaderProgram::create("screen_space_vs.glsl", "post_processing/ao_fs.glsl");

		mAoMap = mProgram->createTextureUniform("aoMap", UniformType::TEXTURE2D, 0);
	}

	void setAoMap(Texture* aoMap) {
		mProgram->setTexture(aoMap, Sampler::getPoint(), mAoMap.bindingSlot);
	}

	UniformTex mAoMap;
};

nex::PostProcessor::PostProcessor(unsigned width, unsigned height, DownSampler* downSampler, GaussianBlur* gaussianBlur) :
mPostprocessPass(std::make_unique<PostProcessPass>()), mDownSampler(downSampler), mGaussianBlur(gaussianBlur),
mAoPass(std::make_unique<AoPass>()),
mAoSelector(std::make_unique<AmbientOcclusionSelector>(width, height)),
mFxaa(std::make_unique<FXAA>()),
mTaa(std::make_unique<TAA>()),
mSSR(std::make_unique<SSR>())
{	
	resize(width, height);
}

nex::PostProcessor::~PostProcessor() = default;

nex::PostProcessor::BloomTextures nex::PostProcessor::computeBloom(Texture2D * source, Texture2D * bloomTexture)
{
	// Bloom
	auto* half = mDownSampler->downsampleHalfResolution(bloomTexture);
	auto* quarter = mDownSampler->downsampleQuarterResolution(half);
	auto* eighth = mDownSampler->downsampleEighthResolution(quarter);
	auto* sixteenth = mDownSampler->downsampleSixteenthResolution(eighth);

	half = mGaussianBlur->blurHalfResolution(half, mBloomHalfth.get());
	quarter = mGaussianBlur->blurQuarterResolution(quarter, mBloomQuarter.get());
	eighth = mGaussianBlur->blurEighthResolution(eighth, mBloomEighth.get());
	sixteenth = mGaussianBlur->blurSixteenthResolution(sixteenth, mBloomSixteenth.get());

	return { half , quarter , eighth , sixteenth };
}

void nex::PostProcessor::doPostProcessing(Texture2D* source, 
	Texture2D* glowTexture, 
	Texture2D* aoMap, 
	Texture2D* motionMap, 
	const BloomTextures& bloomTextures)
{	
	mPostprocessPass->bind();
	setPostProcessTexture(source);
	setGlowTextures(bloomTextures.bloomHalf, bloomTextures.bloomQuarter, bloomTextures.bloomEighth, bloomTextures.bloomSixteenth);
	setAoMap(aoMap);
	setMotionMap(motionMap);

	const auto& state = RenderState::getNoDepthTest();
	StaticMeshDrawer::drawFullscreenTriangle(state, mPostprocessPass.get());
}

void nex::PostProcessor::antialias(Texture2D * source, RenderTarget * output)
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

nex::FXAA* nex::PostProcessor::getFXAA()
{
	return mFxaa.get();
}

nex::TAA* nex::PostProcessor::getTAA()
{
	return mTaa.get();
}

nex::SSR* nex::PostProcessor::getSSR()
{
	return mSSR.get();
}

void nex::PostProcessor::resize(unsigned width, unsigned height)
{
	mBloomHalfth = std::make_unique<RenderTarget2D>(width / 2, height / 2, TextureDesc::createRenderTargetRGBAHDR());
	mBloomQuarter = std::make_unique<RenderTarget2D>(width / 4, height / 4, TextureDesc::createRenderTargetRGBAHDR());
	mBloomEighth = std::make_unique<RenderTarget2D>(width / 8, height / 8, TextureDesc::createRenderTargetRGBAHDR());
	mBloomSixteenth = std::make_unique<RenderTarget2D>(width / 16, height / 16, TextureDesc::createRenderTargetRGBAHDR());

	// Avoid double resizing of SMAA class.
	if (!mSmaa)
	{
		mSmaa = std::make_unique<SMAA>(width, height);
	} else
	{
		mSmaa->resize(width, height);
	}

	mAoSelector->onSizeChange(width, height);

	mSSR->resize(width, height);
}

nex::AmbientOcclusionSelector* nex::PostProcessor::getAOSelector()
{
	return mAoSelector.get();
}

void nex::PostProcessor::renderAO(Texture* aoMap)
{
	mAoPass->bind();
	mAoPass->setAoMap(aoMap);

	const auto& state = RenderState::getMultiplicativeBlending();
	StaticMeshDrawer::drawFullscreenTriangle(state, mAoPass.get());
}

void nex::PostProcessor::setAoMap(Texture2D* aoMap)
{
	auto& uniform = mPostprocessPass->aoMap;
	mPostprocessPass->getShader()->setTexture(aoMap, Sampler::getLinear(), uniform.bindingSlot);
}

void nex::PostProcessor::setMotionMap(Texture2D* motionMap)
{
	auto& uniform = mPostprocessPass->motionMap;
	mPostprocessPass->getShader()->setTexture(motionMap, Sampler::getLinear(), uniform.bindingSlot);
}

void nex::PostProcessor::setPostProcessTexture(Texture* texture)
{
	auto& uniform = mPostprocessPass->sourceTextureUniform;
	mPostprocessPass->getShader()->setTexture(texture, Sampler::getLinear(), uniform.bindingSlot);
}

void nex::PostProcessor::setGlowTextures(Texture* halfth, nex::Texture* quarter, nex::Texture* eigth, nex::Texture* sixteenth)
{
	mPostprocessPass->getShader()->setTexture(halfth, Sampler::getLinear(), mPostprocessPass->bloomHalfth.bindingSlot);
	mPostprocessPass->getShader()->setTexture(quarter, Sampler::getLinear(), mPostprocessPass->bloomQuarter.bindingSlot);
	mPostprocessPass->getShader()->setTexture(eigth, Sampler::getLinear(), mPostprocessPass->bloomEigth.bindingSlot);
	mPostprocessPass->getShader()->setTexture(sixteenth, Sampler::getLinear(), mPostprocessPass->bloomSixteenth.bindingSlot);
}