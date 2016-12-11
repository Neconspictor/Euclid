#ifndef SIMPLE_COLOR_SHADER_HPP
#define SIMPLE_COLOR_SHADER_HPP
#include <shader/shader.hpp>

class SimpleColorShader : public Shader
{
public:
	SimpleColorShader() : Shader() {}
	virtual ~SimpleColorShader() {};

	virtual const glm::vec4& getObjectColor() const = 0;

	virtual void setObjectColor(glm::vec4 color) = 0;
};
#endif
