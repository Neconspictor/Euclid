#ifndef ENGINE_MATERIAL_PHONG_MATERIAL
#define ENGINE_MATERIAL_PHONG_MATERIAL

#include <glm/glm.hpp>

class PhongMaterial
{
public:
	PhongMaterial(glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular, float specularPower);
	PhongMaterial(const PhongMaterial& other);
	PhongMaterial(PhongMaterial&& other);
	PhongMaterial& operator=(const PhongMaterial& other);
	PhongMaterial& operator=(PhongMaterial&& other);

	virtual ~PhongMaterial();

	const glm::vec4& getAmbient() const;
	const glm::vec4& getDiffuse() const;
	const glm::vec4& getSpecular() const;
	float getSpecularPower() const;


protected:
	glm::vec4 ambientColor;
	glm::vec4 diffuseColor;
	glm::vec4 specularColor;
	float specularPower; // influences the shininess of the specular color
};

#endif