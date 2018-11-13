#pragma once

#include <nex/opengl/material/Material.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <memory>
#include <assimp/scene.h>
#include<vector>

class AbstractMaterialLoader
{
public:
	AbstractMaterialLoader(TextureManagerGL* textureManager);

	virtual ~AbstractMaterialLoader();
	
	virtual std::unique_ptr<Material> loadShadingMaterial(aiMesh* mesh, const aiScene* scene) const = 0;


protected:
	std::vector<std::string> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureData data) const;

	TextureManagerGL* textureManager;
};