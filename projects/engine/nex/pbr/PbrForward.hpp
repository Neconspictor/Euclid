#pragma once

#include "Pbr.hpp"
#include <nex/shader/Shader.hpp>
#include <nex/material/PbrMaterialLoader.hpp>

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
			CascadedShadow* cascadeShadow, DirLight* dirLight);

		virtual ~PbrForward();

		void reloadLightingShaders() override;

		void configurePass(const Constants& constants);

		void updateLight(const DirLight& light, const Camera& camera);

		std::shared_ptr<PbrShaderProvider> getShaderProvider();

	private:
		LightingPassFactory mFactory;
		std::shared_ptr<PbrShaderProvider> mProvider;
		std::unique_ptr<Sampler> mPointSampler;
	};
}
