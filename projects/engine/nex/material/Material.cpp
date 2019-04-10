#include <nex/material/Material.hpp>
#include <nex/texture/Texture.hpp>
#include <nex/shader/Shader.hpp>

using namespace std;
using namespace nex;


Material::Material() : mProgram(nullptr)
{
}

ShaderProgram* Material::getProgram()
{
	return mProgram;
}

void Material::setProgram(ShaderProgram* program)
{
	mProgram = program;
}

void Material::init(ShaderProgram* program)
{
	setProgram(program);

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
	if (mProgram == nullptr) return;

	mProgram->bind();

	for (auto& uniform : mTextures)
	{
		mProgram->setTexture(uniform.second, uniform.first);
	}

	for (auto& uniform : mFloats)
	{
		mProgram->setFloat(uniform.first, uniform.second);
	}

	for (auto& uniform : mInts)
	{
		mProgram->setInt(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat2s)
	{
		mProgram->setMat3(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat3s)
	{
		mProgram->setMat3(uniform.first, uniform.second);
	}

	for (auto& uniform : mMat4s)
	{
		mProgram->setMat4(uniform.first, uniform.second);
	}

	for (auto& uniform : mUints)
	{
		mProgram->setUInt(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec2s)
	{
		mProgram->setUVec2(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec3s)
	{
		mProgram->setUVec3(uniform.first, uniform.second);
	}

	for (auto& uniform : mUVec4s)
	{
		mProgram->setUVec4(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec2s)
	{
		mProgram->setVec2(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec3s)
	{
		mProgram->setVec3(uniform.first, uniform.second);
	}

	for (auto& uniform : mVec4s)
	{
		mProgram->setVec4(uniform.first, uniform.second);
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
	mTextures[0] = albedoMap;
	mTextures[1] = aoMap;
	mTextures[2] = metallicMap;
	mTextures[3] = normalMap;
	mTextures[4] = roughnessMap;
}

const Texture * PbrMaterial::getAlbedoMap() const
{
	return mTextures.at(0);
}

const Texture * PbrMaterial::getAoMap() const
{
	return mTextures.at(1);
}

const Texture * PbrMaterial::getEmissionMap() const
{
	return nullptr;
}

const Texture * PbrMaterial::getMetallicMap() const
{
	return mTextures.at(2);
}

const Texture * PbrMaterial::getNormalMap() const
{
	return mTextures.at(3);
}

const Texture * PbrMaterial::getRoughnessMap() const
{
	return mTextures.at(4);
}

void PbrMaterial::setAlbedoMap(Texture * albedoMap)
{
	mTextures[0] = albedoMap;
}

void PbrMaterial::setAoMap(Texture * aoMap)
{
	mTextures[1] = aoMap;
}

void PbrMaterial::setEmissionMap(Texture * emissionMap)
{
}

void PbrMaterial::setMetallicMap(Texture * metallicMap)
{
	mTextures[2] = metallicMap;
}

void PbrMaterial::setNormalMap(Texture * normalMap)
{
	mTextures[3] = normalMap;
}

void PbrMaterial::setRoughnessMap(Texture * roughnessMap)
{
	mTextures[4] = roughnessMap;
}