#pragma once

#include <glm/glm.hpp>
#include <shader/Shader.hpp>

class Vob;
class Model;

class ModelDrawer
{
public:
	virtual ~ModelDrawer(){}

	/**
	 * Draws the specified model with a given shader onto the screen.
	 */
	virtual void draw( const Model& model, Shader* shader, Shader::TransformData data) = 0;

	/**
	 * Draws the specified model onto the screen and outlines
	 * it with a border.
	 */
	virtual void drawOutlined(const Model& model, Shader* shader, Shader::TransformData data, glm::vec4 borderColor) = 0;

	virtual void drawWired(const Model& model, Shader* shader, Shader::TransformData data, int lineStrength) = 0;
};