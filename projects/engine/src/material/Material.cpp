#include <material/Material.hpp>
#include <type_traits>

using namespace std;

Material::Material() : diffuseMap(nullptr), emissionMap(nullptr), specularMap(nullptr), shininess(0)
{
}

Material::Material(Texture* diffuseMap, Texture* emissionMap, Texture* specularMap, float shininess)
{
	this->diffuseMap = diffuseMap;
	this->emissionMap = emissionMap;
	this->specularMap = specularMap;
	this->shininess = shininess;
}

Material::Material(const Material& other) :
diffuseMap(other.diffuseMap), emissionMap(other.emissionMap), specularMap(other.specularMap), 
shininess(other.shininess)
{}

Material::Material(Material&& other)
{
	diffuseMap = move(other.diffuseMap);
	emissionMap = move(other.emissionMap);
	specularMap = move(other.specularMap);
	shininess = move(other.shininess);
}

Material& Material::operator=(const Material& other)
{
	diffuseMap = other.diffuseMap;
	emissionMap = other.emissionMap;
	specularMap = other.specularMap;
	shininess = other.shininess;
	return *this;
}

Material& Material::operator=(Material&& other)
{
	diffuseMap = move(other.diffuseMap);
	emissionMap = move(other.emissionMap);
	specularMap = move(other.specularMap);
	shininess = move(other.shininess);
	return *this;
}

Material::~Material()
{
}

Texture* Material::getDiffuseMap() const
{
	return diffuseMap;
}

Texture* Material::getEmissionMap() const
{
	return emissionMap;
}

float Material::getShininess() const
{
	return shininess;
}

Texture* Material::getSpecularMap() const
{
	return specularMap;
}

void Material::setDiffuseMap(Texture* diffuse)
{
	diffuseMap = diffuse;
}

void Material::setEmissionMap(Texture* emission)
{
	emissionMap = emission;
}

void Material::setSpecularMap(Texture* specular)
{
	specularMap = specular;
}

void Material::setShininess(float shininess)
{
	this->shininess = shininess;
}