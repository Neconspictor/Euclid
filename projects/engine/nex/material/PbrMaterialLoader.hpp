#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>

namespace nex
{
	class BasePbrGeometryShader;

	class PbrMaterialLoader : public AbstractMaterialLoader {

	public:

		enum class LoadMode {
			ALPHA_TRANSPARENCY,
			SOLID_ALPHA_STENCIL,
			SOLID,
		};

		PbrMaterialLoader(BasePbrGeometryShader* pbrTechnique,
			TextureManager* textureManager,
			LoadMode mode = LoadMode::SOLID);

		virtual ~PbrMaterialLoader();

		void setLoadMode(LoadMode mode);

		void loadShadingMaterial(const std::filesystem::path& meshPath, const aiScene* scene, MaterialStore& store, unsigned materialIndex) const override;
		std::unique_ptr<Material> createMaterial(const MaterialStore& store) const override;
	
	private:
		BasePbrGeometryShader* mShader;
		LoadMode mMode;
	};
}