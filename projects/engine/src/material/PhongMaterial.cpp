#include <material/PhongMaterial.hpp>

using namespace glm;
using namespace std;

PhongMaterial::PhongMaterial(vec3 ambient, vec3 diffuse, vec3 specular, float specularPower) :
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