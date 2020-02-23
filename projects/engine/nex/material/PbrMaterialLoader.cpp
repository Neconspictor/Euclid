#include <nex/material/PbrMaterialLoader.hpp>
#include <nex/material/Material.hpp>
#include <string>
#include <nex/texture/Texture.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/pbr/PbrDeferred.hpp>
#include <assimp/pbrmaterial.h>


using namespace std;
using namespace nex;



static TextureDesc SRGB_DESC = {
		TexFilter::Linear_Mipmap_Linear,
		TexFilter::Linear,
		UVTechnique::Repeat,
		UVTechnique::Repeat,
		UVTechnique::Repeat,
		InternalFormat::SRGBA8,
		true
};


static TextureDesc RGB_DESC = {
		TexFilter::Linear_Mipmap_Linear,
		TexFilter::Linear,
		UVTechnique::Repeat,
		UVTechnique::Repeat,
		UVTechnique::Repeat,
		InternalFormat::RGBA8,
		true,
		true // auto swizzle for gray channels
};



static TextureDesc RGB_DESC_NO_MIP = {
		TexFilter::Linear,
		TexFilter::Linear,
		UVTechnique::Repeat,
		UVTechnique::Repeat,
		UVTechnique::Repeat,
		InternalFormat::RGB8,
		false
};

PbrMaterialLoader::PbrMaterialLoader(std::shared_ptr<PbrShaderProvider> staticMeshShaderProvider,
	std::shared_ptr<PbrShaderProvider> skinnedMeshShaderProvider,
	TextureManager* textureManager,
	LoadMode mode) : 
	AbstractMaterialLoader(textureManager),
	mStaticMeshShaderProvider(std::move(staticMeshShaderProvider)), 
	mSkinnedMeshShaderProvider(std::move(skinnedMeshShaderProvider)),
	mMode(mode)
{
}

PbrMaterialLoader::~PbrMaterialLoader() = default;

void nex::PbrMaterialLoader::setLoadMode(LoadMode mode)
{
	mMode = mode;
}

std::unique_ptr<Material> PbrMaterialLoader::createMaterial(const MaterialStore& store) const
{
	std::unique_ptr<PbrMaterial> material;

	if (store.isSkinned) {
		material = std::make_unique<PbrMaterial>(mSkinnedMeshShaderProvider);
	}
	else {
		material = std::make_unique<PbrMaterial>(mStaticMeshShaderProvider);
	}



	Texture2D* albedoMap = nullptr;

	// a material can have more than one diffuse/specular/normal map,
	// but we only use the first one by now
	if (store.albedoMap != "")
	{
		albedoMap = textureManager->getImage(store.albedoMap, true, SRGB_DESC, true);
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
			material->getRenderState().doCullFaces = false;
			material->getRenderState().blendDesc = BlendDesc::createAlphaTransparency();
		}
	//}

	if (store.emissionMap != "")
	{
		material->setEmissionMap(textureManager->getImage(store.emissionMap, true, SRGB_DESC, true));
	}
	else
	{
		material->setEmissionMap(textureManager->getDefaultBlackTexture()); // no emission
	}

	// the following textures are linear and have no alpha, so we use the RGB color space

	if (store.aoMap != "")
	{
		material->setAoMap(textureManager->getImage(store.aoMap, true, RGB_DESC, true));
	}
	else
	{
		material->setAoMap(textureManager->getDefaultWhiteTexture()); // no ao
	}


	if (store.metallicMap != "")
	{
		material->setMetallicMap(textureManager->getImage(store.metallicMap, true, RGB_DESC, true));
	}
	else
	{
		material->setMetallicMap(textureManager->getDefaultBlackTexture()); // we assume a non metallic material
	}

	if (store.roughnessMap != "")
	{
		material->setRoughnessMap(textureManager->getImage(store.roughnessMap, true, RGB_DESC, true));
	}
	else
	{
		material->setRoughnessMap(textureManager->getDefaultWhiteTexture()); // we assume a full rough material
	}


	//normal maps shouldn't use mipmaps (important for shading!)
	// TODO use mip maps or not???

	if (store.normalMap != "")
	{
		Texture* texture = textureManager->getImage(store.normalMap, true, RGB_DESC, true);
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

void PbrMaterialLoader::loadShadingMaterial(const std::filesystem::path& meshPath, const aiScene * scene, MaterialStore& store, unsigned materialIndex, bool isSkinned) const
{
	if (scene->mNumMaterials <= materialIndex)
	{
		throw_with_trace(std::runtime_error("PbrMaterialLoader: materialIndex out of range!"));
	}

	aiMaterial* mat = scene->mMaterials[materialIndex];

	store.isSkinned = isSkinned;

	std::vector<aiTexture*> texs(scene->mNumTextures);
	for (int i = 0; i < scene->mNumTextures; ++i) {
		texs[i] = scene->mTextures[i];
	}

	// a material can have more than one diffuse/specular/normal map,
	// but we only use the first one by now
	vector<string> albedoMaps = loadMaterialTextures(scene, meshPath, mat, aiTextureType_DIFFUSE);
	if (albedoMaps.size())
	{
		store.albedoMap = albedoMaps[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.albedoMap), meshPath, scene, SRGB_DESC, true);
	}

	vector<string> aoMaps = loadMaterialTextures(scene, meshPath, mat, aiTextureType_AMBIENT);
	vector<string> aoMaps2 = loadMaterialTextures(scene, meshPath, mat, aiTextureType_LIGHTMAP);
	if (aoMaps.size())
	{
		store.aoMap = aoMaps[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.aoMap), meshPath, scene, RGB_DESC, true);
	}
	else if (aoMaps2.size()) {
		store.aoMap = aoMaps2[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.aoMap), meshPath, scene, RGB_DESC, true);
	}

	vector<string> emissionMaps = loadMaterialTextures(scene, meshPath, mat, aiTextureType_EMISSIVE);
	vector<string> emissionMaps2 = loadMaterialTextures(scene, meshPath, mat, aiTextureType_EMISSION_COLOR);
	if (emissionMaps.size())
	{
		store.emissionMap = emissionMaps[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.emissionMap), meshPath, scene, SRGB_DESC, true);
	}
	else if (emissionMaps2.size()) {
		store.emissionMap = emissionMaps2[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.emissionMap), meshPath, scene, SRGB_DESC, true);
	}

	vector<string> metallicMaps = loadMaterialTextures(scene, meshPath, mat, aiTextureType_SPECULAR);
	vector<string> metallicMaps2 = loadMaterialTextures(scene, meshPath, mat, aiTextureType_UNKNOWN); //metalRoughness
	if (metallicMaps.size())
	{
		store.metallicMap = metallicMaps[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.metallicMap), meshPath, scene, RGB_DESC, true);
	}
	else if (metallicMaps2.size()) {
		store.metallicMap = metallicMaps2[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.metallicMap), meshPath, scene, RGB_DESC, true);
	}

	vector<string> roughnessMaps = loadMaterialTextures(scene, meshPath, mat, aiTextureType_SHININESS);
	vector<string> roughnessMaps2 = loadMaterialTextures(scene, meshPath, mat, aiTextureType_UNKNOWN);  //metalRoughness

	
	if (roughnessMaps.size())
	{
		store.roughnessMap = roughnessMaps[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.roughnessMap), meshPath, scene, RGB_DESC, true);
	}
	else if (roughnessMaps2.size()) {
		store.roughnessMap = roughnessMaps2[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.roughnessMap), meshPath, scene, RGB_DESC, true);
	}

	vector<string> normalMaps = loadMaterialTextures(scene, meshPath, mat, aiTextureType_HEIGHT);
	vector<string> normalMaps2 = loadMaterialTextures(scene, meshPath, mat, aiTextureType_NORMALS);
	if (normalMaps.size())
	{
		store.normalMap = normalMaps[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.normalMap), meshPath, scene, RGB_DESC, true);
	}
	else if (normalMaps2.size()) {
		store.normalMap = normalMaps2[0];
		loadOptionalEmbeddedTexture(std::filesystem::path(store.normalMap), meshPath, scene, RGB_DESC, true);
	}
}