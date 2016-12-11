#ifndef RENDERER_RENDERER3D_HPP
#define RENDERER_RENDERER3D_HPP

#include <texture/TextureManager.hpp>
#include <shader/ShaderManager.hpp>
#include <platform/Renderer.hpp>
#include <model/ModelManager.hpp>
#include <drawing/ModelDrawer.hpp>

/**
 * A 3D renderer is a renderer specific for 3D content. 
 * A 3D renderer renders so called models (visual content)
 * and shades them with the use of shaders. A 3D renderer
 * also understands the concept of textures. As shaders 
 * and textures are strong coupled with the implementation
 * of a renderer, the Renderer3D interface provides methods
 * for creating and storing shaders and textures via specialised
 * manager objects.
 */
class Renderer3D : public Renderer
{
public:

	virtual void enableAlphaBlending(bool enable) = 0;

	/**
	 * Provides access to a shader manager that creates and stores shaders
	 * compatible to this renderer
	 */
	virtual ShaderManager* getShaderManager() = 0;

	/**
	* Provides a texture manager, that creates and stores textures in a format
	* this renderer is able to handle.
	*/
	virtual TextureManager* getTextureManager() = 0;

	/**
	 * Provides a facility class for drawing models.
	 */
	virtual ModelDrawer* getModelDrawer() = 0;

	/** 
	* Provides access to a mesh manager, that creates and stores 3d meshes.
	 */
	virtual ModelManager* getModelManager() = 0;

	virtual void setBackgroundColor(glm::vec3 color) = 0;

	/**
	 * All draw calls are performed on a offscreen texture.
	 * The output of all draw calls won't be visible after swapping the window's screen buffer
	 */
	virtual void useOffscreenBuffer() = 0;

	/**
	 * Draws directly to the screen buffer -> 
	 */
	virtual void useScreenBuffer() = 0;

	/**
	 * Draws the content of the offscreen frame screen buffer to screen
	 * using a ScreenShader 
	 */
	virtual void drawOffscreenBuffer() = 0;
};

#endif