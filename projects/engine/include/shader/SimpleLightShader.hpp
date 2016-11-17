#ifndef SIMPLE_LIGHT_SHADER_HPP
#define SIMPLE_LIGHT_SHADER_HPP
#include <shader/shader.hpp>
#include <glm/glm.hpp>

class SimpleLightShader : public Shader
{
public:
	SimpleLightShader() {}
	virtual ~SimpleLightShader(){};

	virtual glm::vec3 getLightColor() = 0;
	virtual glm::vec3 getObjectColor() = 0;

	virtual void setLightColor(glm::vec3 color) = 0;
	virtual void setObjectColor(glm::vec3 color) = 0;
};
#endif