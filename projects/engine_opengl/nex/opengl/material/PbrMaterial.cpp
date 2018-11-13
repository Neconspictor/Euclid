#include <nex/opengl/material/PbrMaterial.hpp>

using namespace std;

PbrMaterial::PbrMaterial():
albedoMap(nullptr),
aoMap(nullptr),
emissionMap(nullptr),
metallicMap(nullptr),
normalMap(nullptr),
roughnessMap(nullptr)
{
}

PbrMaterial::PbrMaterial(TextureGL * albedoMap,
	TextureGL * aoMap,
	TextureGL * emissionMap,
	TextureGL * metallicMap,
	TextureGL * normalMap,
	TextureGL * roughnessMap) :

	albedoMap(albedoMap),
	aoMap(aoMap),
	emissionMap(emissionMap),
	metallicMap(metallicMap),
	normalMap(normalMap),
	roughnessMap(roughnessMap)
{
}

PbrMaterial::PbrMaterial(const PbrMaterial & other) : Material(), 
albedoMap(other.albedoMap), aoMap(other.aoMap),
emissionMap(other.emissionMap), metallicMap(other.metallicMap), 
normalMap(other.normalMap),roughnessMap(other.roughnessMap)
{
}

PbrMaterial & PbrMaterial::operator=(const PbrMaterial & other)
{
	if (this != &other) {
		this->albedoMap = other.albedoMap;
		this->aoMap = other.aoMap;
		this->emissionMap = other.emissionMap;
		this->metallicMap = other.metallicMap;
		this->normalMap = other.normalMap;
		this->roughnessMap = other.roughnessMap;
	}
	return *this;
}

TextureGL * PbrMaterial::getAlbedoMap() const
{
	return albedoMap;
}

TextureGL * PbrMaterial::getAoMap() const
{
	return aoMap;
}

TextureGL * PbrMaterial::getEmissionMap() const
{
	return emissionMap;
}

TextureGL * PbrMaterial::getMetallicMap() const
{
	return metallicMap;
}

TextureGL * PbrMaterial::getNormalMap() const
{
	return normalMap;
}

TextureGL * PbrMaterial::getRoughnessMap() const
{
	return roughnessMap;
}

void PbrMaterial::setAlbedoMap(TextureGL * albedoMap)
{
	this->albedoMap = albedoMap;
}

void PbrMaterial::setAoMap(TextureGL * aoMap)
{
	this->aoMap = aoMap;
}

void PbrMaterial::setEmissionMap(TextureGL * emissionMap)
{
	this->emissionMap = emissionMap;
}

void PbrMaterial::setMetallicMap(TextureGL * metallicMap)
{
	this->metallicMap = metallicMap;
}

void PbrMaterial::setNormalMap(TextureGL * normalMap)
{
	this->normalMap = normalMap;
}

void PbrMaterial::setRoughnessMap(TextureGL * roughnessMap)
{
	this->roughnessMap = roughnessMap;
}