#include <nex/pbr/PbrDeferred.hpp>
#include <nex/pbr/PbrPass.hpp>

#include <nex/texture/GBuffer.hpp>
#include <nex/drawing/MeshDrawer.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sampler.hpp>
#include "nex/light/Light.hpp"
#include "PbrProbe.hpp"

namespace nex {
	PbrDeferred::~PbrDeferred()
	{
	}

	PbrDeferred::PbrDeferred(
		std::unique_ptr<PbrDeferredGeometryShader> geometryShader,
		std::unique_ptr<PbrDeferredGeometryBonesShader> geometryBonesShader,
		LightingPassFactory lightingPassFactory,
		GlobalIllumination* globalIllumination,
		CascadedShadow* cascadeShadow,
		DirLight* dirLight) : Pbr(globalIllumination, cascadeShadow, dirLight),
		mLightingPassFactory(std::move(lightingPassFactory)),
		mGeometryShader(std::move(geometryShader)),
		mGeometryBonesShader(std::move(geometryBonesShader))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TexFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = UVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);

		PbrDeferred::reloadLightingShaders();
	}

	void PbrDeferred::drawAmbientLighting(PBR_GBuffer* gBuffer, Texture* depth, const Constants& constants)
	{
		if (!mAmbientPass) return;

		mAmbientPass->bind();

		mAmbientPass->setAlbedoMap(gBuffer->getAlbedo());
		mAmbientPass->setAoMetalRoughnessMap(gBuffer->getAoMetalRoughness());
		mAmbientPass->setNormalEyeMap(gBuffer->getNormal());
		mAmbientPass->setDepthMap(depth);

		mAmbientPass->updateConstants(constants);

		static RenderState state;
		state.doDepthTest = false;
		state.doDepthWrite = false;

		MeshDrawer::drawFullscreenTriangle(state, mAmbientPass.get());
	}

	void PbrDeferred::drawLighting(PBR_GBuffer * gBuffer, Texture* irradiance, Texture* ambientReflection, const Constants& constants, const DirLight& light)
	{
		mLightPass->bind();
		mLightPass->updateConstants(constants);
		mLightPass->updateLight(light, *constants.camera);


		mLightPass->setAlbedoMap(gBuffer->getAlbedo());
		mLightPass->setAoMetalRoughnessMap(gBuffer->getAoMetalRoughness());
		mLightPass->setNormalEyeMap(gBuffer->getNormal());
		mLightPass->setNormalizedViewSpaceZMap(gBuffer->getDepth());
		mLightPass->setIrradianceOutMap(irradiance);
		mLightPass->setAmbientReflectionOutMap(ambientReflection);

		static RenderState state;
		state.doDepthTest = false;
		state.doDepthWrite = false;
		state.doBlend = true;
		state.blendDesc.operation = BlendOperation::ADD;
		state.blendDesc.source = BlendFunc::ONE;
		state.blendDesc.destination = BlendFunc::ONE;

		MeshDrawer::drawFullscreenTriangle(state, mLightPass.get());
	}

	std::unique_ptr<PBR_GBuffer> PbrDeferred::createMultipleRenderTarget(int width, int height)
	{
		return std::make_unique<PBR_GBuffer>(width, height);
	}

	PbrDeferredGeometryShader* PbrDeferred::getGeometryShader()
	{
		return mGeometryShader.get();
	}

	PbrDeferredGeometryBonesShader* PbrDeferred::getGeometryBonesShader()
	{
		return mGeometryBonesShader.get();
	}

	PbrDeferredLightingPass* PbrDeferred::getLightingPass()
	{
		return mLightPass.get();
	}

	void PbrDeferred::reloadLightingShaders()
	{
		mLightPass = mLightingPassFactory(mCascadedShadow, mGlobalIllumination);

		if (mGlobalIllumination) {
			mAmbientPass = std::make_unique<PbrDeferredAmbientPass>(mGlobalIllumination);
		}
		else {
			mAmbientPass = nullptr;
		}
	}
}