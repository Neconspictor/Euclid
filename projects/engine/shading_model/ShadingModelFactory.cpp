#include <shading_model/ShadingModelFactory.hpp>

using namespace std;


ShadingModelFactory::ShadingModelFactory(Renderer3D* renderer) : renderer(renderer)
{
}

std::unique_ptr<PBR> ShadingModelFactory::create_PBR_Model(Texture* backgroundHDR)
{
	// ensure that PBR is not created twice
	return make_unique<PBR>(renderer, backgroundHDR);
}