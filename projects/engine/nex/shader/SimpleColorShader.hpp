#pragma once
#include <nex/shader/shader.hpp>

class SimpleColorShader : public ShaderConfig
{
public:
	SimpleColorShader() {}
	virtual ~SimpleColorShader() {};

	virtual const glm::vec4& getObjectColor() const = 0;

	virtual void setObjectColor(glm::vec4 color) = 0;
};