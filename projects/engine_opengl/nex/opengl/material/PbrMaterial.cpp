#include <nex/opengl/material/PbrMaterial.hpp>


using namespace std;
using namespace nex;


PbrMaterial::PbrMaterial():
albedoMap(nullptr),
aoMap(nullptr),
emissionMap(nullptr),
metallicMap(nullptr),
normalMap(nullptr),
roughnessMap(nullptr)
{
}

PbrMaterial::PbrMaterial(Texture * albedoMap,
	Texture * aoMap,
	Texture * emissionMap,
	Texture * metallicMap,
	Texture * normalMap,
	Texture * roughnessMap) :

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

Texture * PbrMaterial::getAlbedoMap() const
{
	return albedoMap;
}

Texture * PbrMaterial::getAoMap() const
{
	return aoMap;
}

Texture * PbrMaterial::getEmissionMap() const
{
	return emissionMap;
}

Texture * PbrMaterial::getMetallicMap() const
{
	return metallicMap;
}

Texture * PbrMaterial::getNormalMap() const
{
	return normalMap;
}

Texture * PbrMaterial::getRoughnessMap() const
{
	return roughnessMap;
}

void PbrMaterial::setAlbedoMap(Texture * albedoMap)
{
	this->albedoMap = albedoMap;
}

void PbrMaterial::setAoMap(Texture * aoMap)
{
	this->aoMap = aoMap;
}

void PbrMaterial::setEmissionMap(Texture * emissionMap)
{
	this->emissionMap = emissionMap;
}

void PbrMaterial::setMetallicMap(Texture * metallicMap)
{
	this->metallicMap = metallicMap;
}

void PbrMaterial::setNormalMap(Texture * normalMap)
{
	this->normalMap = normalMap;
}

void PbrMaterial::setRoughnessMap(Texture * roughnessMap)
{
	this->roughnessMap = roughnessMap;
}