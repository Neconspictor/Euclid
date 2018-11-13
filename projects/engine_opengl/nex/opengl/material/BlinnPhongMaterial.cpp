#include <nex/opengl/material/BlinnPhongMaterial.hpp>
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

BlinnPhongMaterial::BlinnPhongMaterial(TextureGL* diffuseMap, TextureGL* emissionMap, TextureGL* normalMap, TextureGL* reflectionMap, TextureGL* specularMap, float shininess) :
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

TextureGL* BlinnPhongMaterial::getDiffuseMap() const
{
	return diffuseMap;
}

TextureGL* BlinnPhongMaterial::getEmissionMap() const
{
	return emissionMap;
}

TextureGL * BlinnPhongMaterial::getNormalMap() const
{
	return normalMap;
}

TextureGL* BlinnPhongMaterial::getReflectionMap() const
{
	return reflectionMap;
}

float BlinnPhongMaterial::getShininess() const
{
	return shininess;
}

TextureGL* BlinnPhongMaterial::getSpecularMap() const
{
	return specularMap;
}

void BlinnPhongMaterial::setDiffuseMap(TextureGL* diffuse)
{
	diffuseMap = diffuse;
}

void BlinnPhongMaterial::setEmissionMap(TextureGL* emission)
{
	emissionMap = emission;
}

void BlinnPhongMaterial::setNormalMap(TextureGL * normal)
{
	normalMap = normal;
}

void BlinnPhongMaterial::setReflectionMap(TextureGL* reflection)
{
	reflectionMap = reflection;
}

void BlinnPhongMaterial::setSpecularMap(TextureGL* specular)
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