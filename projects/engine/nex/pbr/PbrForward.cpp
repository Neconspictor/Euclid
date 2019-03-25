#include <nex/pbr/PbrForward.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/gui/Util.hpp>
#include <nex/shader/PBRShader.hpp>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include "PbrProbe.hpp"
#include "nex/light/Light.hpp"
#include "nex/drawing/StaticMeshDrawer.hpp"

using namespace glm;

using namespace std;

namespace nex {

	PbrForward::PbrForward(PbrProbe* probe, DirectionalLight* light, CascadedShadow* cascadeShadow) :
		mProbe(probe),
		mCascadeShadow(cascadeShadow),
		mAmbientLightPower(1.0f),
		mLight(light),
		mForwardShader(std::make_unique<PBRShader>(*cascadeShadow))
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


	void PbrForward::drawSky(Camera* camera)
	{
		mProbe->drawSky(camera->getPerspProjection(), camera->getView());
	}

	float PbrForward::getAmbientLightPower() const
	{
		return mAmbientLightPower;
	}

	DirectionalLight* PbrForward::getDirLight()
	{
		return mLight;
	}

	void PbrForward::setDirLight(DirectionalLight* light)
	{
		mLight = light;
	}

	void PbrForward::setAmbientLightPower(float power)
	{
		mAmbientLightPower = power;
	}

	void PbrForward::reloadLightingShader(const CascadedShadow& cascadedShadow)
	{
		mForwardShader = std::make_unique<PBRShader>(cascadedShadow);
	}


	void PbrForward::drawLighting(SceneNode * scene,
		Camera* camera)
	{

		mForwardShader->bind();

		Sampler* sampler = TextureManager::get()->getDefaultImageSampler();

		for (int i = 0; i < 5; ++i)
		{
			sampler->bind(i);
		}

		sampler->unbind(8);
		
		//TODO update!!!
		mForwardShader->setInverseViewMatrix(inverse(camera->getView()));
		mForwardShader->setView(camera->getView());
		mForwardShader->setProjection(camera->getPerspProjection());


		mForwardShader->setBrdfLookupTexture(mProbe->getBrdfLookupTexture());
		mForwardShader->setInverseViewMatrix(inverse(camera->getView()));
		mForwardShader->setIrradianceMap(mProbe->getConvolutedEnvironmentMap());
		mForwardShader->setLightColor(mLight->getColor());

		vec4 lightEyeDirection = camera->getView() * vec4(mLight->getDirection(), 0);
		mForwardShader->setEyeLightDirection(vec3(lightEyeDirection));
		mForwardShader->setLightPower(mLight->getLightPower());
		mForwardShader->setAmbientLightPower(mAmbientLightPower);
		mForwardShader->setShadowStrength(mCascadeShadow->getShadowStrength());

		mForwardShader->setPrefilterMap(mProbe->getPrefilteredEnvironmentMap());

		mForwardShader->pbr::CommonGeometryMaterial::setNearFarPlane(camera->getNearFarPlaneViewSpace(Perspective));
		mForwardShader->pbr::CommonLightingMaterial::setNearFarPlane(camera->getNearFarPlaneViewSpace(Perspective));

		//TODO
		mForwardShader->setCascadedData(mCascadeShadow->getCascadeBuffer());
		mForwardShader->setCascadedDepthMap(mCascadeShadow->getDepthTextureArray());

		StaticMeshDrawer::draw(scene, mForwardShader.get());

		for (int i = 0; i < 5; ++i)
		{
			sampler->unbind(i);
		}
	}


	PbrForward_ConfigurationView::PbrForward_ConfigurationView(PbrForward* pbr) : mPbr(pbr)
	{
	}

	void PbrForward_ConfigurationView::drawSelf()
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

	void PbrForward_ConfigurationView::drawLightSphericalDirection()
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