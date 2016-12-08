#ifndef ENGINE_MATERIAL_MATERIAL_HPP
#define ENGINE_MATERIAL_MATERIAL_HPP
#include <string>

class Material
{
public:
	Material();
	Material(std::string diffuseMap, std::string emissionMap, std::string specularMap, float shininess);
	Material(const Material& other);
	Material(Material&& other);
	Material& operator=(const Material& other);
	Material& operator=(Material&& other);

	virtual ~Material();

	const std::string& getDiffuseMap() const;
	const std::string& getEmissionMap() const;
	float getShininess() const;
	const std::string& getSpecularMap() const;

	void setDiffuseMap(std::string diffuse);
	void setEmissionMap(std::string emission);
	void setSpecularMap(std::string specular);
	void setShininess(float shininess);

protected:
	std::string diffuseMap;
	std::string emissionMap;
	std::string specularMap;
	float shininess;
};

#endif