#pragma once


#include <nex/opengl/material/Material.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
	class BlinnPhongMaterial : public Material
	{
	public:
		BlinnPhongMaterial();
		BlinnPhongMaterial(Texture* diffuseMap, Texture* emissionMap, Texture* normalMap, Texture* reflectionMap,
			Texture* specularMap, float shininess);

		BlinnPhongMaterial(const BlinnPhongMaterial& other);
		BlinnPhongMaterial(BlinnPhongMaterial&& other);
		BlinnPhongMaterial& operator=(const BlinnPhongMaterial& other);
		BlinnPhongMaterial& operator=(BlinnPhongMaterial&& other);

		virtual ~BlinnPhongMaterial();

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
}