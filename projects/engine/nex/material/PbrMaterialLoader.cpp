#include <nex/material/PbrMaterialLoader.hpp>
#include <nex/material/Material.hpp>
#include <string>
#include <nex/texture/Texture.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/pbr/PbrDeferred.hpp>


using namespace std;
using namespace nex;


PbrMaterialLoader::PbrMaterialLoader(PbrDeferred* pbrDeferred, PbrForward* pbrForward, TextureManager* textureManager) : AbstractMaterialLoader(textureManager),
mPbrDeferred(pbrDeferred), mPbrForward(pbrForward)
{
}

PbrMaterialLoader::~PbrMaterialLoader() = default;

std::unique_ptr<Material> PbrMaterialLoader::loadShadingMaterial(const aiScene * scene, unsigned materialIndex) const
{
	if (scene->mNumMaterials <= materialIndex)
	{
		throw std::runtime_error("PbrMaterialLoader: materialIndex out of range!");
	}

	aiMaterial* mat = scene->mMaterials[materialIndex];

	// TODO decide when to use pbrForward!
	auto material = make_unique<PbrMaterial>(mPbrDeferred);

	TextureData data = {
		TextureFilter::Linear_Mipmap_Linear,
		TextureFilter::Linear,
		TextureUVTechnique::Repeat,
		TextureUVTechnique::Repeat,
		TextureUVTechnique::Repeat,
		ColorSpace::SRGBA,
		PixelDataType::UBYTE,
		InternFormat::SRGBA8,
		true
	};

	// a material can have more than one diffuse/specular/normal map,
	// but we only use the first one by now
	vector<string> albedoMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, data);
	if (albedoMaps.size())
	{
		material->setAlbedoMap(textureManager->getImage(albedoMaps[0], data));
	} else
	{
		material->setAlbedoMap(textureManager->getDefaultWhiteTexture()); // assume white material
	}

	vector<string> aoMaps = loadMaterialTextures(mat, aiTextureType_AMBIENT, data);
	if (aoMaps.size())
	{
		material->setAoMap(textureManager->getImage(aoMaps[0], data));
	} else
	{
		material->setAoMap(textureManager->getDefaultWhiteTexture()); // no ao
	}

	vector<string> emissionMaps = loadMaterialTextures(mat, aiTextureType_EMISSIVE, data);
	if (emissionMaps.size())
	{
		material->setEmissionMap(textureManager->getImage(emissionMaps[0], data));
	} else
	{
		material->setEmissionMap(textureManager->getDefaultBlackTexture()); // no emission
	}

	// the following textures are linear, so we use the RGBA color space
	data.colorspace = ColorSpace::RGBA;
	data.internalFormat = InternFormat::RGBA8;

	vector<string> metallicMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, data);
	if (metallicMaps.size())
	{
		material->setMetallicMap(textureManager->getImage(metallicMaps[0], data));
	} else
	{
		material->setMetallicMap(textureManager->getDefaultBlackTexture()); // we assume a non metallic material
	}

	vector<string> roughnessMaps = loadMaterialTextures(mat, aiTextureType_SHININESS, data);
	if (roughnessMaps.size())
	{
		material->setRoughnessMap(textureManager->getImage(roughnessMaps[0], data));
	} else
	{
		material->setRoughnessMap(textureManager->getDefaultWhiteTexture()); // we assume a full rough material
	}


	//normal maps shouldn't use mipmaps (important for shading!)
	data.generateMipMaps = true; // TODO use mip maps or not????

	vector<string> normalMaps = loadMaterialTextures(mat, aiTextureType_HEIGHT, data);
	if (normalMaps.size())
	{
		Texture* texture = textureManager->getImage(normalMaps[0], data);
		material->setNormalMap(texture);
		//material->setNormalMap(textureManager->getDefaultNormalTexture());
	} else
	{
		Texture* texture = textureManager->getDefaultNormalTexture();
		material->setNormalMap(texture);
	}

	return material;
}