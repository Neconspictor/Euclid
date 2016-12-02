#ifndef ENGINE_SHADER_PHONG_SHADER_HPP
#define ENGINE_SHADER_PHONG_SHADER_HPP
#include <shader/shader.hpp>

class PhongShader : public Shader
{
public:
	PhongShader() {}
	virtual ~PhongShader() {};

	virtual const glm::vec3& getLightColor() const = 0;
	virtual const glm::vec3& getLightPosition() const = 0;
	virtual const glm::vec3& getObjectColor() const = 0;

	virtual void setLightColor(glm::vec3 color) = 0;
	virtual void setLightPosition(glm::vec3 position) = 0;
	virtual void setObjectColor(glm::vec3 color) = 0;
};
#endif