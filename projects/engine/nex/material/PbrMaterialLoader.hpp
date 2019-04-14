#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>

namespace nex
{
	class PbrMaterialLoader : public AbstractMaterialLoader {

	public:

		PbrMaterialLoader(TechniqueSelector* selector, TextureManager* textureManager);

		virtual ~PbrMaterialLoader();

		std::vector<std::unique_ptr<Material>> loadShadingMaterial(const aiScene* scene) const override;
	};
}