#include <nex/GI/ProbeManager.hpp>
#include <nex/GI/PbrProbe.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/cluster/Cluster.hpp>
#include <nex/mesh/MeshGroup.hpp>


nex::ProbeManager::ProbeManager(unsigned prefilteredSize, unsigned depth) : 
	mFactory(prefilteredSize, depth),
	mEnvironmentLights(0, 0, nullptr, ShaderBuffer::UsageHint::DYNAMIC_COPY),
	mProbeCluster(std::make_unique<ProbeCluster>()),
	mNextStoreID(0),
	mActive(nullptr)
{
}

nex::ProbeVob* nex::ProbeManager::addUninitProbeUnsafe(const glm::vec3& position, unsigned storeID)
{
	advanceNextStoreID(storeID);
	return createUninitializedProbeVob(position, storeID);
}

nex::PbrProbe* nex::ProbeManager::getActiveProbe()
{
	return mActive;
}

nex::PbrProbeFactory* nex::ProbeManager::getFactory()
{
	return &mFactory;
}

nex::CubeMapArray* nex::ProbeManager::getIrradianceMaps()
{
	return mFactory.getIrradianceMaps();
}

unsigned nex::ProbeManager::getNextStoreID() const
{
	return mNextStoreID;
}

nex::CubeMapArray* nex::ProbeManager::getReflectionMaps()
{
	return mFactory.getReflectionMaps();
}

const std::vector<std::unique_ptr<nex::PbrProbe>>& nex::ProbeManager::getProbes() const
{
	return mProbes;
}

nex::ProbeCluster* nex::ProbeManager::getProbeCluster()
{
	return mProbeCluster.get();
}

const nex::ProbeCluster* nex::ProbeManager::getProbeCluster() const
{
	return mProbeCluster.get();
}

nex::ShaderStorageBuffer* nex::ProbeManager::getEnvironmentLightShaderBuffer()
{
	return &mEnvironmentLights;
}

void nex::ProbeManager::setActiveProbe(PbrProbe* probe)
{
	mActive = probe;
}

void nex::ProbeManager::update(const nex::Scene::ProbeRange& activeProbes)
{
	const auto byteSize = activeProbes.size() * sizeof(EnvironmentLight);

	if (mEnvironmentLights.getSize() < byteSize)
		mEnvironmentLights.resize(byteSize, nullptr, GpuBuffer::UsageHint::DYNAMIC_COPY);

	std::vector<EnvironmentLight> lights(activeProbes.size());

	size_t counter = 0;
	for (auto* vob : activeProbes) {
		auto& light = lights[counter];
		auto* probe = vob->getProbe();
		++counter;

		const auto& trafo = vob->getTrafoMeshToWorld();
		light.enabled = true;
		light.position = glm::vec4(trafo[3][0], trafo[3][1], trafo[3][2], 1.0f);
		light.sphereRange = probe->getInfluenceRadius();
		const auto& box = probe->getInfluenceBox();
		light.minWorld = glm::vec4(box.min, 0.0);
		light.maxWorld = glm::vec4(box.max, 0.0);
		light.usesBoundingBox = probe->getInfluenceType() == PbrProbe::InfluenceType::BOX;
		light.arrayIndex = probe->getArrayIndex();
	}

	mEnvironmentLights.update(byteSize, lights.data());
}

void nex::ProbeManager::advanceNextStoreID(unsigned id)
{
	if (id == PbrProbe::INVALID_STOREID) return;

	mNextStoreID = max(mNextStoreID, id);

	if (mNextStoreID == id) ++mNextStoreID;
}

nex::ProbeVob* nex::ProbeManager::createUninitializedProbeVob(const glm::vec3& position, unsigned storeID)
{
	auto probe = std::make_unique<PbrProbe>(position, storeID);

	auto* probVobPtr = new ProbeVob(nullptr, probe.get());
	std::unique_ptr<ProbeVob> vob(probVobPtr);
	vob->setBatches(probe->getMeshGroup()->getBatches());
	vob->setScaleLocalToParent(glm::vec3(0.3f));

	mProbes.emplace_back(std::move(probe));
	mProbeVobs.emplace_back(std::move(vob));

	return mProbeVobs.back().get();
}
