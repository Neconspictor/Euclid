#include <nex/pbr/PbrForward.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/shader/PbrPass.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/texture/Sampler.hpp>
#include "PbrProbe.hpp"
#include "nex/light/Light.hpp"
#include "nex/drawing/StaticMeshDrawer.hpp"

using namespace glm;

using namespace std;

namespace nex {

	PbrForward::PbrForward(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe) :
	Pbr(ambientLight, cascadeShadow, dirLight, probe, nullptr), mForwardShader(std::make_unique<PbrForwardPass>(cascadeShadow))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);

		// set the active submesh pass
		setSelected(mForwardShader.get());

		mForwardShader->setProbe(mProbe);
		mForwardShader->setAmbientLight(mAmbientLight);
		mForwardShader->setDirLight(mLight);
	}

	void PbrForward::reloadLightingShader(CascadedShadow* cascadedShadow)
	{
		mForwardShader = std::make_unique<PbrForwardPass>(cascadedShadow);
		setSelected(mForwardShader.get());
	}

	void PbrForward::configureSubMeshPass(Camera* camera)
	{
		mForwardShader->bind();
		mForwardShader->setProbe(mProbe);
		mForwardShader->updateConstants(camera);
	}
}