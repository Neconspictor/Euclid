#include <nex/pbr/PbrForward.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/pbr/PbrPass.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/texture/Sampler.hpp>
#include "PbrProbe.hpp"
#include "nex/light/Light.hpp"
#include "nex/drawing/StaticMeshDrawer.hpp"

using namespace glm;

using namespace std;

namespace nex {

	PbrForward::PbrForward(
		LightingPassFactory factory,
		GlobalIllumination* globalIllumination,
		CascadedShadow* cascadeShadow, DirLight* dirLight) :
	Pbr(globalIllumination, cascadeShadow, dirLight), 
		mFactory(std::move(factory)),
		mForwardShader(mFactory(cascadeShadow, globalIllumination))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);
	}

	PbrForward::~PbrForward()
	{
	}

	void PbrForward::reloadLightingShader(CascadedShadow* cascadedShadow)
	{
		mForwardShader = mFactory(cascadedShadow, mGlobalIllumination);
	}

	void PbrForward::configurePass(const Pass::Constants& constants)
	{
		mForwardShader->bind();
		mForwardShader->updateConstants(constants);
	}

	void PbrForward::updateLight(const DirLight& light, const Camera & camera)
	{
		mForwardShader->bind();
		mForwardShader->updateLight(light, camera);
	}

	PbrForwardPass* PbrForward::getPass()
	{
		return mForwardShader.get();
	}
}