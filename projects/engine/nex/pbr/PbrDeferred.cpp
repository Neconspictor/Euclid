#include <nex/pbr/PbrDeferred.hpp>
#include <nex/pbr/PbrPass.hpp>

#include <nex/texture/GBuffer.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sampler.hpp>
#include "nex/light/Light.hpp"
#include "PbrProbe.hpp"

namespace nex {
	PbrDeferred::~PbrDeferred()
	{
	}

	PbrDeferred::PbrDeferred(GlobalIllumination* globalIllumination,
		CascadedShadow* cascadeShadow, DirectionalLight* dirLight) : Pbr(globalIllumination, cascadeShadow, dirLight),
		mGeometryPass(std::make_unique<PbrDeferredGeometryPass>()),
		mLightPass(std::make_unique<PbrDeferredLightingPass>(globalIllumination, cascadeShadow))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);

		mLightPass->setDirLight(mLight);
	}

	void PbrDeferred::configureGeometryPass(Camera* camera)
	{
		mGeometryPass->bind();
		mGeometryPass->updateConstants(camera);
	}

	void PbrDeferred::drawLighting(PBR_GBuffer * gBuffer, Camera* camera)
	{
		mLightPass->bind();
		mLightPass->updateConstants(camera);


		mLightPass->setAlbedoMap(gBuffer->getAlbedo());
		mLightPass->setAoMetalRoughnessMap(gBuffer->getAoMetalRoughness());
		mLightPass->setNormalEyeMap(gBuffer->getNormal());
		mLightPass->setNormalizedViewSpaceZMap(gBuffer->getNormalizedViewSpaceZ());

		static RenderState state;
		state.doDepthTest = false;
		state.doDepthWrite = false;

		StaticMeshDrawer::drawFullscreenTriangle(state, mLightPass.get());
	}

	std::unique_ptr<PBR_GBuffer> PbrDeferred::createMultipleRenderTarget(int width, int height)
	{
		return std::make_unique<PBR_GBuffer>(width, height);
	}

	PbrDeferredGeometryPass* PbrDeferred::getGeometryPass()
	{
		return mGeometryPass.get();
	}

	PbrDeferredLightingPass* PbrDeferred::getLightingPass()
	{
		return mLightPass.get();
	}

	void PbrDeferred::reloadLightingShader(CascadedShadow* cascadedShadow)
	{
		mLightPass = std::make_unique<PbrDeferredLightingPass>(mGlobalIllumination, cascadedShadow);

		mLightPass->setDirLight(mLight);
	}
}