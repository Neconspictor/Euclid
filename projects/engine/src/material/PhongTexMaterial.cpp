#include <material/PhongTexMaterial.hpp>

using namespace glm;
using namespace std;

PhongTexMaterial::PhongTexMaterial(vec3 ambient, string diffuseMap, vec3 specular, float specularPower)
{
	this->ambientColor = ambient;
	this->diffuseMap = diffuseMap;
	this->specularColor = specular;
	this->specularPower = specularPower;
}

PhongTexMaterial::PhongTexMaterial(const PhongTexMaterial& other) : ambientColor(other.ambientColor),
diffuseMap(other.diffuseMap), specularColor(other.specularColor), specularPower(other.specularPower)
{}

PhongTexMaterial::PhongTexMaterial(PhongTexMaterial&& other)
{
	ambientColor = other.ambientColor;
	diffuseMap = other.diffuseMap;
	specularColor = other.specularColor;
	specularPower = other.specularPower;
}

PhongTexMaterial& PhongTexMaterial::operator=(const PhongTexMaterial& other)
{
	ambientColor = other.ambientColor;
	diffuseMap = other.diffuseMap;
	specularColor = other.specularColor;
	specularPower = other.specularPower;
	return *this;
}

PhongTexMaterial& PhongTexMaterial::operator=(PhongTexMaterial&& other)
{
	ambientColor = move(other.ambientColor);
	diffuseMap = move(other.diffuseMap);
	specularColor = move(other.specularColor);
	specularPower = move(other.specularPower);
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

const vec3& PhongTexMaterial::getSpecular() const
{
	return specularColor;
}

float PhongTexMaterial::getSpecularPower() const
{
	return specularPower;
}