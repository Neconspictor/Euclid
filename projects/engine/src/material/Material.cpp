#include <material/Material.hpp>

using namespace std;

Material::Material() : diffuseMap(""), emissionMap(""), specularMap(""), shininess(0)
{
}

Material::Material(string diffuseMap, string emissionMap, string specularMap, float shininess)
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

const string& Material::getDiffuseMap() const
{
	return diffuseMap;
}

const string& Material::getEmissionMap() const
{
	return emissionMap;
}

float Material::getShininess() const
{
	return shininess;
}

const string& Material::getSpecularMap() const
{
	return specularMap;
}

void Material::setDiffuseMap(string diffuse)
{
	diffuseMap = move(diffuse);
}

void Material::setEmissionMap(string emission)
{
	emissionMap = move(emission);
}

void Material::setSpecularMap(std::string specular)
{
	specularMap = move(specular);
}

void Material::setShininess(float shininess)
{
	this->shininess = shininess;
}