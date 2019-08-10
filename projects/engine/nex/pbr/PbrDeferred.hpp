#pragma once

#include "Pbr.hpp"

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

	class PbrDeferred : public Pbr {

	public:

		using LightingPassFactory = std::function<std::unique_ptr<PbrDeferredLightingPass>(CascadedShadow*, GlobalIllumination*)>;

		virtual ~PbrDeferred();

		PbrDeferred(
			std::unique_ptr<PbrDeferredGeometryPass> geometryPass,
			LightingPassFactory lightingPassFactory,
			GlobalIllumination* globalIllumination,
			CascadedShadow* cascadeShadow, DirLight* dirLight);

		void configureGeometryPass(const Camera& camera);

		void drawLighting(PBR_GBuffer* gBuffer, const Camera& camera, const DirLight& light);

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