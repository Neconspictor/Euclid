#include <nex/pbr/PbrDeferred.hpp>
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
#include <nex/texture/Sampler.hpp>
#include "nex/light/Light.hpp"
#include "PbrProbe.hpp"

using namespace glm;

using namespace std;

namespace nex {


	PbrDeferred::PbrDeferred(PbrProbe* probe, DirectionalLight* dirLight, CascadedShadow* cascadeShadow) :
		mProbe(probe),
		mGeometryPass(make_unique<PBRShader_Deferred_Geometry>()),
		mLightPass(make_unique<PBRShader_Deferred_Lighting>(*cascadeShadow)),
		mCascadeShadow(cascadeShadow),
		mAmbientLightPower(1.0f),
		mLight(dirLight)
	{


		mCascadeShadow->addCascadeChangeCallback([&](const CascadedShadow& cascade)->void
		{
			reloadLightingShader(cascade);
		});


		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);
	}


	void PbrDeferred::drawGeometryScene(SceneNode * scene, Camera* camera)
	{
		const auto & view = camera->getView();
		const auto & projection = camera->getPerspProjection();
		mGeometryPass->bind();
		mGeometryPass->setView(view);
		mGeometryPass->setProjection(projection);
		mGeometryPass->setNearFarPlane(camera->getNearFarPlaneViewSpace(Perspective));

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
	}

	void PbrDeferred::drawLighting(SceneNode * scene, PBR_GBuffer * gBuffer, Camera* camera)
	{
		mLightPass->bind();

		Sampler* sampler = TextureManager::get()->getDefaultImageSampler();

		for (int i = 0; i < 4; ++i)
		{
			//mPointSampler->bind(i);
		}

		//sampler->bind(4);
		//sampler->bind(5);


		mLightPass->setAlbedoMap(gBuffer->getAlbedo());
		mLightPass->setAoMetalRoughnessMap(gBuffer->getAoMetalRoughness());
		mLightPass->setNormalEyeMap(gBuffer->getNormal());
		mLightPass->setNormalizedViewSpaceZMap(gBuffer->getNormalizedViewSpaceZ());

		mLightPass->setBrdfLookupTexture(mProbe->getBrdfLookupTexture());
		mLightPass->setInverseViewMatrix(inverse(camera->getView()));
		mLightPass->setInverseProjMatrixFromGPass(inverse(camera->getPerspProjection()));
		mLightPass->setIrradianceMap(mProbe->getConvolutedEnvironmentMap());
		mLightPass->setLightColor(mLight->getColor());

		vec4 lightEyeDirection = camera->getView() * vec4(mLight->getDirection(), 0);
		mLightPass->setEyeLightDirection(vec3(lightEyeDirection));
		mLightPass->setLightPower(mLight->getLightPower());
		mLightPass->setAmbientLightPower(mAmbientLightPower);
		mLightPass->setShadowStrength(mCascadeShadow->getShadowStrength());

		mLightPass->setPrefilterMap(mProbe->getPrefilteredEnvironmentMap());

		mLightPass->setNearFarPlane(camera->getNearFarPlaneViewSpace(Perspective));

		//TODO
		mLightPass->setCascadedData(mCascadeShadow->getCascadeBuffer());
		mLightPass->setCascadedDepthMap(mCascadeShadow->getDepthTextureArray());


		StaticMeshDrawer::draw(Sprite::getScreenSprite(), mLightPass.get());


		for (int i = 0; i < 4; ++i)
		{
			//mPointSampler->unbind(i);
			//sampler->unbind(i);
		}

		//sampler->bind(4);
		//sampler->bind(5);
	}

	void PbrDeferred::drawSky(Camera* camera)
	{
		mProbe->drawSky(camera->getPerspProjection(), camera->getView());
	}

	std::unique_ptr<PBR_GBuffer> PbrDeferred::createMultipleRenderTarget(int width, int height)
	{
		return make_unique<PBR_GBuffer>(width, height);
	}

	float PbrDeferred::getAmbientLightPower() const
	{
		return mAmbientLightPower;
	}

	DirectionalLight* PbrDeferred::getDirLight()
	{
		return mLight;
	}

	void PbrDeferred::setDirLight(DirectionalLight* light)
	{
		mLight = light;
	}

	void PbrDeferred::setAmbientLightPower(float power)
	{
		mAmbientLightPower = power;
	}

	void PbrDeferred::reloadLightingShader(const CascadedShadow& cascadedShadow)
	{
		mLightPass = make_unique<PBRShader_Deferred_Lighting>(cascadedShadow);
	}


	PBR_Deferred_ConfigurationView::PBR_Deferred_ConfigurationView(PbrDeferred* pbr) : mPbr(pbr)
	{
	}

	void PBR_Deferred_ConfigurationView::drawSelf()
	{
		ImGui::PushID(m_id.c_str());
		//m_pbr
		ImGui::LabelText("", "PBR:");


		auto* dirLight = mPbr->getDirLight();

		glm::vec3 lightColor = dirLight->getColor();
		float dirLightPower = dirLight->getLightPower();
		

		if (ImGui::InputScalarN("Directional Light Color", ImGuiDataType_Float, &lightColor, 3))
		{
			lightColor = clamp(lightColor, glm::vec3(0), glm::vec3(1));
			dirLight->setColor(lightColor);
		}
		
		if (ImGui::DragFloat("Directional Light Power", &dirLightPower, 0.1f, 0.0f, 10.0f))
		{
			dirLight->setPower(dirLightPower);
		}

		drawLightSphericalDirection();


		float ambientLightPower = mPbr->getAmbientLightPower();

		if(ImGui::DragFloat("Amblient Light Power", &ambientLightPower, 0.1f, 0.0f, 10.0f))
		{
			mPbr->setAmbientLightPower(ambientLightPower);
		}


		nex::gui::Separator(2.0f);
		ImGui::PopID();
	}

	void PBR_Deferred_ConfigurationView::drawLightSphericalDirection()
	{
		auto* dirLight = mPbr->getDirLight();
		glm::vec3 lightDirection = normalize(dirLight->getDirection());

		static SphericalCoordinate sphericalCoordinate = SphericalCoordinate::convert(-lightDirection);

		float temp[2] = {sphericalCoordinate.polar, sphericalCoordinate.azimuth};

		if (ImGui::DragFloat2("Light position (spherical coordinates)", temp, 0.05f))
		{
			//temp = clamp(temp, glm::vec2(-1.0f), glm::vec2(1.0f));
			//temp[0] = std::clamp<float>(temp[0], 0, M_PI);
			//temp[0] = std::clamp<float>(temp[0], -2 * M_PI, 2 * M_PI);
			//temp[1] = std::clamp<float>(temp[1], -2*M_PI, 2*M_PI);
			sphericalCoordinate.polar = temp[0];
			sphericalCoordinate.azimuth = temp[1];
			lightDirection = -SphericalCoordinate::cartesian(sphericalCoordinate);
			lightDirection = clamp(lightDirection, glm::vec3(-1), glm::vec3(1));
			dirLight->setDirection(lightDirection);
		}

	}
}
