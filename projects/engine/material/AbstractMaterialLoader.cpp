#include <material/AbstractMaterialLoader.hpp>

using namespace std;

AbstractMaterialLoader::AbstractMaterialLoader(TextureManager * textureManager)
{
	this->textureManager = textureManager;
}

AbstractMaterialLoader::~AbstractMaterialLoader()
{
}

vector<string> AbstractMaterialLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureData data) const
{
	vector<string> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString texture;
		mat->GetTexture(type, i, &texture);

		textureManager->getImage(texture.C_Str(), data);
		textures.push_back(texture.C_Str());
	}

	return textures;
}