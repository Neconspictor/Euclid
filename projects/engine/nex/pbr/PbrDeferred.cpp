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

	PbrDeferred::PbrDeferred(
		std::unique_ptr<PbrDeferredGeometryPass> geometryPass,
		LightingPassFactory lightingPassFactory,
		GlobalIllumination* globalIllumination,
		CascadedShadow* cascadeShadow, 
		DirLight* dirLight) : Pbr(globalIllumination, cascadeShadow, dirLight),
		mLightingPassFactory(std::move(lightingPassFactory)),
		mGeometryPass(std::move(geometryPass)),
		mLightPass(mLightingPassFactory(cascadeShadow, globalIllumination))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);
	}

	void PbrDeferred::configureGeometryPass(const Camera& camera)
	{
		mGeometryPass->bind();
		mGeometryPass->updateConstants(camera);
	}

	void PbrDeferred::drawLighting(PBR_GBuffer * gBuffer, const Camera& camera, const DirLight& light)
	{
		mLightPass->bind();
		mLightPass->updateConstants(camera);
		mLightPass->updateLight(light, camera);


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
		mLightPass = mLightingPassFactory(cascadedShadow, mGlobalIllumination);
	}
}