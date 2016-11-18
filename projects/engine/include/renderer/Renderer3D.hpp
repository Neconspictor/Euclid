#ifndef RENDERER_RENDERER3D_HPP
#define RENDERER_RENDERER3D_HPP

#include <texture/TextureManager.hpp>
#include <shader/ShaderManager.hpp>
#include <platform/Renderer.hpp>

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
};

#endif