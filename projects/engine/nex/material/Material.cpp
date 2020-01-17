#include <nex/material/Material.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/pbr/pbr.hpp>
#include <nex/pbr/PbrPass.hpp>
#include "nex/common/File.hpp"
#include "nex/texture/TextureManager.hpp"
#include <nex/effects/SimpleColorPass.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/shader/ShaderProvider.hpp>

using namespace std;
using namespace nex;


Material::Material(std::shared_ptr<ShaderProvider> provider) : mShaderProvider(std::move(provider))
{
}

Material::~Material() = default;

RenderState& Material::getRenderState()
{
	return mRenderState;
}

const RenderState& nex::Material::getRenderState() const
{
	return mRenderState;
}

nex::Shader* nex::Material::getShader()
{
	if (mShaderProvider)
		return mShaderProvider->getShader();
	return nullptr;
}

nex::Shader* nex::Material::getShader() const
{
	if (mShaderProvider)
		return mShaderProvider->getShader();
	return nullptr;
}

void nex::Material::setShaderProvider(std::shared_ptr<ShaderProvider> provider)
{
	mShaderProvider = std::move(provider);
}
std::ostream& nex::operator<<(std::ostream& os, nex::MaterialType type)
{
	os << enumToString(type, materialEnumConversion);
	return os;
}

PbrMaterial::PbrMaterial(std::shared_ptr<ShaderProvider> provider) :
	PbrMaterial(provider, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
{
}

PbrMaterial::PbrMaterial(
	std::shared_ptr<ShaderProvider> provider,
	Texture * albedoMap,
	Texture * aoMap,
	Texture * emissionMap,
	Texture * metallicMap,
	Texture * normalMap,
	Texture * roughnessMap) :
	Material(provider)
{
	setAlbedoMap(albedoMap);
	setAoMap(aoMap);
	setEmissionMap(emissionMap);
	setMetallicMap(metallicMap);
	setNormalMap(normalMap);
	setRoughnessMap(roughnessMap);
}

const Texture * PbrMaterial::getAlbedoMap() const
{
	return mAlbedoMap;
}

const Texture * PbrMaterial::getAoMap() const
{
	return mAoMap;
}

const Texture * PbrMaterial::getEmissionMap() const
{
	return mEmissionMap;
}

const Texture * PbrMaterial::getMetallicMap() const
{
	return mMetallicMap;
}

const Texture * PbrMaterial::getNormalMap() const
{
	return mNormalMap;
}

const Texture * PbrMaterial::getRoughnessMap() const
{
	return mRoughnessMap;
}

void PbrMaterial::setAlbedoMap(Texture * albedoMap)
{
	mAlbedoMap = albedoMap;
}

void PbrMaterial::setAoMap(Texture * aoMap)
{
	mAoMap = aoMap;
}

void PbrMaterial::setEmissionMap(Texture * emissionMap)
{
	mEmissionMap = emissionMap;
}

void PbrMaterial::setMetallicMap(Texture * metallicMap)
{
	mMetallicMap = metallicMap;
}

void PbrMaterial::setNormalMap(Texture * normalMap)
{
	mNormalMap = normalMap;
}

void PbrMaterial::setRoughnessMap(Texture * roughnessMap)
{
	mRoughnessMap = roughnessMap;
}

void MaterialStore::test()
{
	{
		MaterialStore store;
		store.albedoMap = "albedo";
		store.alphaMap = "alpha";
		store.aoMap = "ao";
		store.emissionMap = "emission";
		store.metallicMap = "metal";
		store.normalMap = "normal";
		store.roughnessMap = "roughness";
		store.state.doCullFaces = false;
		store.state.cullSide = PolygonSide::FRONT;
		store.state.doShadowCast = false;
		store.state.depthCompare = CompFunc::GREATER_EQUAL;
		store.state.fillMode = FillMode::LINE;
		nex::BinStream file;
		file.open("material.bin", std::ios::out | std::ios::trunc);
		file << store;

	}

	{
		MaterialStore store;
		nex::BinStream file;
		file.open("material.bin", std::ios::in);
		file >> store;

		std::cout << "store.albedoMap : " << store.albedoMap << std::endl;
		std::cout << "store.alphaMap : " << store.alphaMap << std::endl;
		std::cout << "store.aoMap : " << store.aoMap << std::endl;
		std::cout << "store.emissionMap : " << store.emissionMap << std::endl;
		std::cout << "store.metallicMap : " << store.metallicMap << std::endl;
		std::cout << "store.normalMap : " << store.normalMap << std::endl;
		std::cout << "store.roughnessMap : " << store.roughnessMap << std::endl;
		std::cout << "store.state.doCullFaces : " << store.state.doCullFaces << std::endl;

	}
}

nex::BinStream& nex::operator>>(nex::BinStream& in, MaterialStore& store)
{
	in >> store.albedoMap;
	in >> store.alphaMap;
	in >> store.aoMap;
	in >> store.diffuseColor;
	in >> store.emissionMap;
	in >> store.metallicMap;
	in >> store.normalMap;
	in >> store.roughnessMap;
	in >> store.type;
	in >> store.state;

	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const MaterialStore& store)
{
	out << store.albedoMap;
	out << store.alphaMap;
	out << store.aoMap;
	out << store.diffuseColor;
	out << store.emissionMap;
	out << store.metallicMap;
	out << store.normalMap;
	out << store.roughnessMap;
	out << store.type;
	out << store.state;
	return out;
}

nex::SimpleColorMaterial::SimpleColorMaterial(std::shared_ptr<ShaderProvider> provider) :
	Material(std::move(provider)), mColor(1.0f)
{
	mRenderState.blendDesc = BlendDesc::createAlphaTransparency();

	setColor(mColor);
}

void nex::SimpleColorMaterial::setColor(const glm::vec4& color)
{
	mColor = color;
	mRenderState.doBlend = mColor.w != 1.0f;
}

const glm::vec4& nex::SimpleColorMaterial::getColor() const
{
	return mColor;
}