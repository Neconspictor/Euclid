#include <nex/GI/GlobalIllumination.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/resource/FileSystem.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <glm/glm.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <list>
#include <nex/scene/Scene.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/pbr/PbrPass.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/pbr/PbrDeferred.hpp>
#include <nex/pbr/PbrForward.hpp>
#include <nex/renderer/Renderer.hpp>
#include <nex/cluster/Cluster.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/effects/EffectLibrary.hpp>
#include <nex/renderer/Drawer.hpp>
#include <nex/GI/IrradianceSphereHullDrawPass.hpp>
#include <nex/shadow/ShadowMap.hpp>
#include <nex/shader/ShaderProvider.hpp>


nex::GlobalIllumination::GlobalIllumination(unsigned reflectionMapSize, 
	unsigned irradianceArraySize, 
	unsigned reflectionArraySize) :
	mAmbientLightPower(1.0f),
	mProbeManager(reflectionMapSize, irradianceArraySize, reflectionArraySize),
	mVoxelConeTracer()
{
}

float nex::GlobalIllumination::getAmbientPower() const
{
	return mAmbientLightPower;
}

nex::ProbeBaker* nex::GlobalIllumination::getProbeBaker()
{
	return &mProbeBaker;
}

const nex::ProbeBaker* nex::GlobalIllumination::getProbeBaker() const
{
	return &mProbeBaker;
}

nex::ProbeManager* nex::GlobalIllumination::getProbeManager()
{
	return &mProbeManager;
}

const nex::ProbeManager* nex::GlobalIllumination::getProbeManager() const
{
	return &mProbeManager;
}

nex::VoxelConeTracer* nex::GlobalIllumination::getVoxelConeTracer()
{
	return &mVoxelConeTracer;
}

const nex::VoxelConeTracer* nex::GlobalIllumination::getVoxelConeTracer() const
{
	return &mVoxelConeTracer;
}

void nex::GlobalIllumination::setAmbientPower(float ambientPower)
{
	mAmbientLightPower = ambientPower;
}