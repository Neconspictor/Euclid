#pragma once
#include <shader/shader.hpp>

class PointShadowShader : public ShaderConfig
{
public:
	PointShadowShader() : ShaderConfig() {}
	virtual ~PointShadowShader() {};

	virtual void setLightPosition(glm::vec3 pos) = 0;
	virtual void setRange(float range) = 0;
	virtual void setShadowMatrices(glm::mat4 matrices[6]) = 0;
};

class ShadowShader : public ShaderConfig
{
public:
	ShadowShader() : ShaderConfig() {}
	virtual ~ShadowShader() {};
};

class VarianceShadowShader : public ShaderConfig
{
public:
	VarianceShadowShader() : ShaderConfig() {}
	virtual ~VarianceShadowShader() {};
};