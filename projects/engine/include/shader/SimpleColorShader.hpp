#pragma once
#include <shader/shader.hpp>

class SimpleColorShader : public Shader
{
public:
	SimpleColorShader() : Shader() {}
	virtual ~SimpleColorShader() {};

	virtual const glm::vec4& getObjectColor() const = 0;

	virtual void setObjectColor(glm::vec4 color) = 0;
};