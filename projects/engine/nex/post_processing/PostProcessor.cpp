#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Pass.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/post_processing/DownSampler.hpp>
#include <nex/post_processing//SMAA.hpp>
#include "AmbientOcclusion.hpp"
#include "nex/drawing/StaticMeshDrawer.hpp"
#include <nex/post_processing/FXAA.hpp>
#include <nex/post_processing/TAA.hpp>


class nex::PostProcessor::PostProcessPass : public nex::Pass
{
public:
	PostProcessPass()
	{
		mShader = nex::Shader::create("screen_space_vs.glsl", "post_processing/postProcess_fs.glsl");
		sourceTextureUniform = { mShader->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D, 0 };
		bloomHalfth = { mShader->getUniformLocation("bloomHalfth"), UniformType::TEXTURE2D, 1 };
		bloomQuarter = { mShader->getUniformLocation("bloomQuarter"), UniformType::TEXTURE2D, 2 };
		bloomEigth = { mShader->getUniformLocation("bloomEigth"), UniformType::TEXTURE2D, 3 };
		bloomSixteenth = { mShader->getUniformLocation("bloomSixteenth"), UniformType::TEXTURE2D, 4 };

		aoMap = { mShader->getUniformLocation("aoMap"), UniformType::TEXTURE2D, 5 };

		motionMap = { mShader->getUniformLocation("motionMap"), UniformType::TEXTURE2D, 6 };

		oceanHeightMap = mShader->createTextureUniform("oceanHeightMap", UniformType::TEXTURE2D, 7);
		depthMap = mShader->createTextureUniform("depthMap", UniformType::TEXTURE2D, 8);
		stencilMap = mShader->createTextureUniform("stencilMap", UniformType::TEXTURE2D, 9);
		//oceanDX = mShader->createTextureUniform("oceanDX", UniformType::TEXTURE2D, 9);
		//oceanDZ = mShader->createTextureUniform("oceanDZ", UniformType::TEXTURE2D, 10);
		
		oceanMinHeightMap = mShader->createTextureUniform("oceanMinHeightMap", UniformType::TEXTURE1D, 10);
		oceanMaxHeightMap = mShader->createTextureUniform("oceanMaxHeightMap", UniformType::TEXTURE1D, 11);
		

		mShader->setBinding(sourceTextureUniform.location, sourceTextureUniform.bindingSlot);
		mShader->setBinding(bloomHalfth.location, bloomHalfth.bindingSlot);
		mShader->setBinding(bloomQuarter.location, bloomQuarter.bindingSlot);
		mShader->setBinding(bloomEigth.location, bloomEigth.bindingSlot);
		mShader->setBinding(bloomSixteenth.location, bloomSixteenth.bindingSlot);
		mShader->setBinding(aoMap.location, aoMap.bindingSlot);
		mShader->setBinding(motionMap.location, motionMap.bindingSlot);


		inverseViewProjMatrix_GPass = { mShader->getUniformLocation("inverseViewProjMatrix_GPass"), UniformType::MAT4};
		inverseModelMatrix_Ocean = { mShader->getUniformLocation("inverseModelMatrix_Ocean"), UniformType::MAT4 };
		oceanTileSize = { mShader->getUniformLocation("oceanTileSize"), UniformType::FLOAT };
		cameraPosition = { mShader->getUniformLocation("cameraPosition"), UniformType::VEC3 };
	}

	void setInverseViewProjMatrix_GPass(const glm::mat4& mat) {
		mShader->setMat4(inverseViewProjMatrix_GPass.location, mat);
	}

	void setInverseModelMatrix_Ocean(const glm::mat4& mat) {
		mShader->setMat4(inverseModelMatrix_Ocean.location, mat);
	}

	void setOceanTileSize(float tileSize) {
		mShader->setFloat(oceanTileSize.location, tileSize);
	}

	void setDepthMap(Texture* texture) {
		mShader->setTexture(texture,  Sampler::getPoint(), depthMap.bindingSlot);
	}

	void setOceanHeightMap(Texture* texture) {
		mShader->setTexture(texture, Sampler::getLinearRepeat(), oceanHeightMap.bindingSlot);
	}

	void setOceanDX(Texture* texture) {
		//mShader->setTexture(texture, Sampler::getLinearRepeat(), oceanDX.bindingSlot);
	}

	void setOceanDZ(Texture* texture) {
		//mShader->setTexture(texture, Sampler::getLinearRepeat(), oceanDZ.bindingSlot);
	}

	void setOceanMinHeightMap(Texture* texture) {
		mShader->setTexture(texture, Sampler::getLinearRepeat(), oceanMinHeightMap.bindingSlot);
	}

	void setOceanMaxHeightMap(Texture* texture) {
		mShader->setTexture(texture, Sampler::getLinearRepeat(), oceanMaxHeightMap.bindingSlot);
	}

	void setStencilMap(Texture* texture) {
		mShader->setTexture(texture, Sampler::getPoint(), stencilMap.bindingSlot);
	}

	void setCameraPosition(const glm::vec3& pos) {
		mShader->setVec3(cameraPosition.location, pos);
	}


	UniformTex sourceTextureUniform;
	UniformTex bloomHalfth;
	UniformTex bloomQuarter;
	UniformTex bloomEigth;
	UniformTex bloomSixteenth;
	UniformTex aoMap;
	UniformTex motionMap;
	UniformTex oceanHeightMap;
	UniformTex depthMap;
	UniformTex oceanDX;
	UniformTex oceanDZ;
	UniformTex oceanMinHeightMap;
	UniformTex oceanMaxHeightMap;
	UniformTex stencilMap;

	Uniform inverseViewProjMatrix_GPass;
	Uniform inverseModelMatrix_Ocean;
	Uniform oceanTileSize;
	Uniform cameraPosition;
};


class nex::PostProcessor::AoPass : public nex::Pass
{
public:
	AoPass()
	{
		mShader = nex::Shader::create("screen_space_vs.glsl", "post_processing/ao_fs.glsl");

		mAoMap = mShader->createTextureUniform("aoMap", UniformType::TEXTURE2D, 0);
	}

	void setAoMap(Texture* aoMap) {
		mShader->setTexture(aoMap, Sampler::getPoint(), mAoMap.bindingSlot);
	}

	UniformTex mAoMap;
};

nex::PostProcessor::PostProcessor(unsigned width, unsigned height, DownSampler* downSampler, GaussianBlur* gaussianBlur) :
mPostprocessPass(std::make_unique<PostProcessPass>()), mDownSampler(downSampler), mGaussianBlur(gaussianBlur),
mAoPass(std::make_unique<AoPass>()),
mAoSelector(std::make_unique<AmbientOcclusionSelector>(width, height)),
mFxaa(std::make_unique<FXAA>()),
mTaa(std::make_unique<TAA>())
{	
	resize(width, height);
}

nex::PostProcessor::~PostProcessor() = default;

nex::Texture* nex::PostProcessor::doPostProcessing(Texture2D* source, 
	Texture2D* glowTexture, 
	Texture2D* aoMap, 
	Texture2D* motionMap, 
	Texture* depth,
	Texture* stencil,
	Texture* oceanHeightMap,
	Texture* oceanDX,
	Texture* oceanDZ,
	Texture* oceanMinHeightMap,
	Texture* oceanMaxHeightMap,
	float oceanTileSize,
	const glm::mat4& inverseModelMatrix_Ocean,
	const glm::mat4& inverseViewProjection_GPass,
	const glm::vec3& cameraPosition,
	RenderTarget* output)
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
	setPostProcessTexture(source);
	setGlowTextures(glowHalfth, glowQuarter, glowEigth, glowSixteenth);
	setAoMap(aoMap);
	setMotionMap(motionMap);

	//ocean
	mPostprocessPass->setDepthMap(depth);
	mPostprocessPass->setStencilMap(stencil);
	mPostprocessPass->setOceanHeightMap(oceanHeightMap);
	mPostprocessPass->setOceanDX(oceanDX);
	mPostprocessPass->setOceanDZ(oceanDZ);
	mPostprocessPass->setOceanMinHeightMap(oceanMinHeightMap);
	mPostprocessPass->setOceanMaxHeightMap(oceanMaxHeightMap);
	mPostprocessPass->setInverseModelMatrix_Ocean(inverseModelMatrix_Ocean);
	mPostprocessPass->setInverseViewProjMatrix_GPass(inverseViewProjection_GPass);
	mPostprocessPass->setOceanTileSize(oceanTileSize);
	mPostprocessPass->setCameraPosition(cameraPosition);


	const auto& state = RenderState::getNoDepthTest();
	StaticMeshDrawer::drawFullscreenTriangle(state, mPostprocessPass.get());
	return output->getColorAttachmentTexture(0);
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

void nex::PostProcessor::resize(unsigned width, unsigned height)
{
	mBloomHalfth = std::make_unique<RenderTarget2D>(width / 2, height / 2, TextureDesc::createRenderTargetRGBAHDR());
	mBloomQuarter = std::make_unique<RenderTarget2D>(width / 4, height / 4, TextureDesc::createRenderTargetRGBAHDR());
	mBloomEigth = std::make_unique<RenderTarget2D>(width / 8, height / 8, TextureDesc::createRenderTargetRGBAHDR());
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