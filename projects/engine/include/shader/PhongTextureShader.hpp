#pragma once
#include <shader/shader.hpp>

class PhongTextureShader : public Shader
{
public:
	PhongTextureShader() {}
	virtual ~PhongTextureShader() {};

	virtual const glm::vec3& getLightColor() const = 0;
	virtual const glm::vec3& getLightPosition() const = 0;

	virtual void setLightColor(glm::vec3 color) = 0;
	virtual void setLightPosition(glm::vec3 position) = 0;

	// NOTE: positions is assumed to be an array with 4 elements!
	virtual void setPointLightPositions(glm::vec3* positions) = 0;
	virtual void setSpotLightDiection(glm::vec3 direction) = 0;
	virtual void setViewPosition(glm::vec3 position) = 0;
};