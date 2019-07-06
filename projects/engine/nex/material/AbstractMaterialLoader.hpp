#pragma once

#include <memory>
#include <assimp/scene.h>
#include<vector>
#include <nex/material/Material.hpp>

namespace nex
{

	class TextureManager;

	class AbstractMaterialLoader
	{
	public:
		AbstractMaterialLoader(TextureManager* textureManager);

		virtual ~AbstractMaterialLoader();

		virtual void loadShadingMaterial(const std::filesystem::path& meshPathAbsolute, const aiScene* scene, MaterialStore& store, unsigned materialIndex) const = 0;

		virtual std::unique_ptr<Material> createMaterial(const MaterialStore& store) const = 0;


	protected:
		std::vector<std::string> loadMaterialTextures(const std::filesystem::path& meshDirectoryAbsolute, aiMaterial* mat, aiTextureType type) const;

		TextureManager* textureManager;
	};

	class DefaultMaterialLoader : public AbstractMaterialLoader
	{
	public:
		DefaultMaterialLoader() : AbstractMaterialLoader(nullptr) {}
		virtual ~DefaultMaterialLoader();
		virtual void loadShadingMaterial(const std::filesystem::path& meshPathAbsolute, const aiScene* scene, MaterialStore& store, unsigned materialIndex) const override { }
		std::unique_ptr<Material> createMaterial(const MaterialStore& store) const override { return nullptr; }
	};
}
