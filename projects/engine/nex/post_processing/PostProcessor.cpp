#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/RenderBackend.hpp"

nex::PostProcessor::PostProcessor(unsigned width, unsigned height) : mWidth(width), mHeight(height)
{
	mFullscreenPlane = StaticMeshManager::get()->getNDCFullscreenPlane();
	
	resize(width, height);

	mPostprocessPass = std::make_unique<Shader>(ShaderProgram::create("post_processing/postProcess_vs.glsl", "post_processing/postProcess_fs.glsl"));
	mSourceTextureUniform = { mPostprocessPass->getProgram()->getUniformLocation("sourceTexture"), UniformType::TEXTURE2D};
}

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
	mPostprocessPass->getProgram()->setTexture(mSourceTextureUniform.location, texture, 0);
}