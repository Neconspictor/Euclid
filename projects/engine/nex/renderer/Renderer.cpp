#include <nex/renderer/Renderer.hpp>
#include <nex/pbr/pbr.hpp>
#include <nex/pbr/Cluster.hpp>

nex::Renderer::Renderer(nex::PbrTechnique * pbrTechnique) : mPbrTechnique(pbrTechnique),
mWidth(0), mHeight(0), mActiveRenderLayerProvider([]() {return nullptr; })
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

const std::vector<std::string>& nex::Renderer::getRenderLayerDescriptions()
{
	return mRenderLayerDescs;
}

nex::Texture* nex::Renderer::getActiveRenderLayer()
{
	return mActiveRenderLayerProvider();
}

void nex::Renderer::setActiveRenderLayer(const std::string& desc)
{
	auto it = mRenderlayers.find(desc);
	if (it != mRenderlayers.end())
		mActiveRenderLayerProvider = it->second;
}