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
	class PBRShader_Deferred_Geometry;
	class PBRShader_Deferred_Lighting;

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
		std::unique_ptr<PBRShader_Deferred_Geometry> mGeometryPass;
		std::unique_ptr<PBRShader_Deferred_Lighting> mLightPass;
		std::unique_ptr<Sampler> mPointSampler;
	};
}