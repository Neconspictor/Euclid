#pragma once
#include <nex/shader/shader.hpp>

class PointShadowShader : public ShaderConfig
{
public:
	virtual ~PointShadowShader() {};

	virtual void setLightPosition(glm::vec3 pos) = 0;
	virtual void setRange(float range) = 0;
	virtual void setShadowMatrices(glm::mat4 matrices[6]) = 0;
};

class ShadowShader : public ShaderConfig
{
public:
	virtual ~ShadowShader() {};
};

class VarianceShadowShader : public ShaderConfig
{
public:
	virtual ~VarianceShadowShader() {};
};