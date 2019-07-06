#include <nex/material/AbstractMaterialLoader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/resource/FileSystem.hpp>

using namespace std;
using namespace nex;

AbstractMaterialLoader::AbstractMaterialLoader(TextureManager * textureManager)
{
	this->textureManager = textureManager;
}

AbstractMaterialLoader::~AbstractMaterialLoader() = default;

vector<string> AbstractMaterialLoader::loadMaterialTextures(const std::filesystem::path& meshPathAbsolute, aiMaterial* mat, aiTextureType type) const
{
	auto* fileSystem = textureManager->getFileSystem();
	vector<string> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString texture;
		auto result = mat->GetTexture(type, i, &texture);

		std::filesystem::path texturePath = texture.C_Str();
		texturePath = fileSystem->resolveAbsolute(texturePath);

		if (fileSystem->isContained(texturePath, meshPathAbsolute)) {
			textures.emplace_back(texturePath.generic_string());
		}
		else {
			textures.push_back(texture.C_Str());
		}
	}

	return textures;
}

DefaultMaterialLoader::~DefaultMaterialLoader() = default;