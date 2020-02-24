#pragma once

#include <nex/material/AbstractMaterialLoader.hpp>
#include <nex/shader/ShaderProvider.hpp>


namespace nex
{
	class BasePbrGeometryShader;


	using PbrShaderProvider = TypedOwningShaderProvider<BasePbrGeometryShader>;

	class PbrMaterialLoader : public AbstractMaterialLoader {

	public:

		PbrMaterialLoader(std::shared_ptr<PbrShaderProvider> staticDeferredMeshShaderProvider,
			std::shared_ptr<PbrShaderProvider> skinnedDeferredMeshShaderProvider,
			std::shared_ptr<PbrShaderProvider> staticForwardMeshShaderProvider,
			std::shared_ptr<PbrShaderProvider> skinnedForwardMeshShaderProvider,
			TextureManager* textureManager);

		virtual ~PbrMaterialLoader();

		void loadShadingMaterial(const std::filesystem::path& meshPath, const aiScene* scene, MaterialStore& store, unsigned materialIndex, bool isSkinned) const override;
		std::unique_ptr<Material> createMaterial(const MaterialStore& store) const override;
	
	private:
		std::shared_ptr<PbrShaderProvider> mStaticDeferredMeshShaderProvider;
		std::shared_ptr<PbrShaderProvider> mSkinnedDeferredMeshShaderProvider;
		std::shared_ptr<PbrShaderProvider> mStaticForwardMeshShaderProvider;
		std::shared_ptr<PbrShaderProvider> mSkinnedForwardMeshShaderProvider;

		AlphaMode getAlphaMode(const std::string& name) const;
	};
}