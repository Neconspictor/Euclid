#include <nex/pbr/PBR_Deferred.hpp>
#include <nex/shader/SkyBoxShader.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/gui/Util.hpp>
#include <nex/shader/PBRShader.hpp>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/texture/GBuffer.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/RenderBackend.hpp>

using namespace glm;

using namespace std;

namespace nex {


	PBR_Deferred::PBR_Deferred(Texture* backgroundHDR, CascadedShadow* cascadeShadow) :
		PBR(backgroundHDR),
		mGeometryPass(make_unique<PBRShader_Deferred_Geometry>()),
		mLightPass(make_unique<PBRShader_Deferred_Lighting>(*cascadeShadow)),
		mCascadeShadow(cascadeShadow),
		mAmbientLightPower(1.0f)
	{
		vec2 dim = { 1.0, 1.0 };
		vec2 pos = { 0, 0 };

		// center
		pos.x = 0.5f * (1.0f - dim.x);
		pos.y = 0.5f * (1.0f - dim.y);

		screenSprite.setPosition(pos);
		screenSprite.setWidth(dim.x);
		screenSprite.setHeight(dim.y);

		mCascadeShadow->addCascadeChangeCallback([&](const CascadedShadow& cascade)->void
		{
			reloadLightingShader(cascade);
		});
	}

	void PBR_Deferred::drawGeometryScene(SceneNode * scene, const glm::mat4 & view, const glm::mat4 & projection)
	{
		static auto* renderBackend = RenderBackend::get();

		auto* stencilTest = renderBackend->getStencilTest();
		stencilTest->enableStencilTest(true);
		stencilTest->setCompareFunc(CompareFunction::ALWAYS, 1, 0xFF);
		//glStencilFunc(GL_ALWAYS, 1, 1);
		//glStencilMask(0xFF);
		stencilTest->setOperations(StencilTest::Operation::KEEP, StencilTest::Operation::KEEP, StencilTest::Operation::REPLACE);

		mGeometryPass->bind();
		mGeometryPass->setView(view);
		mGeometryPass->setProjection(projection);



		Sampler* sampler = TextureManager::get()->getDefaultImageSampler();

		for (int i = 0; i < 6; ++i)
		{
			sampler->bind(i);
		}

		StaticMeshDrawer::draw(scene, mGeometryPass.get());

		for (int i = 0; i < 6; ++i)
		{
			sampler->unbind(i);
		}

		stencilTest->enableStencilTest(false);
	}

	void PBR_Deferred::drawLighting(SceneNode * scene, PBR_GBuffer * gBuffer, Camera* camera,
		Texture* ssaoMap, const DirectionalLight & light)
	{

		static auto* renderBackend = RenderBackend::get();

		auto* stencilTest = renderBackend->getStencilTest();
		stencilTest->enableStencilTest(false);
		stencilTest->enableStencilTest(true);
		stencilTest->setCompareFunc(CompareFunction::EQUAL, 1, 1);


		mLightPass->bind();

		mLightPass->setAlbedoMap(gBuffer->getAlbedo());
		mLightPass->setAoMetalRoughnessMap(gBuffer->getAoMetalRoughness());
		mLightPass->setNormalEyeMap(gBuffer->getNormal());
		mLightPass->setDepthMap(gBuffer->getDepth());

		mLightPass->setBrdfLookupTexture(getBrdfLookupTexture());
		//shader->setGBuffer(gBuffer);
		mLightPass->setViewGPass(camera->getView());
		mLightPass->setInverseViewFromGPass(inverse(camera->getView()));
		mLightPass->setInverseProjMatrixFromGPass(inverse(camera->getPerspProjection()));
		mLightPass->setIrradianceMap(getConvolutedEnvironmentMap());
		mLightPass->setLightColor(light.getColor());
		mLightPass->setWorldLightDirection(light.getLook());

		vec4 lightEyeDirection = camera->getView() * vec4(light.getLook(), 0);
		mLightPass->setEyeLightDirection(vec3(lightEyeDirection));
		mLightPass->setLightPower(light.getLightPower());
		mLightPass->setAmbientLightPower(mAmbientLightPower);
		mLightPass->setShadowStrength(mCascadeShadow->getShadowStrength());

		mLightPass->setPrefilterMap(getPrefilteredEnvironmentMap());
		//shader->setShadowMap(shadowMap);
		mLightPass->setAOMap(ssaoMap);
		//TODO
		//shader->setSkyBox(environmentMap->getCubeMap());
		mLightPass->setWorldToLightSpaceMatrix(mCascadeShadow->getWorldToShadowSpace());
		mLightPass->setEyeToLightSpaceMatrix(mCascadeShadow->getWorldToShadowSpace()  * camera->getView());
		mLightPass->setCascadedData(mCascadeShadow->getCascadeData());
		mLightPass->setCascadedDepthMap(mCascadeShadow->getDepthTextureArray());


		StaticMeshDrawer::draw(&screenSprite, mLightPass.get());

		stencilTest->enableStencilTest(false);
	}

	void PBR_Deferred::drawSky(Camera* camera)
	{
		static auto* renderBackend = RenderBackend::get();

		auto* stencilTest = renderBackend->getStencilTest();
		stencilTest->enableStencilTest(true);
		stencilTest->setCompareFunc(CompareFunction::NOT_EQUAL, 1, 1);
		PBR::drawSky(camera->getPerspProjection(), camera->getView());
		stencilTest->enableStencilTest(false);
	}

	std::unique_ptr<PBR_GBuffer> PBR_Deferred::createMultipleRenderTarget(int width, int height)
	{
		return make_unique<PBR_GBuffer>(width, height);
	}

	float PBR_Deferred::getAmbientLightPower() const
	{
		return mAmbientLightPower;
	}

	void PBR_Deferred::setAmbientLightPower(float power)
	{
		mAmbientLightPower = power;
	}

	void PBR_Deferred::reloadLightingShader(const CascadedShadow& cascadedShadow)
	{
		mLightPass = make_unique<PBRShader_Deferred_Lighting>(cascadedShadow);
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
		nex::gui::Separator(2.0f);
		ImGui::PopID();
	}
}