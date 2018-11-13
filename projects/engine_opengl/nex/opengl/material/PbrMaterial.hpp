#pragma once


#include <nex/opengl/material/Material.hpp>
#include <nex/opengl/texture/TextureGL.hpp>

class PbrMaterial : public Material
{
public:
	PbrMaterial();
	PbrMaterial(TextureGL* albedoMap,
		TextureGL* aoMap,
		TextureGL* emissionMap,
		TextureGL* metallicMap,
		TextureGL* normalMap,
		TextureGL* roughnessMap);
	
	PbrMaterial(const PbrMaterial& other);

	PbrMaterial& operator=(const PbrMaterial& other);

	virtual ~PbrMaterial() = default;

	TextureGL* getAlbedoMap() const;
	TextureGL* getAoMap() const;
	TextureGL* getEmissionMap() const;
	TextureGL* getMetallicMap() const;
	TextureGL* getNormalMap() const;
	TextureGL* getRoughnessMap() const;


	void setAlbedoMap(TextureGL* albedoMap);
	void setAoMap(TextureGL* aoMap);
	void setEmissionMap(TextureGL* emissionMap);
	void setMetallicMap(TextureGL* metallicMap);
	void setNormalMap(TextureGL* normalMap);
	void setRoughnessMap(TextureGL* roughnessMap);

protected:
	TextureGL* albedoMap;
	TextureGL* aoMap;
	TextureGL* emissionMap;
	TextureGL* metallicMap;
	TextureGL* normalMap;
	TextureGL* roughnessMap;
};