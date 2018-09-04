#include <nex/material/PbrMaterialLoader.hpp>
#include <nex/material/PbrMaterial.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <string>

using namespace std;


PbrMaterialLoader::PbrMaterialLoader(TextureManager* textureManager) : AbstractMaterialLoader(textureManager), logClient(nex::getLogServer())
{
	logClient.setPrefix("PbrMaterialLoader");
}

PbrMaterialLoader::~PbrMaterialLoader()
{
}

std::unique_ptr<Material> PbrMaterialLoader::loadShadingMaterial(aiMesh * mesh, const aiScene * scene) const
{
	unique_ptr<PbrMaterial> material = make_unique<PbrMaterial>();

	// process material (if any available)
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		TextureData data;
		data.useSRGB = true;
		data.generateMipMaps = true;
		data.uvTechnique = Repeat;
		data.minFilter = Linear_Mipmap_Linear;
		data.magFilter = Linear;
		data.colorspace = RGBA;
		data.isFloatData = false;
		data.resolution = BITS_8;

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

		data.useSRGB = false;
		vector<string> metallicMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, data);
		if (metallicMaps.size())
		{
			material->setMetallicMap(textureManager->getImage(metallicMaps[0], data));
		} else
		{
			material->setMetallicMap(textureManager->getDefaultBlackTexture()); // we assume a non metallic material
		}

		data.useSRGB = false;
		vector<string> roughnessMaps = loadMaterialTextures(mat, aiTextureType_SHININESS, data);
		if (roughnessMaps.size())
		{
			material->setRoughnessMap(textureManager->getImage(roughnessMaps[0], data));
		} else
		{
			material->setRoughnessMap(textureManager->getDefaultWhiteTexture()); // we assume a full rough material
		}

		data.useSRGB = false;

		//normal maps shouldn't use mipmaps (important for shading!)
		data.generateMipMaps = true;
		// Linear_Mipmap_Linear is also important!
		data.magFilter = Linear;
		data.minFilter = Linear_Mipmap_Linear;
		data.colorspace = RGB;
		data.uvTechnique = Repeat;
		data.isFloatData = false;
		data.resolution = BITS_8;
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
	}

	return move(material);
}