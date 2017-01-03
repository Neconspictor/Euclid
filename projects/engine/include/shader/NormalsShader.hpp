#pragma once
#include <shader/shader.hpp>

/**
 * Draws normals of a mesh in a certain color
 */
class NormalsShader : public Shader
{
public:
	NormalsShader() {}
	virtual ~NormalsShader() {};

	virtual const glm::vec4& getNormalColor() const = 0;

	virtual void setNormalColor(glm::vec4 color) = 0;
};