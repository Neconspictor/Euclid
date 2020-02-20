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

		std::filesystem::path createEmbeddedTexturePath(const std::filesystem::path& meshPathAbsolute, unsigned textureIndex) const;

		void loadEmbeddedTexture(const std::filesystem::path& meshPathAbsolute, const aiScene* scene, unsigned index, const TextureDesc& data, bool detectColorSpace) const;

		virtual void loadShadingMaterial(const std::filesystem::path& meshPathAbsolute, const aiScene* scene, MaterialStore& store, unsigned materialIndex) const = 0;

		virtual std::unique_ptr<Material> createMaterial(const MaterialStore& store) const = 0;


	protected:
		std::vector<std::string> loadMaterialTextures(const aiScene* scene, const std::filesystem::path& meshPathAbsolute, aiMaterial* mat, aiTextureType type) const;

		bool isEmbedded(const std::filesystem::path& path) const;
		static unsigned getEmbeddedTextureIndex(const std::filesystem::path& path);

		void loadOptionalEmbeddedTexture(std::filesystem::path& source, const std::filesystem::path& meshPathAbsolute, const aiScene* scene, const TextureDesc& desc, bool detectColorSpace) const;

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
