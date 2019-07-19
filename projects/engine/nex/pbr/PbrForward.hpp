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

		using LightingPassFactory = std::function<std::unique_ptr<PbrForwardPass>(CascadedShadow*, GlobalIllumination*)>;

		PbrForward(
			LightingPassFactory factory,
			GlobalIllumination* globalIllumination, 
			CascadedShadow* cascadeShadow, DirectionalLight* dirLight);

		virtual ~PbrForward();

		void reloadLightingShader(CascadedShadow* cascadedShadow) override;

		void configurePass(const Camera& camera);

		void updateLight(const DirectionalLight & light, const Camera& camera);

		PbrForwardPass* getPass();

	private:
		LightingPassFactory mFactory;
		std::unique_ptr<PbrForwardPass> mForwardShader;
		std::unique_ptr<Sampler> mPointSampler;
	};
}
