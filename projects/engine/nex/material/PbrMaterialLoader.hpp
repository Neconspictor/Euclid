#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>
#include <nex/shader/ShaderProvider.hpp>


namespace nex
{
	class BasePbrGeometryShader;


	using PbrShaderProvider = TypedOwningShaderProvider<BasePbrGeometryShader>;

	class PbrMaterialLoader : public AbstractMaterialLoader {

	public:

		enum class LoadMode {
			ALPHA_TRANSPARENCY,
			SOLID_ALPHA_STENCIL,
			SOLID,
		};

		PbrMaterialLoader(std::shared_ptr<PbrShaderProvider> staticMeshShaderProvider,
			std::shared_ptr<PbrShaderProvider> skinnedMeshShaderProvider,
			TextureManager* textureManager,
			LoadMode mode = LoadMode::SOLID);

		virtual ~PbrMaterialLoader();

		void setLoadMode(LoadMode mode);

		void loadShadingMaterial(const std::filesystem::path& meshPath, const aiScene* scene, MaterialStore& store, unsigned materialIndex, bool isSkinned) const override;
		std::unique_ptr<Material> createMaterial(const MaterialStore& store) const override;
	
	private:
		std::shared_ptr<PbrShaderProvider> mStaticMeshShaderProvider;
		std::shared_ptr<PbrShaderProvider> mSkinnedMeshShaderProvider;
		LoadMode mMode;
	};
}