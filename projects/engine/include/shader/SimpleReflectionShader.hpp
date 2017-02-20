#pragma once
#include <shader/Shader.hpp>

class SimpleReflectionShader : public Shader
{
public:
	virtual ~SimpleReflectionShader() {}

	virtual void setCameraPosition(glm::vec3 position) = 0;

	virtual void setReflectionTexture(CubeMap* reflectionTex) = 0;
};