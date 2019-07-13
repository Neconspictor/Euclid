#include <nex/material/Material.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/pbr/pbr.hpp>
#include <nex/pbr/PbrPass.hpp>
#include "nex/common/File.hpp"
#include "nex/texture/TextureManager.hpp"

using namespace std;
using namespace nex;


Material::Material(Technique* technique) : mTechnique(technique)
{
}

Material::~Material() = default;

RenderState& Material::getRenderState()
{
	return mRenderState;
}

Technique* Material::getTechnique()
{
	return mTechnique;
}

void Material::setTechnique(Technique* technique)
{
	mTechnique = technique;
}

void nex::Material::upload()
{
}

std::ostream& nex::operator<<(std::ostream& os, nex::MaterialType type)
{
	os << enumToString(type, materialEnumConversion);
	return os;
}

PbrMaterial::PbrMaterial(PbrTechnique* technique) :
	PbrMaterial(technique, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
{
}

PbrMaterial::PbrMaterial(
	PbrTechnique* technique,
	Texture * albedoMap,
	Texture * aoMap,
	Texture * emissionMap,
	Texture * metallicMap,
	Texture * normalMap,
	Texture * roughnessMap) :
	Material(technique)
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

void nex::PbrMaterial::upload()
{
	PbrTechnique* pbrTechnique = (PbrTechnique*)mTechnique;

	auto* pass = pbrTechnique->getActiveGeometryPass();
	auto* shaderInterface = pass->getShaderInterface();
	shaderInterface->setAlbedoMap(mAlbedoMap);
	shaderInterface->setAoMap(mAoMap);
	shaderInterface->setMetalMap(mMetallicMap);
	shaderInterface->setNormalMap(mNormalMap);
	shaderInterface->setRoughnessMap(mRoughnessMap);
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
		store.state.depthCompare = CompareFunction::GREATER_EQUAL;
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