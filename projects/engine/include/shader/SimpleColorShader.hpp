#ifndef SIMPLE_COLOR_SHADER_HPP
#define SIMPLE_COLOR_SHADER_HPP
#include <shader/shader.hpp>

class SimpleColorShader : public Shader
{
public:
	SimpleColorShader() {}
	virtual ~SimpleColorShader() {};

	virtual const glm::vec3& getObjectColor() const = 0;

	virtual void setObjectColor(glm::vec3 color) = 0;
};
#endif