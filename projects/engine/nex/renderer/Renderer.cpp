#include <nex/renderer/Renderer.hpp>
#include <nex/pbr/pbr.hpp>
#include <nex/pbr/Cluster.hpp>

nex::Renderer::Renderer(nex::PbrTechnique * pbrTechnique) : mPbrTechnique(pbrTechnique),
mWidth(0), mHeight(0), mActiveRenderLayer(0)
{
}

nex::Renderer::~Renderer() = default;

unsigned nex::Renderer::getWidth() const
{
	return mWidth;
}

unsigned nex::Renderer::getHeight() const
{
	return mHeight;
}

void nex::Renderer::updateRenderTargets(unsigned width, unsigned height)
{
	mWidth = width;
	mHeight = height;
}

nex::PbrTechnique * nex::Renderer::getPbrTechnique()
{
	return mPbrTechnique;
}

const nex::PbrTechnique* nex::Renderer::getPbrTechnique() const
{
	return mPbrTechnique;
}

const std::vector<nex::Renderer::RenderLayer>& nex::Renderer::getRenderLayers()
{
	return mRenderLayers;
}

size_t nex::Renderer::getRenderLayerIndexByName(const std::string& desc) const
{
	for (int i = 0; i < mRenderLayers.size(); ++i) {
		if (mRenderLayers[i].desc == desc) return i;
	}

	//default value
	return 0;
}

size_t nex::Renderer::getActiveRenderLayer() const
{
	return mActiveRenderLayer;
}

void nex::Renderer::setActiveRenderLayer(size_t index)
{
	if (mRenderLayers.size() <= index) {
		throw_with_trace(std::runtime_error("nex::Renderer::setActiveRenderLayer : index out of bounds!"));
	}

	mActiveRenderLayer = index;
		
}