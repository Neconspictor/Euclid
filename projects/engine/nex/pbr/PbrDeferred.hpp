#pragma once

#include "Pbr.hpp"

namespace nex
{
	class Camera;
	class CascadedShadow;
	class DirectionalLight;
	class SceneNode;

	class PBR_GBuffer;
	class Sampler;
	class PbrProbe;
	class PbrDeferredGeometryPass;
	class PbrDeferredLightingPass;

	class PbrDeferred : public Pbr {

	public:
		PbrDeferred(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe);

		void drawGeometryScene(SceneNode * scene, Camera* camera);

		void drawLighting(SceneNode * scene,
			PBR_GBuffer* gBuffer,
			Camera* camera);

		std::unique_ptr<PBR_GBuffer> createMultipleRenderTarget(int width, int height);

		void reloadLightingShader(const CascadedShadow& cascadedShadow) override;

	private:
		std::unique_ptr<PbrDeferredGeometryPass> mGeometryPass;
		std::unique_ptr<PbrDeferredLightingPass> mLightPass;
		std::unique_ptr<Sampler> mPointSampler;
	};
}