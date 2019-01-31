#include <nex/opengl/material/BlinnPhongMaterialLoader.hpp>
#include <string>
#include <nex/opengl/material/Material.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>

using namespace std;
using namespace nex;


BlinnPhongMaterialLoader::BlinnPhongMaterialLoader(TextureManagerGL* textureManager) : AbstractMaterialLoader(textureManager), m_logger("BlinnPhongMaterialLoader")
{
}

std::vector<std::unique_ptr<Material>> BlinnPhongMaterialLoader::loadShadingMaterial(const aiScene* scene) const
{
	std::vector<std::unique_ptr<Material>> materials;

	// process material (if any available)
	for (unsigned i = 0; i < scene->mNumMaterials; ++i)
	{
		auto material = make_unique<BlinnPhongMaterial>();
		aiMaterial* mat = scene->mMaterials[i];
		
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



		// the following textures are linear, so we use the RGBA color space
		data.colorspace = ColorSpace::RGBA;
		data.internalFormat = InternFormat::RGBA8;

		vector<string> specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, data);
		if (specularMaps.size())
		{
			material->setSpecularMap(textureManager->getImage(specularMaps[0], data));
		}

		vector<string> reflectionMaps = loadMaterialTextures(mat, aiTextureType_AMBIENT, data);
		if (reflectionMaps.size())
		{
			material->setReflectionMap(textureManager->getImage(reflectionMaps[0], data));
		}

		//normal maps shouldn't use mipmaps (important for shading!)
		data.generateMipMaps = false;
		data.magFilter = TextureFilter::Linear;

		vector<string> normalMaps = loadMaterialTextures(mat, aiTextureType_HEIGHT, data);
		if (normalMaps.size())
		{
			material->setNormalMap(textureManager->getImage(normalMaps[0], data));
		}

		material->setShininess(32);

		materials.emplace_back(std::move(material));
	}

	return materials;
}