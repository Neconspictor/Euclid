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
			LightingPassFactory staticShaderfactory,
			LightingPassFactory boneShaderFactory,
			GlobalIllumination* globalIllumination, 
			CascadedShadow* cascadeShadow, DirLight* dirLight);

		virtual ~PbrForward();

		void reloadLightingShaders() override;

		std::shared_ptr<PbrShaderProvider> getShaderProvider();
		std::shared_ptr<PbrShaderProvider> getBoneShaderProvider();

	private:
		LightingPassFactory mFactory;
		LightingPassFactory mBoneFactory;
		std::shared_ptr<PbrShaderProvider> mProvider;
		std::shared_ptr<PbrShaderProvider> mBoneProvider;
		std::unique_ptr<Sampler> mPointSampler;
	};
}
