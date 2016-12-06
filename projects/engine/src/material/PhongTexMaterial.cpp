#include <material/PhongTexMaterial.hpp>

using namespace glm;
using namespace std;

PhongTexMaterial::PhongTexMaterial(string diffuseMap, string emissionMap, string specularMap, float shininess)
{
	this->diffuseMap = diffuseMap;
	this->emissionMap = emissionMap;
	this->specularMap = specularMap;
	this->shininess = shininess;
}

PhongTexMaterial::PhongTexMaterial(const PhongTexMaterial& other) :
diffuseMap(other.diffuseMap), emissionMap(other.emissionMap), specularMap(other.specularMap), 
shininess(other.shininess)
{}

PhongTexMaterial::PhongTexMaterial(PhongTexMaterial&& other)
{
	diffuseMap = move(other.diffuseMap);
	emissionMap = move(other.emissionMap);
	specularMap = move(other.specularMap);
	shininess = move(other.shininess);
}

PhongTexMaterial& PhongTexMaterial::operator=(const PhongTexMaterial& other)
{
	diffuseMap = other.diffuseMap;
	emissionMap = other.emissionMap;
	specularMap = other.specularMap;
	shininess = other.shininess;
	return *this;
}

PhongTexMaterial& PhongTexMaterial::operator=(PhongTexMaterial&& other)
{
	diffuseMap = move(other.diffuseMap);
	emissionMap = move(other.emissionMap);
	specularMap = move(other.specularMap);
	shininess = move(other.shininess);
	return *this;
}

PhongTexMaterial::~PhongTexMaterial()
{
}

const string& PhongTexMaterial::getDiffuseMap() const
{
	return diffuseMap;
}

const string& PhongTexMaterial::getEmissionMap() const
{
	return emissionMap;
}

float PhongTexMaterial::getShininess() const
{
	return shininess;
}

const string& PhongTexMaterial::getSpecularMap() const
{
	return specularMap;
}