#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>

using namespace std;


ShadingModelFactoryGL::ShadingModelFactoryGL()
{
}

std::unique_ptr<PBR_DeferredGL> ShadingModelFactoryGL::create_PBR_Deferred_Model(RendererOpenGL* renderer, TextureGL* backgroundHDR)
{
	return make_unique<PBR_DeferredGL>(renderer, backgroundHDR);
}