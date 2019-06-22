﻿#include <techniques/GlobalIllumination.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/FileSystem.hpp>
#include <nex/Scene.hpp>
#include <nex/mesh/StaticMesh.hpp>


nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory) : 
mFactory(PbrProbeFactory::get(compiledProbeDirectory))
{
	TextureManager* textureManager = TextureManager::get();

	auto* hdr = textureManager->getImage("hdr/HDR_040_Field.hdr",
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

	mProbes.emplace_back(mFactory->create(hdr, mProbes.size()));
}

nex::GlobalIllumination::~GlobalIllumination() = default;

nex::PbrProbe* nex::GlobalIllumination::getProbe()
{
	return mProbes[0].get();
}

nex::Vob* nex::GlobalIllumination::createVob(PbrProbe* probe, Scene& scene)
{
	auto* meshRootNode = StaticMesh::createNodeHierarchy(&scene,
		{ std::pair<Mesh*, Material*>(PbrProbe::getSphere(), probe->getMaterial()) });

	auto vob = std::make_unique<ProbeVob>(meshRootNode, probe);

	return scene.addVob(std::move(vob), true);
}