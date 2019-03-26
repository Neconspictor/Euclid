#pragma once

#include "Pbr.hpp"

namespace nex
{
	class Camera;
	class SceneNode;
	class PBRShader;
	class Sampler;

	class PbrForward : public Pbr {

	public:
		PbrForward(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe);

		void drawLighting(SceneNode * scene,
			Camera* camera);

		void reloadLightingShader(const CascadedShadow& cascadedShadow) override;

	private:
		std::unique_ptr<PBRShader> mForwardShader;
		std::unique_ptr<Sampler> mPointSampler;
	};
}
