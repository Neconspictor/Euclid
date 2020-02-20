#include <nex/material/AbstractMaterialLoader.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/resource/FileSystem.hpp>
#include <nex/texture/Texture.hpp>

using namespace std;
using namespace nex;

AbstractMaterialLoader::AbstractMaterialLoader(TextureManager * textureManager)
{
	this->textureManager = textureManager;
}

AbstractMaterialLoader::~AbstractMaterialLoader() = default;

std::filesystem::path nex::AbstractMaterialLoader::createEmbeddedTexturePath(const std::filesystem::path & meshPathAbsolute, unsigned textureIndex) const
{
	return meshPathAbsolute.u8string() + "_" + std::to_string(textureIndex) + textureManager->getEmbeddedTextureFileExtension();
}

void nex::AbstractMaterialLoader::loadEmbeddedTexture(const std::filesystem::path & meshPathAbsolute, 
	const aiScene * scene, unsigned index, const TextureDesc & data, bool detectColorSpace) const
{
	auto* tex = scene->mTextures[index];
	if (tex->mHeight != 0) throw_with_trace(std::invalid_argument("Not supported embedded texture format!"));

	auto texPath = createEmbeddedTexturePath(meshPathAbsolute, index);

	auto embeddedTex = textureManager->loadEmbeddedImage(texPath, (const unsigned char*)tex->pcData, sizeof(glm::vec4) * tex->mWidth, true, data, detectColorSpace);
	textureManager->addToCache(std::move(embeddedTex), texPath);
}

vector<string> AbstractMaterialLoader::loadMaterialTextures(const aiScene* scene, const std::filesystem::path& meshPathAbsolute, aiMaterial* mat, aiTextureType type) const
{
	auto* fileSystem = textureManager->getFileSystem();
	vector<string> textures;
	const auto textureCount = mat->GetTextureCount(type);

	for (unsigned int i = 0; i < textureCount; ++i) {
		aiString texture;
		auto result = mat->GetTexture(type, i, &texture);
		textures.push_back(texture.C_Str());
	}

	std::vector<aiMaterialProperty*> properties(mat->mNumProperties);

	for (auto i = 0; i < mat->mNumProperties; ++i) {
		properties[i] = mat->mProperties[i];
	}
	

	for (unsigned int i = 0; i < textureCount; ++i)
	{
		std::filesystem::path texturePath;


		if (isEmbedded(textures[i])) {
			auto index = getEmbeddedTextureIndex(textures[i]);
			textures[i] = createEmbeddedTexturePath(meshPathAbsolute, index).generic_string();
		}
		else {

			texturePath = fileSystem->resolveAbsolute(textures[i], meshPathAbsolute.parent_path());
			textures[i] = fileSystem->rebase(texturePath).generic_u8string();
		}
		
		 

		

		
	}

	return textures;
}

bool nex::AbstractMaterialLoader::isEmbedded(const std::filesystem::path& path) const
{
	if (path.has_extension()) {
		return path.extension() == textureManager->getEmbeddedTextureFileExtension();
	}

	auto str = path.generic_string();
	if (str.size() > 0) {
		return str[0] == '*';
	}

	return false;
}

unsigned nex::AbstractMaterialLoader::getEmbeddedTextureIndex(const std::filesystem::path& path)
{
	std::string indexStr;

	if (path.has_extension()) {
		auto fileName = path.filename().generic_string();
		auto extension = path.extension().generic_string();
		fileName = fileName.substr(0, fileName.size() - extension.size());
		auto splitIndex = fileName.find_last_of("_");
		indexStr = fileName.substr(splitIndex + 1, fileName.size() - splitIndex);
	}
	else {
		indexStr = path.generic_string();
		indexStr = indexStr.substr(1, indexStr.size() - 1);
	}

	return static_cast<unsigned>(std::stoul(indexStr));
}

void nex::AbstractMaterialLoader::loadOptionalEmbeddedTexture(std::filesystem::path& source, const std::filesystem::path& meshPathAbsolute, 
	const aiScene* scene, const TextureDesc& desc, bool detectColorSpace) const
{
	if (isEmbedded(source)) {

		unsigned index = getEmbeddedTextureIndex(source);
		loadEmbeddedTexture(meshPathAbsolute, scene, index, desc, detectColorSpace);
	}
}

DefaultMaterialLoader::~DefaultMaterialLoader() = default;