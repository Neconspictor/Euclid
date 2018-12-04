#pragma once


#include <nex/opengl/material/Material.hpp>
#include <nex/texture/Texture.hpp>

namespace nex
{
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