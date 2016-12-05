#include <material/PhongTexMaterial.hpp>

using namespace glm;
using namespace std;

PhongTexMaterial::PhongTexMaterial(vec3 ambient, string diffuseMap, string specularMap, float shininess)
{
	this->ambientColor = ambient;
	this->diffuseMap = diffuseMap;
	this->specularMap = specularMap;
	this->shininess = shininess;
}

PhongTexMaterial::PhongTexMaterial(const PhongTexMaterial& other) : ambientColor(other.ambientColor),
diffuseMap(other.diffuseMap), specularMap(other.specularMap), shininess(other.shininess)
{}

PhongTexMaterial::PhongTexMaterial(PhongTexMaterial&& other)
{
	ambientColor = move(other.ambientColor);
	diffuseMap = move(other.diffuseMap);
	specularMap = move(other.specularMap);
	shininess = move(other.shininess);
}

PhongTexMaterial& PhongTexMaterial::operator=(const PhongTexMaterial& other)
{
	ambientColor = other.ambientColor;
	diffuseMap = other.diffuseMap;
	specularMap = other.specularMap;
	shininess = other.shininess;
	return *this;
}

PhongTexMaterial& PhongTexMaterial::operator=(PhongTexMaterial&& other)
{
	ambientColor = move(other.ambientColor);
	diffuseMap = move(other.diffuseMap);
	specularMap = move(other.specularMap);
	shininess = move(other.shininess);
	return *this;
}

PhongTexMaterial::~PhongTexMaterial()
{
}

const vec3& PhongTexMaterial::getAmbient() const
{
	return ambientColor;
}

const string& PhongTexMaterial::getDiffuseMap() const
{
	return diffuseMap;
}

float PhongTexMaterial::getShininess() const
{
	return shininess;
}

const string& PhongTexMaterial::getSpecularMap() const
{
	return specularMap;
}
