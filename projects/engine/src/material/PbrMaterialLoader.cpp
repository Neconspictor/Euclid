#pragma once

#include <material/PbrMaterialLoader.hpp>
#include <material/PbrMaterial.hpp>
#include <util/Globals.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <string>

using namespace std;


PbrMaterialLoader::PbrMaterialLoader(TextureManager* textureManager) : AbstractMaterialLoader(textureManager), logClient(platform::getLogServer())
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

		// a material can have more than one diffuse/specular/normal map,
		// but we only use the first one by now
		vector<string> albedoMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, data);
		if (albedoMaps.size())
		{
			material->setAlbedoMap(textureManager->getImage(albedoMaps[0], data));
		}

		vector<string> aoMaps = loadMaterialTextures(mat, aiTextureType_AMBIENT, data);
		if (aoMaps.size())
		{
			material->setAlbedoMap(textureManager->getImage(aoMaps[0], data));
		}

		vector<string> emissionMaps = loadMaterialTextures(mat, aiTextureType_EMISSIVE, data);
		if (emissionMaps.size())
		{
			material->setEmissionMap(textureManager->getImage(emissionMaps[0], data));
		}

		data.useSRGB = false;
		vector<string> metallicMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, data);
		if (metallicMaps.size())
		{
			material->setMetallicMap(textureManager->getImage(metallicMaps[0], data));
		}

		data.useSRGB = false;
		vector<string> roughnessMaps = loadMaterialTextures(mat, aiTextureType_SHININESS, data);
		if (roughnessMaps.size())
		{
			material->setRoughnessMap(textureManager->getImage(roughnessMaps[0], data));
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
		vector<string> normalMaps = loadMaterialTextures(mat, aiTextureType_HEIGHT, data);
		if (normalMaps.size())
		{
			material->setNormalMap(textureManager->getImage(normalMaps[0], data));
			//material->setNormalMap(textureManager->getDefaultNormalTexture());
		}
	}

	return move(material);
}