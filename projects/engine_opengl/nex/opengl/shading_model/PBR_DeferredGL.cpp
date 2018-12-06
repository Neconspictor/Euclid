#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>
#include <nex/opengl/shader/SkyBoxShaderGL.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/gui/Util.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/opengl/shader/PBRShaderGL.hpp>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

using namespace std;

namespace nex {

	PBR_DeferredGL::PBR_DeferredGL(RendererOpenGL* renderer, Texture* backgroundHDR) :
		PBR(renderer, backgroundHDR)
	{

		//TextureManagerGL* textureManager = dynamic_cast<TextureManagerGL*>(renderer->getTextureManager());
		//textureManager->registerAnistropySampler(&m_sampler);

		vec2 dim = { 1.0, 1.0 };
		vec2 pos = { 0, 0 };

		// center
		pos.x = 0.5f * (1.0f - dim.x);
		pos.y = 0.5f * (1.0f - dim.y);

		screenSprite.setPosition(pos);
		screenSprite.setWidth(dim.x);
		screenSprite.setHeight(dim.y);
	}

	void PBR_DeferredGL::drawGeometryScene(SceneNode * scene, const glm::mat4 & view, const glm::mat4 & projection)
	{
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 1);
		glStencilMask(0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		PBRShader_Deferred_Geometry* shader = reinterpret_cast<PBRShader_Deferred_Geometry*> (
			renderer->getShaderManager()->getShader(ShaderType::Pbr_Deferred_Geometry));

		ModelDrawerGL* modelDrawer = renderer->getModelDrawer();

		shader->bind();
		shader->setView(view);
		shader->setProjection(projection);



		Sampler* sampler = TextureManagerGL::get()->getDefaultImageSampler();

		for (int i = 0; i < 6; ++i)
		{
			sampler->bind(i);
		}

		modelDrawer->draw(scene, shader);

		for (int i = 0; i < 6; ++i)
		{
			sampler->unbind(i);
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

	void PBR_DeferredGL::drawLighting(SceneNode * scene, PBR_GBuffer * gBuffer,
		Texture* shadowMap, Texture* ssaoMap, const DirectionalLight & light, const glm::mat4 & viewFromGPass, const glm::mat4 & worldToLight,
		CascadedShadowGL::CascadeData* cascadeData,
		Texture* cascadedDepthMap)
	{
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 1, 1);
		//glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		//glStencilMask(0x00);

		PBRShader_Deferred_Lighting* shader = reinterpret_cast<PBRShader_Deferred_Lighting*>(
			renderer->getShaderManager()->getShader(ShaderType::Pbr_Deferred_Lighting));


		shader->bind();

		shader->setAlbedoMap(gBuffer->getAlbedo());
		shader->setAoMetalRoughnessMap(gBuffer->getAoMetalRoughness());
		shader->setNormalEyeMap(gBuffer->getNormal());
		shader->setPositionEyeMap(gBuffer->getPosition());

		shader->setBrdfLookupTexture(brdfLookupTexture);
		//shader->setGBuffer(gBuffer);
		shader->setViewGPass(viewFromGPass);
		shader->setInverseViewFromGPass(inverse(viewFromGPass));
		shader->setIrradianceMap(convolutedEnvironmentMap);
		shader->setLightColor(light.getColor());
		shader->setWorldLightDirection(light.getLook());

		vec4 lightEyeDirection = viewFromGPass * vec4(light.getLook(), 0);
		shader->setEyeLightDirection(vec3(lightEyeDirection));
		shader->setPrefilterMap(prefilteredEnvMap);
		shader->setShadowMap(shadowMap);
		shader->setAOMap(ssaoMap);
		//TODO
		//shader->setSkyBox(environmentMap->getCubeMap());
		shader->setWorldToLightSpaceMatrix(worldToLight);
		shader->setEyeToLightSpaceMatrix(worldToLight  * viewFromGPass);
		shader->setCascadedData(cascadeData);
		shader->setCascadedDepthMap(cascadedDepthMap);


		ModelDrawerGL* modelDrawer = renderer->getModelDrawer();
		modelDrawer->draw(&screenSprite, shader);

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
		return make_unique<PBR_GBuffer>(width, height);
	}



	PBR_Deferred_ConfigurationView::PBR_Deferred_ConfigurationView(PBR_DeferredGL* pbr) : m_pbr(pbr)
	{
	}

	void PBR_Deferred_ConfigurationView::drawSelf()
	{
		ImGui::PushID(m_id.c_str());
		//m_pbr
		ImGui::LabelText("", "PBR:");
		ImGui::Dummy(ImVec2(0, 20));
		nex::gui::Separator(2.0f);
		ImGui::PopID();
	}
}