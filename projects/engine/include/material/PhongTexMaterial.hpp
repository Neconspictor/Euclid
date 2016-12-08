#ifndef ENGINE_MATERIAL_PHONG_TEX_MATERIAL
#define ENGINE_MATERIAL_PHONG_TEX_MATERIAL
#include <string>

class PhongTexMaterial
{
public:
	PhongTexMaterial(std::string diffuseMap, std::string emissionMap, std::string specularMap, float shininess);
	PhongTexMaterial(const PhongTexMaterial& other);
	PhongTexMaterial(PhongTexMaterial&& other);
	PhongTexMaterial& operator=(const PhongTexMaterial& other);
	PhongTexMaterial& operator=(PhongTexMaterial&& other);

	virtual ~PhongTexMaterial();

	const std::string& getDiffuseMap() const;
	const std::string& getEmissionMap() const;
	float getShininess() const;
	const std::string& getSpecularMap() const;

protected:
	std::string diffuseMap;
	std::string emissionMap;
	std::string specularMap;
	float shininess;
};

#endif