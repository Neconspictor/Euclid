#pragma once
#include <shader/shader.hpp>
#include <material/PhongMaterial.hpp>

class PhongShader : public Shader
{
public:
	PhongShader() {}
	virtual ~PhongShader() {};

	virtual const glm::vec3& getLightColor() const = 0;
	virtual const glm::vec3& getLightPosition() const = 0;

	virtual void setMaterial(const PhongMaterial& material) = 0;
	virtual void setLightColor(glm::vec3 color) = 0;
	virtual void setLightPosition(glm::vec3 position) = 0;
};