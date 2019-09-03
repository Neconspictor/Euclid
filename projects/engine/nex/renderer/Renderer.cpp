#include <nex/renderer/Renderer.hpp>
#include <nex/pbr/pbr.hpp>
#include <nex/pbr/Cluster.hpp>

nex::Renderer::Renderer(nex::PbrTechnique * pbrTechnique) : mPbrTechnique(pbrTechnique),
mWidth(0), mHeight(0),
mProbeCluster(std::make_unique<ProbeCluster>())
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

const nex::PbrTechnique * nex::Renderer::getPbrTechnique() const
{
	return mPbrTechnique;
}

nex::ProbeCluster* nex::Renderer::getProbeCluster() {
	return mProbeCluster.get();
}

const nex::ProbeCluster* nex::Renderer::getProbeCluster() const {
	return mProbeCluster.get();
}