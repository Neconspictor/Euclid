#ifndef ENGINE_MATERIAL_PHONG_MATERIAL
#define ENGINE_MATERIAL_PHONG_MATERIAL

#include <glm/glm.hpp>

class PhongMaterial
{
public:
	PhongMaterial(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float specularPower);
	PhongMaterial(const PhongMaterial& other);
	PhongMaterial(PhongMaterial&& other);
	PhongMaterial& operator=(const PhongMaterial& other);
	PhongMaterial& operator=(PhongMaterial&& other);

	virtual ~PhongMaterial();

protected:
	glm::vec3 ambientColor;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;
	float specularPower; // influences the shininess of the specular color
};

#endif