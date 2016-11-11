#ifndef RENDERER_HPP
#define RENDERER_HPP

class Platform;
#include <ostream>

/**
 * A description for the renderer class which category of renderer it belongs to.
 * Typically the renderer type refers to the underlying 3D rendering library, the 
 * renderer uses for rendering.
 */
enum RendererType : char
{
	OPENGL,
	DIRECTX
};

/**
 * Adds a string representation of the renderer type to an output stream.
 */
inline std::ostream& operator<< (std::ostream & os, RendererType type)
{
	switch (type)
	{
	case OPENGL: return os << "OpenGL";
	case DIRECTX: return os << "DirectX";
		// omit default case to trigger compiler warning for missing cases
	};
	return os << static_cast<std::uint16_t>(type);
}

/**
 * A renderer is responsible for visualizing triangle data onto a screen. Commonly, a renderer uses one of the common 
 * 3D rendering packages like OpenGL and DirectX. The purpose of this abstract class, is to provide library independent
 * access to 3D Rendering. So all library specific tasks are capsulated in a common interface and applications can use 
 * 3D rendering without using specific 3D libraries.
 */
class Renderer
{
public:

	virtual ~Renderer()
	{
	}

	/**
	 * Initializes this renderer. After this function call, the renderer is ready to use.
	 */
	virtual void init() = 0;

	/**
	 * Clears the current scene and begins a new one. A scene is the combination of
	 * all, that should be rendered.
	 * This function should be called before any rendering is done.
	 */
	virtual void beginScene() = 0;
	
	/**
	 * Finishes the current active scene and sends the resulting data to the GPU.
	 */
	virtual void endScene() = 0;
	//virtual void addEntityToScene(Entity* entity) = 0;
	
	/**
	 * Displays the calculdated scene on the screen. This function has to be called after 
	 * virtual void Renderer::endSene().
	 */
	virtual void present() = 0;

	/**
	 * Shuts down this renderer and releases all allocated memory.
	 */
	virtual void release() = 0;

	/**
	 * Provides the type of renderer class, this renderer belongs to.
	 */
	virtual RendererType getType() = 0;

	/**
	 * Sets the base folder where the renderer can access shader files the underlying
	 * rendering API understands.
	 */
	void Renderer::setShaderBaseFolder(std::string baseFolder);

};
#endif