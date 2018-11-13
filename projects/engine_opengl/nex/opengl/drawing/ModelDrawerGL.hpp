#pragma once

#include <nex/opengl/shader/ShaderGL.hpp>

class RendererOpenGL;
class Sprite;
class Vob;
class ModelGL;

enum class DrawingTypes
{
	SOLID = 0,
	INSTANCED,
	OUTLINED,
	WIRED,
};

class ModelDrawerGL
{
public:
	explicit ModelDrawerGL(RendererOpenGL* renderer);

	virtual ~ModelDrawerGL();

	/**
	 * Draws a sprite onto the screen
	 */
	void draw(Sprite* sprite, Shaders shaderType);
	void draw(Sprite* sprite, ShaderGL& shader);

	/**
	 * Draws the specified model with a given shader onto the screen.
	 */
	void draw(Vob* vob, Shaders shaderType, const TransformData& data);
	void draw(ModelGL* vob, ShaderGL* shader);
	void drawInstanced(Vob* vob, Shaders shaderType, const TransformData& data, unsigned amount);

	/**
	 * Draws the specified model onto the screen and outlines
	 * it with a border.
	 */
	void drawOutlined(Vob* vob, Shaders shaderType, const TransformData& data, glm::vec4 borderColor);
	void drawWired(Vob* vob, Shaders shaderType, const TransformData& data, int lineStrength);

private:
	RendererOpenGL* renderer;
};