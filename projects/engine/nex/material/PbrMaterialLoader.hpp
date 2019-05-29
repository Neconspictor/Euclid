#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>

namespace nex
{
	class PbrDeferred;
	class PbrForward;

	class PbrMaterialLoader : public AbstractMaterialLoader {

	public:

		PbrMaterialLoader(PbrDeferred* pbrDeferred, PbrForward* pbrForward, TextureManager* textureManager);

		virtual ~PbrMaterialLoader();

		void loadShadingMaterial(const aiScene* scene, MaterialStore& store, unsigned materialIndex) const override;
		std::unique_ptr<Material> createMaterial(const MaterialStore& store) const override;
	
	private:
		PbrDeferred* mPbrDeferred;
		PbrForward* mPbrForward;
	};
}