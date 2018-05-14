#pragma once
#include <shader/shader.hpp>

class CubeMap;

class PBRShader : public ShaderConfig
{
public:
	virtual ~PBRShader() {};

	virtual const glm::vec3& getLightColor() const = 0;
	virtual const glm::vec3& getLightPosition() const = 0;

	virtual void setBrdfLookupTexture(Texture* brdfLUT) = 0;

	virtual void setIrradianceMap(CubeMap* irradianceMap) = 0;


	virtual void setLightColor(glm::vec3 color) = 0;
	virtual void setLightDirection(glm::vec3 direction) = 0;

	virtual void setLightProjMatrix(glm::mat4 mat) = 0;
	virtual void setLightSpaceMatrix(glm::mat4 mat) = 0;
	virtual void setLightViewMatrix(glm::mat4 mat) = 0;

	virtual void setPrefilterMap(CubeMap* prefilterMap) = 0;


	virtual void setSkyBox(CubeMap* sky) = 0;
	virtual void setShadowMap(Texture* texture) = 0;

	virtual void setCameraPosition(glm::vec3 position) = 0;
};

class PBRShader_Deferred : public PBRShader {
public:
	PBRShader_Deferred() : PBRShader() {}
	virtual ~PBRShader_Deferred() {};
};

class PBR_ConvolutionShader : public ShaderConfig
{
public:
	virtual ~PBR_ConvolutionShader() {};

	virtual void setEnvironmentMap(CubeMap* cubeMap) = 0;
};

class PBR_PrefilterShader : public ShaderConfig
{
public:
	virtual ~PBR_PrefilterShader() {};

	virtual void setMapToPrefilter(CubeMap* cubeMap) = 0;

	virtual void setRoughness(float roughness) = 0;
};

class PBR_BrdfPrecomputeShader : public ShaderConfig
{
public:
	virtual ~PBR_BrdfPrecomputeShader() {};
};