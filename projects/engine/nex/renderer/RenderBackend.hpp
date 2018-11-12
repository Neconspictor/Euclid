#pragma once

#include <nex/logging/LoggingClient.hpp>
#include <ostream>
#include <nex/texture/Texture.hpp>

class CascadedShadow;
struct Dimension;
class GaussianBlur;
class ModelDrawer;
class ModelManager;
class RenderTarget;
class ShaderManager;
class ShadingModelFactory;
class SMAA;
class SSAO_Deferred;
class TextureManager;

namespace hbao
{
	class HBAO;
}

enum class CullingMode
{
	Front, Back
};

enum RenderComponent {
	Color = 1 << 0,
	Depth = 1 << 1,
	Stencil = 1 << 2
};

/**
* A description for the renderer class which category of renderer it belongs to.
* Typically the renderer type refers to the underlying 3D rendering library, the
* renderer uses for rendering.
*/
enum RendererType : char
{
	INVALID,   // use this to specify that no renderer will be used 
	OPENGL,
	DIRECTX
};

struct Viewport
{
	int x;
	int y;
	int width;
	int height;

	Viewport() : x(0), y(0), width(0), height(0) {}
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
	return os << static_cast<uint16_t>(type);
}

class EffectLibrary {
public:
	virtual GaussianBlur* getGaussianBlur() = 0;
};


/**
* A renderer is responsible for visualizing triangle data onto a screen. Commonly, a renderer uses one of the common
* 3D rendering packages like OpenGL and DirectX. The purpose of this abstract class, is to provide library independent
* access to 3D Rendering. So all library specific tasks are capsulated in a common interface and applications can use
* 3D rendering without using specific 3D libraries.
*/

/**
 * A 3D renderer is a renderer specific for 3D content. 
 * A 3D renderer renders so called models (visual content)
 * and shades them with the use of shaders. A 3D renderer
 * also understands the concept of textures. As shaders 
 * and textures are strong coupled with the implementation
 * of a renderer, the RenderBackend interface provides methods
 * for creating and storing shaders and textures via specialised
 * manager objects.
 */
class RenderBackend
{
public:

	RenderBackend();
	virtual ~RenderBackend() = default;

	/**
	* Clears the current scene and begins a new one. A scene is the combination of
	* all, that should be rendered.
	* This function should be called before any rendering is done.
	*/
	virtual void beginScene() = 0;

	virtual void blitRenderTargets(BaseRenderTarget* src , BaseRenderTarget* dest, const Dimension& dim, int renderComponents) = 0;

	virtual void clearRenderTarget(BaseRenderTarget* renderTarget, int renderComponents) = 0;

	virtual CubeDepthMap* createCubeDepthMap(int width, int height) = 0;

	virtual DepthMap* createDepthMap(int width, int height) = 0;

	//const TextureData& data = {false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32}
	virtual CubeRenderTarget* createCubeRenderTarget(int width, int height, const TextureData& data = { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 }) = 0;

	//{ false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 }
	virtual RenderTarget* create2DRenderTarget(int width, int height, const TextureData& data = { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 }, int samples = 1) = 0;

	virtual RenderTarget* createRenderTarget(int samples = 1) = 0;

	virtual std::unique_ptr<CascadedShadow> createCascadedShadow(unsigned int width, unsigned int height) = 0;

	virtual std::unique_ptr<SSAO_Deferred> createDeferredSSAO() = 0;

	virtual std::unique_ptr<hbao::HBAO> createHBAO() = 0;

	virtual RenderTarget* createVarianceShadowMap(int width, int height) = 0;
	
	virtual void cullFaces(CullingMode mode = CullingMode::Back) = 0;

	virtual void destroyCubeRenderTarget(CubeRenderTarget* target) = 0;

	virtual void destroyRenderTarget(RenderTarget* target) = 0;

	virtual void enableAlphaBlending(bool enable) = 0;

	virtual void enableBackfaceDrawing(bool enable) = 0;
	
	/**
	 * Enables / Disables depth mask writing. 
	 * models drawn with disabled depth mask will always overwrite
	 * the existing fragments/pixels.
	 */
	virtual void enableDepthWriting(bool enable) = 0;

	/**
	* Finishes the current active scene and sends the resulting data to the GPU.
	*/
	virtual void endScene() = 0;

	virtual BaseRenderTarget* getDefaultRenderTarget() = 0;

	virtual EffectLibrary* getEffectLibrary() = 0;

	virtual ShadingModelFactory& getShadingModelFactory() = 0;

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

	virtual SMAA* getSMAA() = 0;

	/**
	* Provides the type of renderer class, this renderer belongs to.
	*/
	virtual RendererType getType() const = 0;

	/**
	* Provides the viewport this renderer is rendering to.
	*/
	Viewport getViewport() const;

	/**
	* Initializes this renderer. After this function call, the renderer is ready to use.
	*/
	virtual void init() = 0;

	/**
	* Displays the calculdated scene on the screen. This function has to be called after
	* virtual void Renderer::endSene().
	*/
	virtual void present() = 0;

	virtual void resize(int width, int height) = 0;

	/**
	* Shuts down this renderer and releases all allocated memory.
	*/
	virtual void release() = 0;

	/**
	 * Renders an equirectangular texture (2D) to a cubemap and returns the result;
	 */
	virtual CubeRenderTarget* renderCubeMap(int width, int height, Texture* equirectangularMap) = 0;

	virtual void setBackgroundColor(glm::vec3 color) = 0;

	/**
	 * Sets the number of samples used for msaa
	 */
	virtual void setMSAASamples(unsigned int samples) = 0;
	
	/**
	* Sets the viewport size and position.
	*/
	virtual void setViewPort(int x, int y, int width, int height) = 0;

	virtual void useCubeDepthMap(CubeDepthMap* cubeDepthMap) = 0;

	virtual void useCubeRenderTarget(CubeRenderTarget* target, CubeMap::Side side, unsigned int mipLevel = 0) = 0;

	/**
	* All draw calls are performed on a offscreen texture.
	* The output of all draw calls won't be visible after swapping the window's screen buffer
	*/
	virtual void useBaseRenderTarget(BaseRenderTarget* target) = 0;

	/**
	 * Draws directly to the screen buffer -> 
	 */
	virtual void useScreenTarget() = 0;

	/**
	* All draw calls are performed on a variance shadow map texture.
	* Only the depth value and it's square are written (no color information).
	*/
	virtual void useVarianceShadowMap(RenderTarget* map) = 0;

protected:
	nex::LoggingClient logClient;
	int width;
	int height;
	int xPos;
	int yPos;
};