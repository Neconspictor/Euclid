#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/opengl/texture/TextureGL.hpp>

using namespace glm;

using namespace std;

PBR_DeferredGL::PBR_DeferredGL(RenderBackend* renderer, Texture* backgroundHDR) :
	PBR_Deferred(renderer, backgroundHDR)
{
}

PBR_DeferredGL::~PBR_DeferredGL()
{
}

void PBR_DeferredGL::drawGeometryScene(SceneNode * scene, const glm::mat4 & view, const glm::mat4 & projection)
{
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilMask(0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	PBR_Deferred::drawGeometryScene(scene, view, projection);
	glDisable(GL_STENCIL_TEST);
}

void PBR_DeferredGL::drawLighting(SceneNode * scene, PBR_GBuffer * gBuffer, Texture * shadowMap, Texture * ssaoMap, const DirectionalLight & light, const glm::mat4 & viewFromGPass, const glm::mat4 & worldToLight)
{
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, 1);
	//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	//glStencilMask(0x00);
	PBR_Deferred::drawLighting(scene, gBuffer, shadowMap, ssaoMap, light, viewFromGPass, worldToLight);
	//glStencilMask(0xff);
	glDisable(GL_STENCIL_TEST);

	//PBR_Deferred::drawLighting(scene, frameTimeElapsed, gBuffer, shadowMap, ssaoMap, light, viewFromGPass, worldToLight);
}

void PBR_DeferredGL::drawSky(const glm::mat4 & projection, const glm::mat4 & view)
{
	//glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 1);
	//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	//glStencilMask(0x00);
	PBR::drawSky(projection, view);
	//glStencilMask(0xff);
	glDisable(GL_STENCIL_TEST);
}

std::unique_ptr<PBR_GBuffer> PBR_DeferredGL::createMultipleRenderTarget(int width, int height)
{
	return make_unique<PBR_GBufferGL>(width, height);
}