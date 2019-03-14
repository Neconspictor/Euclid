#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/RenderBackend.hpp"
#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Shader.hpp>


class nex::PostProcessor::PostProcessShader : public nex::Shader
{
public:
	PostProcessShader()
	{
		mProgram = nex::ShaderProgram::create("fullscreenPlane_vs.glsl", "post_processing/postProcess_fs.glsl");
		sourceTextureUniform = { mProgram->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D };
	}

	Uniform sourceTextureUniform;
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
	
	// instead of clearing the buffer we just disable depth and stencil tests for improved performance
	RenderBackend::get()->getDepthBuffer()->enableDepthTest(false);
	RenderBackend::get()->getStencilTest()->enableStencilTest(false);


	//renderer->beginScene();
	//output->clear(RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
	output->bind();
	mPostprocessPass->bind();
	setPostProcessTexture(source);

	mFullscreenPlane->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);
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