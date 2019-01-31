#pragma once

#include <memory>
#include <assimp/scene.h>
#include<vector>


namespace nex
{

	class TextureManagerGL;
	struct TextureData;
	class Material;

	class AbstractMaterialLoader
	{
	public:
		AbstractMaterialLoader(TextureManagerGL* textureManager);

		virtual ~AbstractMaterialLoader();

		virtual std::vector<std::unique_ptr<Material>> loadShadingMaterial(const aiScene* scene) const = 0;


	protected:
		std::vector<std::string> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureData data) const;

		TextureManagerGL* textureManager;
	};
}
