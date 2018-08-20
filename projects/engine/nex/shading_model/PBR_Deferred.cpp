#include <nex/shading_model/PBR_Deferred.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/renderer/RenderBackend.hpp>

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
	const glm::mat4 & worldToLight)
{

	PBRShader_Deferred_Lighting* shader = dynamic_cast<PBRShader_Deferred_Lighting*> (renderer->getShaderManager()->getConfig(Shaders::Pbr_Deferred_Lighting));

	shader->setBrdfLookupTexture(brdfLookupTexture->getTexture());
	shader->setGBuffer(gBuffer);
	shader->setInverseViewFromGPass(inverse(viewFromGPass));
	shader->setIrradianceMap(convolutedEnvironmentMap->getCubeMap());
	shader->setLightColor(light.getColor());
	shader->setLightDirection(light.getLook());
	shader->setPrefilterMap(prefilterRenderTarget->getCubeMap());
	shader->setShadowMap(shadowMap);
	shader->setSSAOMap(ssaoMap);
	shader->setSkyBox(environmentMap->getCubeMap());
	shader->setWorldToLightSpaceMatrix(worldToLight);


	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	modelDrawer->draw(&screenSprite, Shaders::Pbr_Deferred_Lighting);
}