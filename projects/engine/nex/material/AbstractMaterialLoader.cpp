#include <nex/material/AbstractMaterialLoader.hpp>
#include <nex/texture/TextureManager.hpp>

using namespace std;
using namespace nex;

AbstractMaterialLoader::AbstractMaterialLoader(TechniqueSelector* selector, TextureManager * textureManager)
{
	this->textureManager = textureManager;
	mSelector = selector;
}

AbstractMaterialLoader::~AbstractMaterialLoader() = default;

vector<string> AbstractMaterialLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureData data) const
{
	vector<string> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString texture;
		mat->GetTexture(type, i, &texture);

		textureManager->getImage(texture.C_Str(), data);
		textures.push_back(texture.C_Str());
	}

	return textures;
}