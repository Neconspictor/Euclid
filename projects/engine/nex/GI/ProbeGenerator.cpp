#include <nex/GI/ProbeGenerator.hpp>
#include <nex/mesh/UtilityMeshes.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/scene/Scene.hpp>
#include <nex/effects/SimpleColorPass.hpp>
#include <nex/GI/PbrProbe.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/GI/GlobalIllumination.hpp>
#include <nex/renderer/Renderer.hpp>
#include <nex/gui/VisualizationSphere.hpp>

nex::ProbeGenerator::ProbeGenerator(Scene* scene, VisualizationSphere* sphere, 
	nex::GlobalIllumination* globalIllumination, 
	nex::Renderer* renderer) :
	
	mScene(scene),
	mSphere(sphere),
mInfluenceRadius(0.5f),
mGlobalIllumination(globalIllumination),
mRenderer(renderer)
{	
}

nex::ProbeGenerator::~ProbeGenerator() = default;

void nex::ProbeGenerator::show(bool visible)
{
	mSphere->show(visible);
}

const glm::vec3& nex::ProbeGenerator::getProbePosition() const
{
	return mSphere->getVob()->getPositionLocalToParent();
}

float nex::ProbeGenerator::getInfluenceRadius() const
{
	return mInfluenceRadius;
}

nex::ProbeVob* nex::ProbeGenerator::generate()
{
	auto* vob = mSphere->getVob();

	auto* probe = mGlobalIllumination->addUninitProbeUnsafe(vob->getPositionLocalToParent(),
		mGlobalIllumination->getNextStoreID());

	mGlobalIllumination->bakeProbe(probe, *mScene, mRenderer);


	probe->getProbe()->setInfluenceRadius(mInfluenceRadius);

	probe->updateWorldTrafoHierarchy();

	auto lock = mScene->acquireLock();
	mScene->addActiveVobUnsafe(probe);

	return probe;
}

void nex::ProbeGenerator::update(const glm::vec3& position, float influenceRadius)
{
	auto* vob = mSphere->getVob();
	vob->setPositionLocalToParent(position);
	mInfluenceRadius = influenceRadius;
	vob->setScaleLocalToParent(glm::vec3(mInfluenceRadius));
	vob->updateTrafo(true);
}