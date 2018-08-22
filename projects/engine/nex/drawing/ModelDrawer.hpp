#pragma once

#include <glm/glm.hpp>
#include <nex/shader/Shader.hpp>

class Sprite;
class Vob;
class Model;

enum class DrawingTypes
{
	SOLID = 0, 
	INSTANCED,
	OUTLINED,
	WIRED,
};


class ModelDrawer
{
public:
	virtual ~ModelDrawer(){}

	/**
	 * Draws a sprite onto the screen
	 */
	virtual void draw(Sprite* sprite, Shader& shader) = 0;
	virtual void draw(Sprite* sprite, Shaders shaderType) = 0;

	/**
	 * Draws the specified model with a given shader onto the screen.
	 */
	virtual void draw(Vob* vob, Shaders shaderType, const TransformData& data) = 0;

	virtual void drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data,
		unsigned int amount) = 0;

	/**
	 * Draws the specified model onto the screen and outlines
	 * it with a border.
	 */
	virtual void drawOutlined(Vob* vob, Shaders shaderType, const TransformData& data, glm::vec4 borderColor) = 0;

	virtual void drawWired(Vob* vob, Shaders shaderType, const TransformData& data, int lineStrength) = 0;

};