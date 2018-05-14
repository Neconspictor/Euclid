#include <material/BlinnPhongMaterial.hpp>
#include <type_traits>

using namespace std;

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