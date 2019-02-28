#pragma once
#include "nex/util/StringUtils.hpp"

namespace nex
{
	class Texture;

	enum class MaterialType
	{
		BlinnPhong,
		Pbr,
		None
	};

	class Material
	{
	public:
		Material() {};
		virtual ~Material() {};
	};

	/**
		* Maps material enumerations to a string representation.
		*/
	static const util::EnumString<MaterialType> materialEnumConversion[] = {
		{nex::MaterialType::BlinnPhong, "BLINN_PHONG" },
		{ nex::MaterialType::Pbr, "PBR" },
		{ nex::MaterialType::None, "NONE" }
	};

	std::ostream& operator<<(std::ostream& os, nex::MaterialType type);

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


	class PbrMaterial : public Material
	{
	public:
		PbrMaterial();
		PbrMaterial(Texture* albedoMap,
			Texture* aoMap,
			Texture* emissionMap,
			Texture* metallicMap,
			Texture* normalMap,
			Texture* roughnessMap);

		PbrMaterial(const PbrMaterial& other);

		PbrMaterial& operator=(const PbrMaterial& other);

		virtual ~PbrMaterial() = default;

		Texture* getAlbedoMap() const;
		Texture* getAoMap() const;
		Texture* getEmissionMap() const;
		Texture* getMetallicMap() const;
		Texture* getNormalMap() const;
		Texture* getRoughnessMap() const;


		void setAlbedoMap(Texture* albedoMap);
		void setAoMap(Texture* aoMap);
		void setEmissionMap(Texture* emissionMap);
		void setMetallicMap(Texture* metallicMap);
		void setNormalMap(Texture* normalMap);
		void setRoughnessMap(Texture* roughnessMap);

	protected:
		Texture* albedoMap;
		Texture* aoMap;
		Texture* emissionMap;
		Texture* metallicMap;
		Texture* normalMap;
		Texture* roughnessMap;
	};
}

