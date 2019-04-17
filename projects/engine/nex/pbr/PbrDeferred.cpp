#include <nex/pbr/PbrDeferred.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/shader/PbrPass.hpp>

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
		PbrProbe* probe) : Pbr(ambientLight, cascadeShadow, dirLight, probe, nullptr),
		mGeometryPass(std::make_unique<PbrDeferredGeometryPass>()),
		mLightPass(std::make_unique<PbrDeferredLightingPass>(cascadeShadow))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);

		// set the active submesh pass
		setSelected(mGeometryPass.get());

		mLightPass->setProbe(mProbe);
		mLightPass->setAmbientLight(mAmbientLight);
		mLightPass->setDirLight(mLight);
	}

	void PbrDeferred::configureSubMeshPass(Camera* camera)
	{
		mGeometryPass->bind();
		mGeometryPass->updateConstants(camera);
	}

	void PbrDeferred::drawLighting(PBR_GBuffer * gBuffer, Camera* camera)
	{
		mLightPass->bind();

		mLightPass->setProbe(mProbe);
		mLightPass->updateConstants(camera);


		mLightPass->setAlbedoMap(gBuffer->getAlbedo());
		mLightPass->setAoMetalRoughnessMap(gBuffer->getAoMetalRoughness());
		mLightPass->setNormalEyeMap(gBuffer->getNormal());
		mLightPass->setNormalizedViewSpaceZMap(gBuffer->getNormalizedViewSpaceZ());

		static RenderState state;

		StaticMeshDrawer::draw(state, Sprite::getScreenSprite(), mLightPass.get());
	}

	std::unique_ptr<PBR_GBuffer> PbrDeferred::createMultipleRenderTarget(int width, int height)
	{
		return std::make_unique<PBR_GBuffer>(width, height);
	}

	void PbrDeferred::reloadLightingShader(CascadedShadow* cascadedShadow)
	{
		mLightPass = std::make_unique<PbrDeferredLightingPass>(cascadedShadow);

		mLightPass->setProbe(mProbe);
		mLightPass->setAmbientLight(mAmbientLight);
		mLightPass->setDirLight(mLight);
	}
}