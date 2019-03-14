#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>

namespace nex
{
	class PbrMaterialLoader : public AbstractMaterialLoader {

	public:

		PbrMaterialLoader(TextureManager* textureManager);

		virtual ~PbrMaterialLoader();

		std::vector<std::unique_ptr<Material>> loadShadingMaterial(const aiScene* scene) const override;
	};
}