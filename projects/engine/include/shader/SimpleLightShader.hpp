#ifndef SIMPLE_LIGHT_SHADER_HPP
#define SIMPLE_LIGHT_SHADER_HPP
#include <shader/shader.hpp>

class SimpleLightShader : public Shader
{
public:
	SimpleLightShader() {}
	virtual ~SimpleLightShader(){};

	virtual const glm::vec3& getLightColor() const = 0;
	virtual const glm::vec4& getObjectColor() const = 0;

	virtual void setLightColor(glm::vec3 color) = 0;
	virtual void setObjectColor(glm::vec4 color) = 0;
};
#endif