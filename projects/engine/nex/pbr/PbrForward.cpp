#include <nex/pbr/PbrForward.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/pbr/PbrPass.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sampler.hpp>
#include <nex/GI/Probe.hpp>
#include "nex/light/Light.hpp"
#include "nex/renderer/Drawer.hpp"
#include <nex/material/PbrMaterialLoader.hpp>
#include <nex/shader/ShaderProvider.hpp>

using namespace glm;

using namespace std;

namespace nex {

	PbrForward::PbrForward(
		LightingPassFactory factory,
		LightingPassFactory boneFactory,
		GlobalIllumination* globalIllumination,
		CascadedShadow* cascadeShadow, DirLight* dirLight) :
	Pbr(globalIllumination, cascadeShadow, dirLight), 
		mFactory(std::move(factory)),
		mBoneFactory(std::move(boneFactory)),
		mProvider(std::make_shared<PbrShaderProvider>(nullptr)),
		mBoneProvider(std::make_shared<PbrShaderProvider>(nullptr))
	{
		reloadLightingShaders();
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TexFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = UVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);
	}

	PbrForward::~PbrForward() = default;

	void PbrForward::reloadLightingShaders()
	{
		mProvider->setOwningShader(mFactory(mCascadedShadow, mGlobalIllumination));
		mBoneProvider->setOwningShader(mBoneFactory(mCascadedShadow, mGlobalIllumination));
	}

	std::shared_ptr<PbrShaderProvider> PbrForward::getShaderProvider()
	{
		return mProvider;
	}
	std::shared_ptr<PbrShaderProvider> PbrForward::getBoneShaderProvider()
	{
		return mBoneProvider;
	}
}