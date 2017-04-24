#pragma once
#include <texture/Texture.hpp>

class Material
{
public:
	Material();
	Material(Texture* diffuseMap, Texture* emissionMap, Texture* normalMap, Texture* reflectionMap,
		Texture* specularMap, float shininess);
	
	Material(const Material& other);
	Material(Material&& other);
	Material& operator=(const Material& other);
	Material& operator=(Material&& other);

	virtual ~Material();

	Texture* getDiffuseMap() const;
	Texture* getEmissionMap() const;
	Texture* getNormalMap() const;
	Texture* getReflectionMap() const;
	float getShininess() const;
	const float& getShininessRef() const;

	Texture* getSpecularMap() const;

	void setDiffuseMap(Texture* diffuse);
	void setEmissionMap(Texture* emission);
	void setNormalMap(Texture* normal);
	void setReflectionMap(Texture* reflection);
	void setSpecularMap(Texture* specular);
	void setShininess(float shininess);

protected:
	Texture* diffuseMap;
	Texture* emissionMap;
	Texture* normalMap;
	Texture* reflectionMap;
	Texture* specularMap;
	float shininess;
};