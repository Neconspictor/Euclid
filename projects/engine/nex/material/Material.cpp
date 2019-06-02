#include <nex/material/Material.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Shader.hpp>
#include <nex/pbr/PbrPass.hpp>
#include "nex/common/File.hpp"

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

void Material::clear()
{
	mFloats.clear();
	mInts.clear();
	mMat2s.clear();
	mMat3s.clear();
	mMat4s.clear();
	mUints.clear();
	mUVec2s.clear();
	mUVec3s.clear();
	mUVec4s.clear();
	mVec2s.clear();
	mVec3s.clear();
	mVec4s.clear();
	mTextures.clear();
}

void Material::set(UniformLocation loc, float value)
{
	mFloats[loc] = value;
}

void Material::set(UniformLocation loc, int value)
{
	mInts[loc] = value;
}

void Material::set(UniformLocation loc, const glm::mat2& value)
{
	mMat2s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::mat3& value)
{
	mMat3s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::mat4& value)
{
	mMat4s[loc] = value;
}

void Material::set(UniformLocation loc, unsigned value)
{
	mUints[loc] = value;
}

void Material::set(UniformLocation loc, const glm::uvec2& value)
{
	mUVec2s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::uvec3& value)
{
	mUVec3s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::uvec4& value)
{
	mUVec4s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::vec2& value)
{
	mVec2s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::vec3& value)
{
	mVec3s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::vec4& value)
{
	mVec4s[loc] = value;
}

void Material::set(unsigned bindingSlot, const Texture* texture)
{
	mTextures[bindingSlot] = texture;
}

void Material::upload(Shader* shader) const
{
	if (shader == nullptr) return;

	shader->bind();

	for (auto& uniform : mTextures)
	{
		// Note: samplers are not handled by materials. Thus no sampler object is submitted
		shader->setTexture(uniform.second, nullptr, uniform.first);
	}

	for (auto& uniform : mFloats)
	{
		shader->setFloat(uniform.first, uniform.second);
	}

	for (auto& uniform : mInts)
	{
		shader->setInt(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat2s)
	{
		shader->setMat3(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat3s)
	{
		shader->setMat3(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat4s)
	{
		shader->setMat4(uniform.first, uniform.second);
	}

	for (auto& uniform : mUints)
	{
		shader->setUInt(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec2s)
	{
		shader->setUVec2(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec3s)
	{
		shader->setUVec3(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec4s)
	{
		shader->setUVec4(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec2s)
	{
		shader->setVec2(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec3s)
	{
		shader->setVec3(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec4s)
	{
		shader->setVec4(uniform.first, uniform.second);
	}
}

std::ostream& nex::operator<<(std::ostream& os, nex::MaterialType type)
{
	os << enumToString(type, materialEnumConversion);
	return os;
}

PbrMaterial::PbrMaterial(Technique* technique):
	PbrMaterial(technique, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
{
}

PbrMaterial::PbrMaterial(
	Technique* technique,
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
	return mTextures.at(PbrCommonGeometryPass::ALBEDO_BINDING_POINT);
}

const Texture * PbrMaterial::getAoMap() const
{
	return mTextures.at(PbrCommonGeometryPass::AO_BINDING_POINT);
}

const Texture * PbrMaterial::getEmissionMap() const
{
	return nullptr;
}

const Texture * PbrMaterial::getMetallicMap() const
{
	return mTextures.at(PbrCommonGeometryPass::METALLIC_BINDING_POINT);
}

const Texture * PbrMaterial::getNormalMap() const
{
	return mTextures.at(PbrCommonGeometryPass::NORMAL_BINDING_POINT);
}

const Texture * PbrMaterial::getRoughnessMap() const
{
	return mTextures.at(PbrCommonGeometryPass::ROUGHNESS_BINDING_POINT);
}

void PbrMaterial::setAlbedoMap(Texture * albedoMap)
{
	set(PbrCommonGeometryPass::ALBEDO_BINDING_POINT, albedoMap);
}

void PbrMaterial::setAoMap(Texture * aoMap)
{
	set(PbrCommonGeometryPass::AO_BINDING_POINT, aoMap);
}

void PbrMaterial::setEmissionMap(Texture * emissionMap)
{
}

void PbrMaterial::setMetallicMap(Texture * metallicMap)
{
	set(PbrCommonGeometryPass::METALLIC_BINDING_POINT, metallicMap);
}

void PbrMaterial::setNormalMap(Texture * normalMap)
{
	set(PbrCommonGeometryPass::NORMAL_BINDING_POINT, normalMap);
}

void PbrMaterial::setRoughnessMap(Texture * roughnessMap)
{
	set(PbrCommonGeometryPass::ROUGHNESS_BINDING_POINT, roughnessMap);
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
	out << store.emissionMap;
	out << store.metallicMap;
	out << store.normalMap;
	out << store.roughnessMap;
	out << store.type;
	out << store.state;
	return out;
}