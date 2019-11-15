#pragma once

#include "Pbr.hpp"
#include <nex/shader/Shader.hpp>

namespace nex
{
	class Camera;
	class CascadedShadow;
	struct DirLight;

	class PBR_GBuffer;
	class Sampler;
	class PbrProbe;
	class PbrDeferredGeometryPass;
	class PbrDeferredLightingPass;
	class PbrDeferredAmbientPass;

	class PbrDeferred : public Pbr {

	public:

		using LightingPassFactory = std::function<std::unique_ptr<PbrDeferredLightingPass>(CascadedShadow*, GlobalIllumination*)>;

		virtual ~PbrDeferred();

		PbrDeferred(
			std::unique_ptr<PbrDeferredGeometryPass> geometryPass,
			LightingPassFactory lightingPassFactory,
			GlobalIllumination* globalIllumination,
			CascadedShadow* cascadeShadow, DirLight* dirLight);

		void configureGeometryPass(const Shader::Constants& constants);

		void drawAmbientLighting(PBR_GBuffer* gBuffer, Texture* depth, const Shader::Constants& constants);
		void drawLighting(PBR_GBuffer* gBuffer, Texture* irradiance, Texture* ambientReflection, const Shader::Constants& constants, const DirLight& light);

		std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

		PbrDeferredGeometryPass* getGeometryPass();
		PbrDeferredLightingPass* getLightingPass();

		void reloadLightingShaders() override;

	private:
		LightingPassFactory mLightingPassFactory;
		std::unique_ptr<PbrDeferredGeometryPass> mGeometryPass;
		std::unique_ptr<PbrDeferredAmbientPass> mAmbientPass;
		std::unique_ptr<PbrDeferredLightingPass> mLightPass;
		std::unique_ptr<Sampler> mPointSampler;
	};
}