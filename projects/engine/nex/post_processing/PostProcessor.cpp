#include <nex/post_processing/PostProcessor.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include "nex/RenderBackend.hpp"

nex::PostProcessor::PostProcessor(unsigned width, unsigned height)
{
	mFullscreenPlane = StaticMeshManager::get()->getNDCFullscreenPlane();
	
	resize(width, height);

	mPostprocessPass = std::make_unique<Shader>(ShaderProgram::create("post_processing/postProcess_vs.glsl", "post_processing/postProcess_fs.glsl"));
}

void nex::PostProcessor::doPostProcessing(RenderTarget2D* renderTarget)
{
	renderTarget->bind();
	mPostprocessPass->bind();
	mFullscreenPlane->bind();
	RenderBackend::drawArray(Topology::TRIANGLE_STRIP, 0, 4);
}

void nex::PostProcessor::resize(unsigned width, unsigned height)
{
	// Note: We don't need a depth-/stencil buffer
	mTemp = std::make_unique<RenderTarget2D>(width, height, TextureData::createRenderTargetRGBAHDR(), 1);
}
