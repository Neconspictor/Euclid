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

		std::unique_ptr<Material> loadShadingMaterial(const aiScene* scene, unsigned materialIndex) const override;
	
	private:
		PbrDeferred* mPbrDeferred;
		PbrForward* mPbrForward;
	};
}