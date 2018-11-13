#include <nex/opengl/material/BlinnPhongMaterialLoader.hpp>
#include <nex/opengl/material/BlinnPhongMaterial.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <string>

using namespace std;


BlinnPhongMaterialLoader::BlinnPhongMaterialLoader(TextureManagerGL* textureManager) : AbstractMaterialLoader(textureManager), logClient(nex::getLogServer())
{
	logClient.setPrefix("BlinnPhongMaterialLoader");
}

BlinnPhongMaterialLoader::~BlinnPhongMaterialLoader()
{
}

std::unique_ptr<Material> BlinnPhongMaterialLoader::loadShadingMaterial(aiMesh * mesh, const aiScene * scene) const
{
	unique_ptr<BlinnPhongMaterial> material = make_unique<BlinnPhongMaterial>();

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
		vector<string> diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, data);
		if (diffuseMaps.size())
		{
			material->setDiffuseMap(textureManager->getImage(diffuseMaps[0], data));
		}
		vector<string> emissionMaps = loadMaterialTextures(mat, aiTextureType_EMISSIVE, data);
		if (emissionMaps.size())
		{
			material->setEmissionMap(textureManager->getImage(emissionMaps[0], data));
		}

		data.useSRGB = false;
		vector<string> specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, data);
		if (specularMaps.size())
		{
			material->setSpecularMap(textureManager->getImage(specularMaps[0], data));
		}

		data.useSRGB = false;
		vector<string> reflectionMaps = loadMaterialTextures(mat, aiTextureType_AMBIENT, data);
		if (reflectionMaps.size())
		{
			material->setReflectionMap(textureManager->getImage(reflectionMaps[0], data));
		}

		data.useSRGB = false;
		//normal maps shouldn't use mipmaps (important for shading!)
		data.generateMipMaps = false;
		data.magFilter = Linear_Mipmap_Linear;
		data.minFilter = Linear_Mipmap_Linear;
		data.colorspace = RGBA;
		data.uvTechnique = Repeat;
		data.isFloatData = false;
		data.resolution = BITS_8;
		vector<string> normalMaps = loadMaterialTextures(mat, aiTextureType_HEIGHT, data);
		if (normalMaps.size())
		{
			material->setNormalMap(textureManager->getImage(normalMaps[0], data));
		}
	}

	material->setShininess(32);

	return move(material);
}