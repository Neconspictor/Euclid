#include <nex/pbr/PbrForward.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/shader/PbrPass.hpp>

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

	PbrForward::PbrForward(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe) :
	Pbr(ambientLight, cascadeShadow, dirLight, probe, nullptr), mForwardShader(std::make_unique<PbrForwardPass>(*cascadeShadow))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);

		// set the active submesh pass
		setSelected(mForwardShader.get());
	}

	void PbrForward::reloadLightingShader(const CascadedShadow& cascadedShadow)
	{
		mForwardShader = std::make_unique<PbrForwardPass>(cascadedShadow);
		setSelected(mForwardShader.get());
	}


	void PbrForward::drawLighting(SceneNode * scene,
		Camera* camera)
	{
		mForwardShader->bind();

		static Sampler* sampler = TextureManager::get()->getDefaultImageSampler();

		for (int i = 0; i < 5; ++i)
		{
			sampler->bind(i);
		}
		
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
		mForwardShader->setAmbientLightPower(mAmbientLight->getPower());
		mForwardShader->setShadowStrength(mCascadeShadow->getShadowStrength());

		mForwardShader->setPrefilterMap(mProbe->getPrefilteredEnvironmentMap());

		mForwardShader->PbrCommonGeometryPass::setNearFarPlane(camera->getNearFarPlaneViewSpace(Perspective));
		mForwardShader->PbrCommonLightingPass::setNearFarPlane(camera->getNearFarPlaneViewSpace(Perspective));

		//TODO
		mForwardShader->setCascadedData(mCascadeShadow->getCascadeBuffer());
		mForwardShader->setCascadedDepthMap(mCascadeShadow->getDepthTextureArray());

		StaticMeshDrawer::draw(scene, mForwardShader.get());

		for (int i = 0; i < 5; ++i)
		{
			sampler->unbind(i);
		}
	}
}