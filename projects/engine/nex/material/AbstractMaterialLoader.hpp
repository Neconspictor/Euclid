#pragma once

#include <memory>
#include <assimp/scene.h>
#include<vector>


namespace nex
{

	class TextureManager;
	struct TextureData;
	class Material;

	class AbstractMaterialLoader
	{
	public:
		AbstractMaterialLoader(TextureManager* textureManager);

		virtual ~AbstractMaterialLoader();

		virtual std::unique_ptr<Material> loadShadingMaterial(const aiScene* scene, unsigned materialIndex) const = 0;


	protected:
		std::vector<std::string> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureData data) const;

		TextureManager* textureManager;
	};

	class DefaultMaterialLoader : public AbstractMaterialLoader
	{
	public:
		DefaultMaterialLoader() : AbstractMaterialLoader(nullptr) {}
		virtual ~DefaultMaterialLoader();
		std::unique_ptr<Material> loadShadingMaterial(const aiScene* scene, unsigned materialIndex) const override { return nullptr; }
	};
}
