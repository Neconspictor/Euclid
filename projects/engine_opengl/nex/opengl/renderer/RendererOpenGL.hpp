#pragma once
#include <glad/glad.h>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/post_processing/blur/GaussianBlurGL.hpp>
#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include <list>
#include <nex/opengl/post_processing/SSAO_GL.hpp>
#include <nex/opengl/post_processing/HBAO_GL.hpp>
#include <nex/opengl/shader/ShaderManagerGL.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/opengl/shadowing/CascadedShadowGL.hpp>
#include <memory>
#include <nex/common/debug_break.h>
#include "nex/common/Log.hpp"

extern nex::Logger GLOBAL_RENDERER_LOGGER;

#if defined(NDEBUG)
#define SET_BREAK()
#define ASSERT(x)
#define GLCall(x) x;


#else
#define SET_BREAK()	 psnip_trap()
#define ASSERT(x) if (!x) {LOG(GLOBAL_RENDERER_LOGGER, nex::LogLevel::Error) << "Assertion failed!"; SET_BREAK();}

// A macro for validating an OpenGL function call.
#define GLCall(x) GLClearError();\
		x;\
		ASSERT(GLLogCall())
#endif

void GLClearError();
bool GLLogCall();

std::string GLErrorToString(GLuint errorCode);


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


class SMAA_GL;
class RendererOpenGL;

class EffectLibraryGL {
public:

	EffectLibraryGL(RendererOpenGL* renderer);

	// Inherited via EffectLibrary
	 GaussianBlurGL* getGaussianBlur();

	void release();

protected:
	std::unique_ptr<GaussianBlurGL> gaussianBlur;
	RendererOpenGL* renderer;
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
class RendererOpenGL
{
public:
	RendererOpenGL();
	
	virtual ~RendererOpenGL();

	/**
	* Clears the current scene and begins a new one. A scene is the combination of
	* all, that should be rendered.
	* This function should be called before any rendering is done.
	*/
	void beginScene();

	void blitRenderTargets(BaseRenderTargetGL* src, BaseRenderTargetGL* dest, const Dimension& dim, int renderComponents);

	void clearRenderTarget(BaseRenderTargetGL* renderTarget, int renderComponents);

	CubeDepthMapGL* createCubeDepthMap(int width, int height);

	//const TextureData& data = {false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32}
	CubeRenderTargetGL* createCubeRenderTarget(int width, int height, const TextureData& data = { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 });

	RenderTargetGL* create2DRenderTarget(int width, int height, const TextureData& data = { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 }, int samples = 1);

	void clearFrameBuffer(GLuint frameBuffer, glm::vec4 color, float depthValue, int StencilValue);

	std::unique_ptr<CascadedShadowGL> createCascadedShadow(unsigned int width, unsigned int height);

	DepthMapGL* createDepthMap(int width, int height);

	RenderTargetGL* createRenderTarget(int samples = 1);

	RenderTargetGL* createRenderTargetGL(int width, int height, const TextureData& data, GLuint samples,
		GLuint depthStencilType);

	std::unique_ptr<SSAO_DeferredGL> createDeferredSSAO();

	std::unique_ptr<hbao::HBAO_GL> createHBAO();

	RenderTargetGL* createVarianceShadowMap(int width, int height);

	void cullFaces(CullingMode mode);

	void destroyCubeRenderTarget(CubeRenderTargetGL* target);

	void destroyRenderTarget(RenderTargetGL* target);

	void enableAlphaBlending(bool enable);

	void enableBackfaceDrawing(bool enable);

	/**
	 * Enables / Disables depth mask writing.
	 * models drawn with disabled depth mask will always overwrite
	 * the existing fragments/pixels.
	 */
	void enableDepthWriting(bool enable);

	/**
	* Finishes the current active scene and sends the resulting data to the GPU.
	*/
	void endScene();

	GLint getCurrentRenderTarget() const;

	BaseRenderTargetGL* getDefaultRenderTarget();

	// Inherited via RenderBackend
	EffectLibraryGL* getEffectLibrary();

	/**
	 * Provides a facility class for drawing models.
	 */
	ModelDrawerGL* getModelDrawer();

	/**
	* Provides access to a mesh manager, that creates and stores 3d meshes.
	 */
	ModelManagerGL* getModelManager();

	static int getRenderComponentsGL(int renderComponents);


	/**
	 * Provides access to a shader manager that creates and stores shaders
	 * compatible to this renderer
	 */
	ShaderManagerGL* getShaderManager();

	ShadingModelFactoryGL& getShadingModelFactory();

	SMAA_GL* getSMAA();

	/**
	* Provides a texture manager, that creates and stores textures in a format
	* this renderer is able to handle.
	*/
	TextureManagerGL* getTextureManager();

	/**
	* Provides the type of renderer class, this renderer belongs to.
	*/
	RendererType getType() const;

	/**
	* Provides the viewport this renderer is rendering to.
	*/
	const Viewport& getViewport() const;

	/**
	* Initializes this renderer. After this function call, the renderer is ready to use.
	*/
	void init();

	/**
	* Displays the calculdated scene on the screen. This function has to be called after
	* virtual void Renderer::endSene().
	*/
	void present();

	void resize(int width, int height);

	/**
	* Shuts down this renderer and releases all allocated memory.
	*/
	void release();

	/**
	 * Renders an equirectangular texture (2D) to a cubemap and returns the result;
	 */
	CubeRenderTargetGL* renderCubeMap(int width, int height, TextureGL* equirectangularMap);

	void setBackgroundColor(glm::vec3 color);

	/**
	 * Sets the number of samples used for msaa
	 */
	void setMSAASamples(unsigned int samples);

	/**
	* Sets the viewport size and position.
	*/
	void setViewPort(int x, int y, int width, int height);

	void useCubeDepthMap(CubeDepthMapGL* cubeDepthMap);

	void useCubeRenderTarget(CubeRenderTargetGL* target, CubeMapGL::Side side, unsigned int mipLevel = 0);

	/**
	* All draw calls are performed on a offscreen texture.
	* The output of all draw calls won't be visible after swapping the window's screen buffer
	*/
	void useBaseRenderTarget(BaseRenderTargetGL* target);

	/**
	 * Draws directly to the screen buffer ->
	 */
	void useScreenTarget();

	/**
	* All draw calls are performed on a variance shadow map texture.
	* Only the depth value and it's square are written (no color information).
	*/
	void useVarianceShadowMap(RenderTargetGL* map);

	/**
	* A function for checking any opengl related errors.
	* Mainly intended for debug purposes
	* NOTE: Throws an OpenglException if any opengl related error occured
	* since the last call of glGetError()
	* @param errorPrefix: a prefix that will be put in front of the OpenglException.
	*/
	static void checkGLErrors(const std::string& errorPrefix);

	static bool checkGLErrorSilently();

protected:

	static void __clearRenderTarget(RenderTargetGL* screenBuffer, bool releasedAllocatedMemory = true);

protected:
	glm::vec3 backgroundColor;
	std::list<CubeDepthMapGL> cubeDepthMaps;
	std::list<DepthMapGL> depthMaps;
	std::unique_ptr<EffectLibraryGL> effectLibrary;
	std::unique_ptr<ShadingModelFactoryGL> shadingModelFactory;
	ModelDrawerGL modelDrawer;
	unsigned int msaaSamples;
	std::list<CubeRenderTargetGL> cubeRenderTargets;
	std::list<RenderTargetGL> renderTargets;
	std::unique_ptr<SMAA_GL> smaa;
	BaseRenderTargetGL defaultRenderTarget;

protected:
	nex::Logger m_logger;
	Viewport mViewport;
};