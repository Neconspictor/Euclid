#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>

using namespace glm;

using namespace std;

PBR_DeferredGL::PBR_DeferredGL(RenderBackend* renderer, Texture* backgroundHDR) :
	PBR_Deferred(renderer, backgroundHDR)
{

	TextureManagerGL* textureManager = dynamic_cast<TextureManagerGL*>(renderer->getTextureManager());

	//textureManager->registerAnistropySampler(&m_sampler);


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



	//glActiveTexture(GL_TEXTURE0);
	//glBindSampler(0, m_sampler.getID());
	//glBindSampler(0, m_sampler.getID());
	/*glBindSampler(1, m_sampler.getID());
	glBindSampler(2, m_sampler.getID());
	glBindSampler(3, m_sampler.getID());
	glBindSampler(4, m_sampler.getID());
	glBindSampler(5, m_sampler.getID());
	glBindSampler(6, m_sampler.getID());
	glBindSampler(7, m_sampler.getID());
	glBindSampler(8, m_sampler.getID());
	glBindSampler(9, m_sampler.getID());
	glBindSampler(10, m_sampler.getID());
	glBindSampler(11, m_sampler.getID());
	glBindSampler(12, m_sampler.getID());
	glBindSampler(13, m_sampler.getID());
	glBindSampler(14, m_sampler.getID());
	glBindSampler(15, m_sampler.getID());*/

	for (int i = 0; i < 32; ++i)
	{
		//glBindSampler(i, m_sampler.getID());
	}


	PBR_Deferred::drawGeometryScene(scene, view, projection);

	for (int i = 0; i < 7; ++i)
	{
		//glBindSampler(i, GL_FALSE);
	}
	/*glBindSampler(1, GL_FALSE);
	glBindSampler(2, GL_FALSE);
	glBindSampler(3, GL_FALSE);
	glBindSampler(4, GL_FALSE);
	glBindSampler(5, GL_FALSE);
	glBindSampler(6, GL_FALSE);
	glBindSampler(7, GL_FALSE);
	glBindSampler(8, GL_FALSE);
	glBindSampler(9, GL_FALSE);
	glBindSampler(10, GL_FALSE);
	glBindSampler(11, GL_FALSE);
	glBindSampler(12, GL_FALSE);
	glBindSampler(13, GL_FALSE);
	glBindSampler(14, GL_FALSE);
	glBindSampler(15, GL_FALSE);*/

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