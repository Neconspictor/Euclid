#pragma once

#include <memory>
#include <assimp/scene.h>
#include<vector>
#include "Material.hpp"


namespace nex
{

	class TextureManager;
	struct TextureData;

	class AbstractMaterialLoader
	{
	public:
		AbstractMaterialLoader(TechniqueSelector* selector, TextureManager* textureManager);

		virtual ~AbstractMaterialLoader();

		virtual std::vector<std::unique_ptr<Material>> loadShadingMaterial(const aiScene* scene) const = 0;


	protected:
		std::vector<std::string> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureData data) const;

		TextureManager* textureManager;
		TechniqueSelector* mSelector;
	};

	class DefaultMaterialLoader : public AbstractMaterialLoader
	{
	public:
		DefaultMaterialLoader() : AbstractMaterialLoader(nullptr, nullptr) {}
		std::vector<std::unique_ptr<Material>> loadShadingMaterial(const aiScene* scene) const override { return {}; }
	};
}
