#include <techniques/GlobalIllumination.hpp>
#include "nex/texture/TextureManager.hpp"
#include <nex/FileSystem.hpp>
#include <nex/mesh/Sphere.hpp>

nex::GlobalIllumination::GlobalIllumination(const std::string& compiledProbeDirectory) : 
mSphere(std::make_unique<SphereMesh>(16, 16)),
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