#include <nex/pbr/GlobalIllumination.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/resource/FileSystem.hpp>
#include <nex/Scene.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderBackend.hpp>


nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory, unsigned prefilteredSize, unsigned depth) :
mFactory(prefilteredSize, depth)
{
}

nex::GlobalIllumination::~GlobalIllumination() = default;

const std::vector<std::unique_ptr<nex::PbrProbe>>& nex::GlobalIllumination::getProbes() const
{
	return mProbes;
}

nex::Vob* nex::GlobalIllumination::createVobUnsafe(PbrProbe* probe, Scene& scene)
{
	auto* meshRootNode = StaticMesh::createNodeHierarchy(&scene,
		{ std::pair<Mesh*, Material*>(PbrProbe::getSphere(), probe->getMaterial()) });

	auto vob = std::make_unique<ProbeVob>(meshRootNode, probe);

	return scene.addVobUnsafe(std::move(vob), true);
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

nex::PbrProbeFactory* nex::GlobalIllumination::getFactory()
{
	return &mFactory;
}

void nex::GlobalIllumination::setActiveProbe(PbrProbe * probe)
{
	mActive = probe;
}