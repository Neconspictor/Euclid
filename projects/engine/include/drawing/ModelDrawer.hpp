#ifndef ENGINE_DRAWING_MODEL_DRAWER_HPP
#define ENGINE_DRAWING_MODEL_DRAWER_HPP

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
	virtual void drawOutlined(const Model& model, Shader* shader, Shader::TransformData data, glm::vec3 borderColor) = 0;
};

#endif