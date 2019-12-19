#include <nex/material/PbrMaterialLoader.hpp>
#include <nex/material/Material.hpp>
#include <string>
#include <nex/texture/Texture.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/pbr/PbrDeferred.hpp>


using namespace std;
using namespace nex;


PbrMaterialLoader::PbrMaterialLoader(std::shared_ptr<PbrShaderProvider> provider,
	TextureManager* textureManager,
	LoadMode mode) : 
	AbstractMaterialLoader(textureManager),
mProvider(std::move(provider)), mMode(mode)
{
}

PbrMaterialLoader::~PbrMaterialLoader() = default;

void nex::PbrMaterialLoader::setLoadMode(LoadMode mode)
{
	mMode = mode;
}

std::unique_ptr<Material> PbrMaterialLoader::createMaterial(const MaterialStore& store) const
{
	auto material = std::make_unique<PbrMaterial>(mProvider);

	TextureDesc data = {
		TexFilter::Linear_Mipmap_Linear,
		TexFilter::Linear,
		UVTechnique::Repeat,
		UVTechnique::Repeat,
		UVTechnique::Repeat,
		ColorSpace::SRGB,
		PixelDataType::UBYTE,
		InternalFormat::SRGB8,
		true
	};


	Texture2D* albedoMap = nullptr;

	// a material can have more than one diffuse/specular/normal map,
	// but we only use the first one by now
	if (store.albedoMap != "")
	{
		albedoMap = textureManager->getImage(store.albedoMap, data, true);
	}
	else
	{
		albedoMap = textureManager->getDefaultWhiteTexture(); // assume white material
	}


	material->setAlbedoMap(albedoMap);


	//if (getComponents(albedoMap->getTextureData().colorspace) == 4) {
		if (mMode == LoadMode::SOLID_ALPHA_STENCIL) {
			material->getRenderState().doCullFaces = false;
		}
		else if (mMode == LoadMode::ALPHA_TRANSPARENCY) {
			material->getRenderState().doBlend = true;
			material->getRenderState().blendDesc = BlendDesc::createAlphaTransparency();
		}
	//}

	if (store.aoMap != "")
	{
		material->setAoMap(textureManager->getImage(store.aoMap, data, true));
	}
	else
	{
		material->setAoMap(textureManager->getDefaultWhiteTexture()); // no ao
	}

	if (store.emissionMap != "")
	{
		material->setEmissionMap(textureManager->getImage(store.emissionMap, data, true));
	}
	else
	{
		material->setEmissionMap(textureManager->getDefaultBlackTexture()); // no emission
	}

	// the following textures are linear, so we use the RGBA color space
	data.colorspace = ColorSpace::RGB;
	data.internalFormat = InternalFormat::RGB8;

	if (store.metallicMap != "")
	{
		material->setMetallicMap(textureManager->getImage(store.metallicMap, data, true));
	}
	else
	{
		material->setMetallicMap(textureManager->getDefaultBlackTexture()); // we assume a non metallic material
	}

	if (store.roughnessMap != "")
	{
		material->setRoughnessMap(textureManager->getImage(store.roughnessMap, data, true));
	}
	else
	{
		material->setRoughnessMap(textureManager->getDefaultWhiteTexture()); // we assume a full rough material
	}


	//normal maps shouldn't use mipmaps (important for shading!)
	data.generateMipMaps = true; // TODO use mip maps or not????


	if (store.normalMap != "")
	{
		Texture* texture = textureManager->getImage(store.normalMap, data, true);
		material->setNormalMap(texture);
		//material->setNormalMap(textureManager->getDefaultNormalTexture());
	}
	else
	{
		Texture* texture = textureManager->getDefaultNormalTexture();
		material->setNormalMap(texture);
	}

	return material;
}

void PbrMaterialLoader::loadShadingMaterial(const std::filesystem::path& meshPath, const aiScene * scene, MaterialStore& store, unsigned materialIndex) const
{
	if (scene->mNumMaterials <= materialIndex)
	{
		throw_with_trace(std::runtime_error("PbrMaterialLoader: materialIndex out of range!"));
	}

	aiMaterial* mat = scene->mMaterials[materialIndex];

	// a material can have more than one diffuse/specular/normal map,
	// but we only use the first one by now
	vector<string> albedoMaps = loadMaterialTextures(meshPath, mat, aiTextureType_DIFFUSE);
	if (albedoMaps.size())
	{
		store.albedoMap = albedoMaps[0];
	}

	vector<string> aoMaps = loadMaterialTextures(meshPath, mat, aiTextureType_AMBIENT);
	if (aoMaps.size())
	{
		store.aoMap = aoMaps[0];
	}

	vector<string> emissionMaps = loadMaterialTextures(meshPath, mat, aiTextureType_EMISSIVE);
	if (emissionMaps.size())
	{
		store.emissionMap = emissionMaps[0];
	}

	vector<string> metallicMaps = loadMaterialTextures(meshPath, mat, aiTextureType_SPECULAR);
	if (metallicMaps.size())
	{
		store.metallicMap = metallicMaps[0];
	}

	vector<string> roughnessMaps = loadMaterialTextures(meshPath, mat, aiTextureType_SHININESS);
	if (roughnessMaps.size())
	{
		store.roughnessMap = roughnessMaps[0];
	}

	vector<string> normalMaps = loadMaterialTextures(meshPath, mat, aiTextureType_HEIGHT);
	if (normalMaps.size())
	{
		store.normalMap = normalMaps[0];
	}
}