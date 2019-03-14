#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/RenderBackend.hpp"
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Shader.hpp>
#include "nex/texture/TextureManager.hpp"
#include "nex/texture/Sampler.hpp"


class nex::PostProcessor::PostProcessShader : public nex::Shader
{
public:
	PostProcessShader() : mSampler({})
	{
		mProgram = nex::ShaderProgram::create("fullscreenPlane_vs.glsl", "post_processing/postProcess_fs.glsl");
		sourceTextureUniform = { mProgram->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D };

		mSampler.setAnisotropy(0.0f);
		mSampler.setMinFilter(TextureFilter::Linear);
		mSampler.setMagFilter(TextureFilter::Linear);
	}

	Uniform sourceTextureUniform;
	Sampler mSampler;
};

nex::PostProcessor::PostProcessor(unsigned width, unsigned height) : mWidth(width), mHeight(height),
mPostprocessPass(std::make_unique<PostProcessShader>())
{
	mFullscreenPlane = StaticMeshManager::get()->getNDCFullscreenPlane();
	
	resize(width, height);
}

nex::PostProcessor::~PostProcessor() = default;

void nex::PostProcessor::doPostProcessing(Texture* source, RenderTarget2D* output)
{
	RenderBackend::get()->setViewPort(0, 0, mWidth, mHeight);

	//renderer->beginScene();
	//output->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
	output->bind();
	mPostprocessPass->bind();
	//TextureManager::get()->getDefaultImageSampler()->bind(0);
	mPostprocessPass->mSampler.bind(0);
	setPostProcessTexture(source);

	mFullscreenPlane->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);
	mPostprocessPass->mSampler.unbind(0);
	//RenderBackend::get()->getDepthBuffer()->enableDepthTest(true);
}

void nex::PostProcessor::resize(unsigned width, unsigned height)
{
	// Note: We don't need a depth-/stencil buffer
	mWidth = width;
	mHeight = height;
	mTemp = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);
}

void nex::PostProcessor::setPostProcessTexture(Texture* texture)
{
	mPostprocessPass->getProgram()->setTexture(mPostprocessPass->sourceTextureUniform.location, texture, 0);
}