#include <nex/renderer/Renderer.hpp>
#include <nex/pbr/pbr.hpp>

nex::Renderer::Renderer(nex::PbrTechnique * pbrTechnique) : mPbrTechnique(pbrTechnique),
mWidth(0), mHeight(0)
{
}

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
