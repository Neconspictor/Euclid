#include <nex/shading_model/PBR_Deferred.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/gui/Util.hpp>
#include <nex/shader/ShaderManager.hpp>
#include <imgui/imgui.h>

using namespace glm;

PBR_Deferred::PBR_Deferred(RenderBackend* renderer, Texture* backgroundHDR) : PBR(renderer, backgroundHDR)
{
	vec2 dim = { 1.0, 1.0 };
	vec2 pos = { 0, 0 };

	// center
	pos.x = 0.5f * (1.0f - dim.x);
	pos.y = 0.5f * (1.0f - dim.y);

	screenSprite.setPosition(pos);
	screenSprite.setWidth(dim.x);
	screenSprite.setHeight(dim.y);
}

void PBR_Deferred::drawGeometryScene(SceneNode * scene,
	const glm::mat4 & view, 
	const glm::mat4 & projection)
{
	PBRShader_Deferred_Geometry* shader = dynamic_cast<PBRShader_Deferred_Geometry*> (
		renderer->getShaderManager()->getConfig(Shaders::Pbr_Deferred_Geometry));

	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	scene->draw(renderer, modelDrawer, projection, view, Shaders::Pbr_Deferred_Geometry);
}

void PBR_Deferred::drawLighting(SceneNode * scene,
	PBR_GBuffer * gBuffer, 
	Texture * shadowMap, 
	Texture* ssaoMap,
	const DirectionalLight & light, 
	const glm::mat4 & viewFromGPass, 
	const glm::mat4 & worldToLight,
	CascadedShadow::CascadeData* cascadeData,
	Texture* cascadedDepthMap)
{

	Shader* shader = renderer->getShaderManager()->getShader(Shaders::Pbr_Deferred_Lighting);
	shader->use();

	PBRShader_Deferred_Lighting* config = dynamic_cast<PBRShader_Deferred_Lighting*> (renderer->getShaderManager()->getConfig(Shaders::Pbr_Deferred_Lighting));

	config->setBrdfLookupTexture(brdfLookupTexture->getTexture());
	config->setGBuffer(gBuffer);
	config->setInverseViewFromGPass(inverse(viewFromGPass));
	config->setIrradianceMap(convolutedEnvironmentMap->getCubeMap());
	config->setLightColor(light.getColor());
	config->setLightDirection(light.getLook());
	config->setPrefilterMap(prefilterRenderTarget->getCubeMap());
	config->setShadowMap(shadowMap);
	config->setAOMap(ssaoMap);
	config->setSkyBox(environmentMap->getCubeMap());
	config->setWorldToLightSpaceMatrix(worldToLight);
	config->setCascadedData(cascadeData);
	config->setCascadedDepthMap(cascadedDepthMap);


	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	modelDrawer->draw(&screenSprite, Shaders::Pbr_Deferred_Lighting);
}

PBR_Deferred_ConfigurationView::PBR_Deferred_ConfigurationView(PBR_Deferred* pbr) : m_pbr(pbr)
{
}

void PBR_Deferred_ConfigurationView::drawSelf()
{
	ImGui::PushID(m_id.c_str());
	//m_pbr
	ImGui::LabelText("", "PBR:");
	ImGui::Dummy(ImVec2(0, 20));
	nex::engine::gui::Separator(2.0f);
	ImGui::PopID();
}