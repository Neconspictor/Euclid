#pragma once

#include "Pbr.hpp"
#include <nex/shader/Shader.hpp>
#include <nex/shader/ShaderProvider.hpp>
#include <nex/pbr/PbrPass.hpp>
#include <nex/material/PbrMaterialLoader.hpp>

namespace nex
{
	class Camera;
	class CascadedShadow;
	struct DirLight;

	class PBR_GBuffer;
	class Sampler;
	class PbrProbe;
	class PbrDeferredGeometryShader;
	class PbrDeferredGeometryBonesShader;
	class PbrDeferredLightingPass;
	class PbrDeferredAmbientPass;
	

	class PbrDeferred : public Pbr {

	public:

		using LightingPassFactory = std::function<std::unique_ptr<PbrDeferredLightingPass>(CascadedShadow*, GlobalIllumination*)>;

		virtual ~PbrDeferred();

		PbrDeferred(
			std::unique_ptr<PbrDeferredGeometryShader> geometryShader,
			std::unique_ptr<PbrDeferredGeometryBonesShader> geometryBonesShader,
			LightingPassFactory lightingPassFactory,
			GlobalIllumination* globalIllumination,
			CascadedShadow* cascadeShadow, DirLight* dirLight);

		void drawAmbientLighting(PBR_GBuffer* gBuffer, Texture* depth, const Constants& constants);
		void drawLighting(PBR_GBuffer* gBuffer, Texture* irradiance, Texture* ambientReflection, const Constants& constants, const DirLight& light);

		std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

		std::shared_ptr<PbrShaderProvider> getGeometryShaderProvider();
		std::shared_ptr<PbrShaderProvider> getGeometryBonesShaderProvider();
		PbrDeferredLightingPass* getLightingPass();

		void reloadLightingShaders() override;

	private:

		LightingPassFactory mLightingPassFactory;
		std::shared_ptr<PbrShaderProvider> mGeometryShaderProvider;
		std::shared_ptr<PbrShaderProvider> mGeometryBonesShaderProvider;
		//std::unique_ptr<PbrDeferredGeometryShader> mGeometryShader;
		//std::unique_ptr<PbrDeferredGeometryBonesShader> mGeometryBonesShader;
		std::unique_ptr<PbrDeferredAmbientPass> mAmbientPass;
		std::unique_ptr<PbrDeferredLightingPass> mLightPass;
		std::unique_ptr<Sampler> mPointSampler;
	};
}