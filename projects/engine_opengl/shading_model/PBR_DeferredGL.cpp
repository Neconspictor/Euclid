#include <shading_model/PBR_DeferredGL.hpp>
#include <shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <texture/TextureGL.hpp>

using namespace glm;

using namespace std;

PBR_DeferredGL::PBR_DeferredGL(Renderer3D* renderer, Texture* backgroundHDR) :
	PBR_Deferred(renderer, backgroundHDR)
{
}

PBR_DeferredGL::~PBR_DeferredGL()
{
}

PBR_GBuffer* PBR_DeferredGL::createMultipleRenderTarget(int width, int height)
{
	renderTargets.emplace_back(move(PBR_GBufferGL(width, height)));
	return &renderTargets.back();
}