#include <nex/pbr/GlobalIllumination.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/resource/FileSystem.hpp>
#include <nex/Scene.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <glm/glm.hpp>


nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth) :
mFactory(prefilteredSize, depth), mProbesBuffer(1, sizeof(ProbeData), ShaderBuffer::UsageHint::DYNAMIC_COPY)
{
}

nex::GlobalIllumination::~GlobalIllumination() = default;

const std::vector<std::unique_ptr<nex::PbrProbe>>& nex::GlobalIllumination::getProbes() const
{
	return mProbes;
}

nex::ProbeVob* nex::GlobalIllumination::createVobUnsafe(PbrProbe* probe, Scene& scene)
{
	auto* meshRootNode = StaticMesh::createNodeHierarchy(&scene,
		{ std::pair<Mesh*, Material*>(PbrProbe::getSphere(), probe->getMaterial()) });

	auto vob = std::make_unique<ProbeVob>(meshRootNode, probe);

	return (ProbeVob*)scene.addVobUnsafe(std::move(vob), true);
}

void nex::GlobalIllumination::addProbe(std::unique_ptr<PbrProbe> probe)
{
	mProbes.emplace_back(std::move(probe));
}

nex::PbrProbe * nex::GlobalIllumination::getActiveProbe()
{
	return mActive;
}

nex::CubeMapArray * nex::GlobalIllumination::getIrradianceMaps()
{
	return mFactory.getIrradianceMaps();
}

nex::CubeMapArray * nex::GlobalIllumination::getPrefilteredMaps()
{
	return mFactory.getPrefilteredMaps();
}

const nex::GlobalIllumination::ProbesData & nex::GlobalIllumination::getProbesData() const
{
	return mProbesData;
}

nex::ShaderStorageBuffer * nex::GlobalIllumination::getProbesShaderBuffer()
{
	return &mProbesBuffer;
}

nex::PbrProbeFactory* nex::GlobalIllumination::getFactory()
{
	return &mFactory;
}

void nex::GlobalIllumination::setActiveProbe(PbrProbe * probe)
{
	mActive = probe;
}

void nex::GlobalIllumination::update(const nex::Scene::ProbeRange & activeProbes)
{
	mProbesData.resize(activeProbes.size());
	ProbeVob::ProbeData* data = mProbesData.data();

	unsigned counter = 0;

	for (auto* vob : activeProbes) {
		const auto& trafo = vob->getMeshRootNode()->getWorldTrafo();
		data[counter].positionWorld = glm::vec4(trafo[3][0], trafo[3][1], trafo[3][2], 0);
		data[counter].arrayIndex.x = vob->getProbe()->getArrayIndex();
		++counter;
	}

	const auto oldSize = mProbesBuffer.getSize();

	if (mProbesData.memSize() == oldSize) {
		mProbesBuffer.update(data, mProbesData.memSize());
	}
	else {
		mProbesBuffer.resize(data, mProbesData.memSize(), ShaderBuffer::UsageHint::DYNAMIC_COPY);
	}
}