#include <shading_model/PBR_Deferred.hpp>
#include <shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace glm;

PBR_Deferred::PBR_Deferred(Renderer3D* renderer, Texture* backgroundHDR) : PBR(renderer, backgroundHDR)
{
}

PBR_Deferred::~PBR_Deferred()
{
}