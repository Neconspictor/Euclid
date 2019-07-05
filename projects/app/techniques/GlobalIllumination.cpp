#include <techniques/GlobalIllumination.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/resource/FileSystem.hpp>
#include <nex/Scene.hpp>
#include <nex/mesh/StaticMesh.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/renderer/RenderBackend.hpp>


nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory) : 
mFactory(PbrProbeFactory::get(compiledProbeDirectory))
{
}

nex::GlobalIllumination::~GlobalIllumination() = default;

nex::PbrProbe* nex::GlobalIllumination::getProbe()
{
	return mProbes[0].get();
}

nex::Vob* nex::GlobalIllumination::createVobUnsafe(PbrProbe* probe, Scene& scene)
{
	auto* meshRootNode = StaticMesh::createNodeHierarchy(&scene,
		{ std::pair<Mesh*, Material*>(PbrProbe::getSphere(), probe->getMaterial()) });

	auto vob = std::make_unique<ProbeVob>(meshRootNode, probe);

	return scene.addVobUnsafe(std::move(vob), true);
}

void nex::GlobalIllumination::loadHdr()
{
	TextureManager* textureManager = TextureManager::get();
	mHdr = textureManager->getImage("hdr/HDR_040_Field.hdr",
		{
			TextureFilter::Linear,
			TextureFilter::Linear,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			TextureUVTechnique::ClampToEdge,
			ColorSpace::RGB,
			PixelDataType::FLOAT,
			InternFormat::RGB32F,
			false }
	);
}

void nex::GlobalIllumination::loadProbes(std::unique_ptr<PbrProbe> probe)
{
	mProbes.clear();
	auto* pointer = probe.get();
	mProbes.emplace_back(std::move(probe));
}

nex::PbrProbeFactory* nex::GlobalIllumination::getFactory()
{
	return mFactory;
}

nex::Texture2D* nex::GlobalIllumination::getHdr()
{
	return mHdr;
}
