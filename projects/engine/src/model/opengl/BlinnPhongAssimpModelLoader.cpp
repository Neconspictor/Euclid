#pragma once

#include <model/opengl/BlinnPhongAssimpModelLoader.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <texture/opengl/TextureManagerGL.hpp>

using namespace std;


BlinnPhongMaterialLoader::BlinnPhongMaterialLoader() : AssimpModelLoader(), logClient(platform::getLogServer())
{
	logClient.setPrefix("BlinnPhongMaterialLoader");
}

BlinnPhongMaterialLoader::~BlinnPhongMaterialLoader()
{
}

std::unique_ptr<Material> BlinnPhongMaterialLoader::loadShadingMaterial(aiMesh * mesh, const aiScene * scene) const
{
	unique_ptr<BlinnPhongMaterial> material = make_unique<BlinnPhongMaterial>();
	TextureManagerGL* manager = TextureManagerGL::get();

	// process material (if any available)
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		TextureData data;
		data.useSRGB = true;
		data.generateMipMaps = true;
		data.uvTechnique = Repeat;
		data.minFilter = Linear_Linear;
		data.magFilter = Bilinear;
		data.colorspace = RGBA;

		// a material can have more than one diffuse/specular/normal map,
		// but we only use the first one by now
		vector<string> diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, data);
		if (diffuseMaps.size())
		{
			material->setDiffuseMap(manager->getImage(diffuseMaps[0], data));
		}
		vector<string> emissionMaps = loadMaterialTextures(mat, aiTextureType_EMISSIVE, data);
		if (emissionMaps.size())
		{
			material->setEmissionMap(manager->getImage(emissionMaps[0], data));
		}

		data.useSRGB = false;
		vector<string> specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, data);
		if (specularMaps.size())
		{
			material->setSpecularMap(manager->getImage(specularMaps[0], data));
		}

		data.useSRGB = false;
		vector<string> reflectionMaps = loadMaterialTextures(mat, aiTextureType_AMBIENT, data);
		if (reflectionMaps.size())
		{
			material->setReflectionMap(manager->getImage(reflectionMaps[0], data));
		}

		data.useSRGB = false;
		data.generateMipMaps = true;
		data.magFilter = Bilinear;
		data.minFilter = Linear_Linear;
		data.colorspace = RGBA;
		data.uvTechnique = Repeat;
		vector<string> normalMaps = loadMaterialTextures(mat, aiTextureType_HEIGHT, data);
		if (normalMaps.size())
		{
			material->setNormalMap(manager->getImage(normalMaps[0], data));
		}
	}

	material->setShininess(32);

	return move(material);
}