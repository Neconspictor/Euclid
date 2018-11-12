#pragma once
#include <nex/renderer/RenderBackend.hpp>
#include <glad/glad.h>
#include <nex/opengl/model/ModelGL.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/drawing/ModelDrawerGL.hpp>
#include <nex/opengl/post_processing/blur/GaussianBlurGL.hpp>
#include <nex/opengl/shading_model/ShadingModelFactoryGL.hpp>
#include <list>

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


class RendererOpenGL
{
public:
	RendererOpenGL();
	
	virtual ~RendererOpenGL();
	
	void beginScene();

	void blitRenderTargets(BaseRenderTarget* src, BaseRenderTarget* dest, const Dimension& dim, int renderComponents);

	void clearRenderTarget(BaseRenderTarget* renderTarget, int renderComponents);

	CubeDepthMap* createCubeDepthMap(int width, int height);

	//const TextureData& data = {false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32}
	CubeRenderTarget* createCubeRenderTarget(int width, int height, const TextureData& data = { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 });

	RenderTarget* create2DRenderTarget(int width, int height, const TextureData& data = { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 }, int samples = 1);

	void clearFrameBuffer(GLuint frameBuffer, glm::vec4 color, float depthValue, int StencilValue);

	std::unique_ptr<CascadedShadow> createCascadedShadow(unsigned int width, unsigned int height);

	DepthMap* createDepthMap(int width, int height);

	RenderTarget* createRenderTarget(int samples);

	RenderTargetGL* createRenderTargetGL(int width, int height, const TextureData& data, GLuint samples,
		GLuint depthStencilType);

	std::unique_ptr<SSAO_Deferred> createDeferredSSAO();

	std::unique_ptr<hbao::HBAO> createHBAO();

	RenderTarget* createVarianceShadowMap(int width, int height);

	void cullFaces(CullingMode mode);

	void destroyCubeRenderTarget(CubeRenderTarget* target);

	void destroyRenderTarget(RenderTarget* target);

	void enableAlphaBlending(bool enable);

	void enableBackfaceDrawing(bool enable);
	
	void enableDepthWriting(bool enable);

	void endScene();

	GLint getCurrentRenderTarget() const;

	BaseRenderTarget* getDefaultRenderTarget();

	// Inherited via RenderBackend
	EffectLibraryGL* getEffectLibrary();

	ModelDrawer* getModelDrawer();

	ModelManager* getModelManager();

	static int getRenderComponentsGL(int renderComponents);

	ShaderManager* getShaderManager();

	ShadingModelFactoryGL& getShadingModelFactory();

	SMAA* getSMAA();
	
	TextureManager* getTextureManager();
	
	RendererType getType() const;

	/**
	* Provides the viewport this renderer is rendering to.
	*/
	const Viewport& getViewport() const;
	
	void init();

	void present();

	void resize(int width, int height);
	
	void release();

	CubeRenderTarget* renderCubeMap(int width, int height, Texture* equirectangularMap);

	void setBackgroundColor(glm::vec3 color);

	void setMSAASamples(unsigned int samples);

	void setViewPort(int x, int y, int width, int height);

	void useCubeDepthMap(CubeDepthMap* cubeDepthMap);

	void useCubeRenderTarget(CubeRenderTarget* target, CubeMap::Side side, unsigned int mipLevel = 0);

	void useBaseRenderTarget(BaseRenderTarget* target);

	void useScreenTarget();

	void useVarianceShadowMap(RenderTarget* map);

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

	static void clearRenderTarget(RenderTargetGL* screenBuffer, bool releasedAllocatedMemory = true);

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
	nex::LoggingClient logClient;
	Viewport mViewport;
};