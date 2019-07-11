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

	PbrForward::PbrForward(AmbientLight* ambientLight, CascadedShadow* cascadeShadow, DirectionalLight* dirLight, PbrProbe* probe) :
	Pbr(ambientLight, cascadeShadow, dirLight, probe), mForwardShader(std::make_unique<PbrForwardPass>(cascadeShadow))
	{
		SamplerDesc desc;
		desc.minFilter = desc.magFilter = TextureFilter::Linear;
		desc.wrapR = desc.wrapS = desc.wrapT = TextureUVTechnique::ClampToEdge;
		desc.maxAnisotropy = 1.0f;
		mPointSampler = std::make_unique<Sampler>(desc);

		mForwardShader->setProbe(mProbe);
		mForwardShader->setAmbientLight(mAmbientLight);
		mForwardShader->setDirLight(mLight);
	}

	PbrForward::~PbrForward()
	{
	}

	void PbrForward::reloadLightingShader(CascadedShadow* cascadedShadow)
	{
		mForwardShader = std::make_unique<PbrForwardPass>(cascadedShadow);
		mForwardShader->setProbe(mProbe);
		mForwardShader->setAmbientLight(mAmbientLight);
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
	void PbrForward::setProbe(PbrProbe * probe)
	{
		mProbe = probe;
		mForwardShader->setProbe(probe);
	}
}
