#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>

namespace nex
{
	class PbrTechnique;

	class PbrMaterialLoader : public AbstractMaterialLoader {

	public:

		PbrMaterialLoader(PbrTechnique* pbrTechnique, TextureManager* textureManager);

		virtual ~PbrMaterialLoader();

		void loadShadingMaterial(const aiScene* scene, MaterialStore& store, unsigned materialIndex) const override;
		std::unique_ptr<Material> createMaterial(const MaterialStore& store) const override;
	
	private:
		PbrTechnique* mTechnique;
	};
}