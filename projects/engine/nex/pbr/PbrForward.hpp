#pragma once

#include "Pbr.hpp"

namespace nex
{
	class Camera;
	class SceneNode;
	class PbrForwardPass;
	class Sampler;

	class PbrForward : public Pbr {

	public:
		PbrForward(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe);

		void drawLighting(SceneNode * scene,
			Camera* camera);

		void reloadLightingShader(const CascadedShadow& cascadedShadow) override;

	private:
		std::unique_ptr<PbrForwardPass> mForwardShader;
		std::unique_ptr<Sampler> mPointSampler;
	};
}
