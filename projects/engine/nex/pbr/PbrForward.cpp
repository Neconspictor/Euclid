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
		CascadedShadow* cascadeShadow, DirectionalLight* dirLight) :
	Pbr(globalIllumination, cascadeShadow, dirLight), 
		mFactory(std::move(factory)),
		mForwardShader(mFactory(cascadeShadow, globalIllumination))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);

		mForwardShader->setDirLight(mLight);
	}

	PbrForward::~PbrForward()
	{
	}

	void PbrForward::reloadLightingShader(CascadedShadow* cascadedShadow)
	{
		mForwardShader = mFactory(cascadedShadow, mGlobalIllumination);
		mForwardShader->setDirLight(mLight);
	}

	void PbrForward::configurePass(Camera* camera)
	{
		mForwardShader->bind();
		mForwardShader->updateConstants(camera);
	}

	PbrForwardPass* PbrForward::getPass()
	{
		return mForwardShader.get();
	}
}