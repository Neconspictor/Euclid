#pragma once
#include <renderer/RenderBackend.hpp>
#include <glad/glad.h>
#include <model/ModelGL.hpp>
#include <texture/TextureGL.hpp>
#include <drawing/ModelDrawerGL.hpp>
#include <post_processing/blur/GaussianBlurGL.hpp>
#include <shading_model/ShadingModelFactoryGL.hpp>
#include <list>

class SMAA_GL;
class RendererOpenGL;

class EffectLibraryGL : public EffectLibrary {
public:

	EffectLibraryGL(RendererOpenGL* renderer);

	// Inherited via EffectLibrary
	virtual GaussianBlur* getGaussianBlur() override;

	void release();

protected:
	std::unique_ptr<GaussianBlurGL> gaussianBlur;
	RendererOpenGL* renderer;
};


class RendererOpenGL : public RenderBackend
{
public:
	RendererOpenGL();
	
	virtual ~RendererOpenGL();
	
	virtual void beginScene() override;

	virtual void blitRenderTargets(BaseRenderTarget* src, BaseRenderTarget* dest, const Dimension& dim, int renderComponents) override;

	virtual void clearRenderTarget(BaseRenderTarget* renderTarget, int renderComponents) override;

	virtual CubeDepthMap* createCubeDepthMap(int width, int height) override;

	virtual CubeRenderTarget* createCubeRenderTarget(int width, int height, const TextureData& data) override;

	virtual RenderTarget* create2DRenderTarget(int width, int height, const TextureData& data, int samples) override;

	void clearFrameBuffer(GLuint frameBuffer, glm::vec4 color, float depthValue, int StencilValue);

	virtual DepthMap* createDepthMap(int width, int height) override;

	virtual RenderTarget* createRenderTarget(int samples) override;

	RenderTargetGL* createRenderTargetGL(int width, int height, const TextureData& data, GLuint samples,
		GLuint depthStencilType);

	virtual std::unique_ptr<SSAO_Deferred> createDeferredSSAO() override;

	virtual std::unique_ptr<hbao::HBAO> createHBAO() override;

	virtual RenderTarget* createVarianceShadowMap(int width, int height) override;

	virtual void cullFaces(CullingMode mode) override;

	virtual void destroyCubeRenderTarget(CubeRenderTarget* target) override;

	virtual void destroyRenderTarget(RenderTarget* target) override;

	virtual void enableAlphaBlending(bool enable) override;

	virtual void enableBackfaceDrawing(bool enable) override;
	
	virtual void enableDepthWriting(bool enable) override;

	virtual void endScene() override;

	virtual GLint getCurrentRenderTarget() const;

	virtual BaseRenderTarget* getDefaultRenderTarget() override;

	// Inherited via RenderBackend
	virtual EffectLibrary* getEffectLibrary() override;

	virtual ModelDrawer* getModelDrawer() override;

	virtual ModelManager* getModelManager() override;

	static int getRenderComponentsGL(int renderComponents);

	virtual ShaderManager* getShaderManager() override;

	virtual ShadingModelFactory& getShadingModelFactory() override;

	virtual SMAA* getSMAA() override;
	
	virtual TextureManager* getTextureManager() override;
	
	virtual RendererType getType() const override;
	
	virtual void init() override;

	virtual void present() override;
	
	virtual void release() override;

	virtual CubeRenderTarget* renderCubeMap(int width, int height, Texture* equirectangularMap) override;

	virtual void setBackgroundColor(glm::vec3 color) override;

	virtual void setMSAASamples(unsigned int samples) override;

	virtual void setViewPort(int x, int y, int width, int height) override;

	virtual void useCubeDepthMap(CubeDepthMap* cubeDepthMap) override;

	virtual void useCubeRenderTarget(CubeRenderTarget* target, CubeMap::Side side, unsigned int mipLevel) override;

	virtual void useBaseRenderTarget(BaseRenderTarget* target) override;

	virtual void useScreenTarget() override;

	virtual void useVarianceShadowMap(RenderTarget* map) override;

	/**
	* A function for checking any opengl related errors.
	* Mainly intended for debug purposes
	* NOTE: Throws an OpenglException if any opengl related error occured
	* since the last call of glGetError()
	* @param errorPrefix: a prefix that will be put in front of the OpenglException.
	*/
	static void checkGLErrors(std::string errorPrefix);

	static bool checkGLErrorSilently();

protected:

	static void clearRenderTarget(RenderTargetGL* screenBuffer, bool releasedAllocatedMemory = true);

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
};