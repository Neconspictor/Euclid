#include <material/PhongMaterial.hpp>

using namespace glm;
using namespace std;

PhongMaterial::PhongMaterial(vec4 ambient, vec4 diffuse, vec4 specular, float specularPower) :
	ambientColor(ambient), diffuseColor(diffuse), specularColor(specular), specularPower(specularPower)
{}

PhongMaterial::PhongMaterial(const PhongMaterial& o) : ambientColor(o.ambientColor), diffuseColor(o.diffuseColor),
specularColor(o.specularColor), specularPower(o.specularPower)
{}

PhongMaterial::PhongMaterial(PhongMaterial&& o) : ambientColor(o.ambientColor), diffuseColor(o.diffuseColor),
	specularColor(o.ambientColor), specularPower(o.specularPower)
{}

PhongMaterial& PhongMaterial::operator=(const PhongMaterial& other)
{
	if (this == &other) return *this;

	ambientColor = other.ambientColor;
	diffuseColor = other.diffuseColor;
	specularColor = other.specularColor;
	specularPower = other.specularPower;
	return *this;
}

PhongMaterial& PhongMaterial::operator=(PhongMaterial&& other)
{
	if (this == &other) return *this;
	ambientColor = move(other.ambientColor);
	diffuseColor = move(other.diffuseColor);
	specularColor = move(other.specularColor);
	specularPower = move(other.specularPower);
	return *this;
}

PhongMaterial::~PhongMaterial() {}

const vec4& PhongMaterial::getAmbient() const
{
	return ambientColor;
}

const vec4& PhongMaterial::getDiffuse() const
{
	return diffuseColor;
}

const vec4& PhongMaterial::getSpecular() const
{
	return specularColor;
}

float PhongMaterial::getSpecularPower() const
{
	return specularPower;
}