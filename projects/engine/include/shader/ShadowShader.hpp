#pragma once
#include <shader/shader.hpp>

class PointShadowShader : public Shader
{
public:
	PointShadowShader() : Shader() {}
	virtual ~PointShadowShader() {};

	virtual void setLightPosition(glm::vec3 pos) = 0;
	virtual void setRange(float range) = 0;
	virtual void setShadowMatrices(glm::mat4 matrices[6]) = 0;
};

class ShadowShader : public Shader
{
public:
	ShadowShader() : Shader() {}
	virtual ~ShadowShader() {};
};
