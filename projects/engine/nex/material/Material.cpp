#include <nex/material/Material.hpp>
#include <nex/texture/Texture.hpp>

using namespace std;
using namespace nex;


std::ostream& nex::operator<<(std::ostream& os, nex::MaterialType type)
{
	os << enumToString(type, Material::materialEnumConversion);
	return os;
}

BlinnPhongMaterial::BlinnPhongMaterial() : Material(),
diffuseMap(nullptr),
emissionMap(nullptr),
normalMap(nullptr),
reflectionMap(nullptr),
specularMap(nullptr),
shininess(0)
{
}

BlinnPhongMaterial::BlinnPhongMaterial(Texture* diffuseMap, Texture* emissionMap, Texture* normalMap, Texture* reflectionMap, Texture* specularMap, float shininess) :
	Material()
{
	this->diffuseMap = diffuseMap;
	this->emissionMap = emissionMap;
	this->reflectionMap = reflectionMap;
	this->specularMap = specularMap;
	this->shininess = shininess;
}

BlinnPhongMaterial::BlinnPhongMaterial(const BlinnPhongMaterial& other) : Material(),
diffuseMap(other.diffuseMap),
emissionMap(other.emissionMap),
normalMap(other.normalMap),
reflectionMap(other.reflectionMap),
specularMap(other.specularMap),
shininess(other.shininess)
{}

BlinnPhongMaterial::BlinnPhongMaterial(BlinnPhongMaterial&& other)
{
	diffuseMap = move(other.diffuseMap);
	emissionMap = move(other.emissionMap);
	normalMap = move(other.normalMap);
	reflectionMap = move(other.reflectionMap);
	specularMap = move(other.specularMap);
	shininess = move(other.shininess);
}

BlinnPhongMaterial& BlinnPhongMaterial::operator=(const BlinnPhongMaterial& other)
{
	diffuseMap = other.diffuseMap;
	emissionMap = other.emissionMap;
	normalMap = other.normalMap;
	reflectionMap = other.reflectionMap;
	specularMap = other.specularMap;
	shininess = other.shininess;
	return *this;
}

BlinnPhongMaterial& BlinnPhongMaterial::operator=(BlinnPhongMaterial&& other)
{
	diffuseMap = move(other.diffuseMap);
	emissionMap = move(other.emissionMap);
	normalMap = move(other.normalMap);
	reflectionMap = move(other.reflectionMap);
	specularMap = move(other.specularMap);
	shininess = move(other.shininess);
	return *this;
}

BlinnPhongMaterial::~BlinnPhongMaterial()
{
}

Texture* BlinnPhongMaterial::getDiffuseMap() const
{
	return diffuseMap;
}

Texture* BlinnPhongMaterial::getEmissionMap() const
{
	return emissionMap;
}

Texture * BlinnPhongMaterial::getNormalMap() const
{
	return normalMap;
}

Texture* BlinnPhongMaterial::getReflectionMap() const
{
	return reflectionMap;
}

float BlinnPhongMaterial::getShininess() const
{
	return shininess;
}

Texture* BlinnPhongMaterial::getSpecularMap() const
{
	return specularMap;
}

void BlinnPhongMaterial::setDiffuseMap(Texture* diffuse)
{
	diffuseMap = diffuse;
}

void BlinnPhongMaterial::setEmissionMap(Texture* emission)
{
	emissionMap = emission;
}

void BlinnPhongMaterial::setNormalMap(Texture * normal)
{
	normalMap = normal;
}

void BlinnPhongMaterial::setReflectionMap(Texture* reflection)
{
	reflectionMap = reflection;
}

void BlinnPhongMaterial::setSpecularMap(Texture* specular)
{
	specularMap = specular;
}

void BlinnPhongMaterial::setShininess(float shininess)
{
	this->shininess = shininess;
}

const float& BlinnPhongMaterial::getShininessRef() const
{
	return shininess;
}



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