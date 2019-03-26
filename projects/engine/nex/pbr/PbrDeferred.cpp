#include <nex/pbr/PbrDeferred.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/shader/PBRShader.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/texture/GBuffer.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include "nex/light/Light.hpp"
#include "PbrProbe.hpp"

namespace nex {

	PbrDeferred::PbrDeferred(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight,
		PbrProbe* probe) : Pbr(ambientLight, cascadeShadow, dirLight, probe),
		mGeometryPass(std::make_unique<PBRShader_Deferred_Geometry>()),
		mLightPass(std::make_unique<PBRShader_Deferred_Lighting>(*cascadeShadow))
	{
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

		glm::vec4 lightEyeDirection = camera->getView() * glm::vec4(mLight->getDirection(), 0);
		mLightPass->setEyeLightDirection(glm::vec3(lightEyeDirection));
		mLightPass->setLightPower(mLight->getLightPower());
		mLightPass->setAmbientLightPower(mAmbientLight->getPower());
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

	std::unique_ptr<PBR_GBuffer> PbrDeferred::createMultipleRenderTarget(int width, int height)
	{
		return std::make_unique<PBR_GBuffer>(width, height);
	}

	void PbrDeferred::reloadLightingShader(const CascadedShadow& cascadedShadow)
	{
		mLightPass = std::make_unique<PBRShader_Deferred_Lighting>(cascadedShadow);
	}
}