#include <nex/renderer/Renderer.hpp>
#include <nex/pbr/pbr.hpp>

nex::Renderer::Renderer(nex::PbrTechnique * pbrTechnique) : mPbrTechnique(pbrTechnique)
{
}

nex::PbrTechnique * nex::Renderer::getPbrTechnique()
{
	return mPbrTechnique;
}

const nex::PbrTechnique * nex::Renderer::getPbrTechnique() const
{
	return mPbrTechnique;
}
