#ifndef ENGINE_MATERIAL_PHONG_TEX_MATERIAL
#define ENGINE_MATERIAL_PHONG_TEX_MATERIAL

#include <glm/glm.hpp>
#include <string>

class PhongTexMaterial
{
public:
	PhongTexMaterial(glm::vec3 ambient, std::string diffuseMap, std::string specularMap, float shininess);
	PhongTexMaterial(const PhongTexMaterial& other);
	PhongTexMaterial(PhongTexMaterial&& other);
	PhongTexMaterial& operator=(const PhongTexMaterial& other);
	PhongTexMaterial& operator=(PhongTexMaterial&& other);

	virtual ~PhongTexMaterial();

	const glm::vec3& getAmbient() const;
	const std::string& getDiffuseMap() const;
	float getShininess() const;
	const std::string& getSpecularMap() const;

protected:
	glm::vec3 ambientColor;
	std::string diffuseMap;
	std::string specularMap;
	float shininess;
};

#endif