#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>

using namespace std;


ShadingModelFactoryGL::ShadingModelFactoryGL(RenderBackend* renderer) : ShadingModelFactory(renderer)
{
}

std::unique_ptr<PBR_Deferred> ShadingModelFactoryGL::create_PBR_Deferred_Model(Texture* backgroundHDR)
{
	return make_unique<PBR_DeferredGL>(renderer, backgroundHDR);
}