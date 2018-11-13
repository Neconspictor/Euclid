#pragma once


#include <nex/opengl/material/Material.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class BlinnPhongMaterial : public Material
{
public:
	BlinnPhongMaterial();
	BlinnPhongMaterial(TextureGL* diffuseMap, TextureGL* emissionMap, TextureGL* normalMap, TextureGL* reflectionMap,
		TextureGL* specularMap, float shininess);
	
	BlinnPhongMaterial(const BlinnPhongMaterial& other);
	BlinnPhongMaterial(BlinnPhongMaterial&& other);
	BlinnPhongMaterial& operator=(const BlinnPhongMaterial& other);
	BlinnPhongMaterial& operator=(BlinnPhongMaterial&& other);

	virtual ~BlinnPhongMaterial();

	TextureGL* getDiffuseMap() const;
	TextureGL* getEmissionMap() const;
	TextureGL* getNormalMap() const;
	TextureGL* getReflectionMap() const;
	float getShininess() const;
	const float& getShininessRef() const;

	TextureGL* getSpecularMap() const;

	void setDiffuseMap(TextureGL* diffuse);
	void setEmissionMap(TextureGL* emission);
	void setNormalMap(TextureGL* normal);
	void setReflectionMap(TextureGL* reflection);
	void setSpecularMap(TextureGL* specular);
	void setShininess(float shininess);

protected:
	TextureGL* diffuseMap;
	TextureGL* emissionMap;
	TextureGL* normalMap;
	TextureGL* reflectionMap;
	TextureGL* specularMap;
	float shininess;
};