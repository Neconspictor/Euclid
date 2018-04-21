#ifndef ABSTRACT_MATERIAL_LOADER_HPP
#define ABSTRACT_MATERIAL_LOADER_HPP


#include <material/Material.hpp>
#include <texture/TextureManager.hpp>
#include <memory>
#include <assimp/scene.h>
#include<vector>

class AbstractMaterialLoader
{
public:
	AbstractMaterialLoader(TextureManager* textureManager);

	virtual ~AbstractMaterialLoader();
	
	virtual std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const = 0;


protected:
	std::vector<std::string> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureData data) const;

	TextureManager* textureManager;
};

#endif ABSTRACT_MATERIAL_LOADER_HPP