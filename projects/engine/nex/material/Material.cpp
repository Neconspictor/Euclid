#include <nex/material/Material.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Shader.hpp>
#include "nex/shader/PBRShader.hpp"

using namespace std;
using namespace nex;


Material::Material() : mShader(nullptr)
{
}

Shader* Material::getProgram()
{
	return mShader;
}

void Material::setShader(Shader* shader)
{
	mShader = shader;
}

void Material::init(Shader* program)
{
	setShader(program);

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

void Material::set(UniformLocation loc, const glm::u32vec2& value)
{
	mUVec2s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::u32vec3& value)
{
	mUVec3s[loc] = value;
}

void Material::set(UniformLocation loc, const glm::u32vec4& value)
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

void Material::upload() const
{
	if (mShader == nullptr) return;

	mShader->bind();

	for (auto& uniform : mTextures)
	{
		// Note: samplers are not handled by materials. Thus no sampler object is submitted
		mShader->setTexture(uniform.second, nullptr, uniform.first);
	}

	for (auto& uniform : mFloats)
	{
		mShader->setFloat(uniform.first, uniform.second);
	}

	for (auto& uniform : mInts)
	{
		mShader->setInt(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat2s)
	{
		mShader->setMat3(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat3s)
	{
		mShader->setMat3(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat4s)
	{
		mShader->setMat4(uniform.first, uniform.second);
	}

	for (auto& uniform : mUints)
	{
		mShader->setUInt(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec2s)
	{
		mShader->setUVec2(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec3s)
	{
		mShader->setUVec3(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec4s)
	{
		mShader->setUVec4(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec2s)
	{
		mShader->setVec2(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec3s)
	{
		mShader->setVec3(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec4s)
	{
		mShader->setVec4(uniform.first, uniform.second);
	}
}

std::ostream& nex::operator<<(std::ostream& os, nex::MaterialType type)
{
	os << enumToString(type, materialEnumConversion);
	return os;
}

PbrMaterial::PbrMaterial():
	PbrMaterial(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)
{
}

PbrMaterial::PbrMaterial(
	Texture * albedoMap,
	Texture * aoMap,
	Texture * emissionMap,
	Texture * metallicMap,
	Texture * normalMap,
	Texture * roughnessMap)
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