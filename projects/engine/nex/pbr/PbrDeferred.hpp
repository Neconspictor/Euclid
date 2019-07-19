#pragma once

#include "Pbr.hpp"

namespace nex
{
	class Camera;
	class CascadedShadow;
	class DirectionalLight;

	class PBR_GBuffer;
	class Sampler;
	class PbrProbe;
	class PbrDeferredGeometryPass;
	class PbrDeferredLightingPass;

	class PbrDeferred : public Pbr {

	public:

		using LightingPassFactory = std::function<std::unique_ptr<PbrDeferredLightingPass>(CascadedShadow*, GlobalIllumination*)>;

		virtual ~PbrDeferred();

		PbrDeferred(
			std::unique_ptr<PbrDeferredGeometryPass> geometryPass,
			LightingPassFactory lightingPassFactory,
			GlobalIllumination* globalIllumination,
			CascadedShadow* cascadeShadow, DirectionalLight* dirLight);

		void configureGeometryPass(Camera* camera);

		void drawLighting(PBR_GBuffer* gBuffer, Camera* camera);

		std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

		PbrDeferredGeometryPass* getGeometryPass();
		PbrDeferredLightingPass* getLightingPass();

		void reloadLightingShader(CascadedShadow* cascadedShadow) override;

	private:
		LightingPassFactory mLightingPassFactory;
		std::unique_ptr<PbrDeferredGeometryPass> mGeometryPass;
		std::unique_ptr<PbrDeferredLightingPass> mLightPass;
		std::unique_ptr<Sampler> mPointSampler;
	};
}