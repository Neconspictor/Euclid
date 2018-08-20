#pragma once
#include <nex/shader/shader.hpp>

class CubeDepthMapShader : public ShaderConfig
{
public:
	virtual ~CubeDepthMapShader() {};

	virtual void useCubeDepthMap(CubeMap* map) = 0;

	virtual void setLightPos(glm::vec3 pos) = 0;

	virtual void setRange(float range) = 0;
};

class DepthMapShader : public ShaderConfig
{
public:
	virtual ~DepthMapShader() {};

	virtual void useDepthMapTexture(Texture* texture) = 0;
};

class VarianceDepthMapShader : public ShaderConfig
{
public:
	virtual ~VarianceDepthMapShader() {};

	virtual void useVDepthMapTexture(Texture* texture) = 0;
};