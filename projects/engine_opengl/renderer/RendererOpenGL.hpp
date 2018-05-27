#pragma once
#include <renderer/Renderer3D.hpp>
#include <glad/glad.h>
#include <model/ModelGL.hpp>
#include <texture/TextureGL.hpp>
#include <drawing/ModelDrawerGL.hpp>
#include <post_processing/blur/GaussianBlurGL.hpp>
#include <shading_model/ShadingModelFactoryGL.hpp>

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


class RendererOpenGL : public Renderer3D
{
public:
	RendererOpenGL();
	
	virtual ~RendererOpenGL();
	
	void beginScene() override;

	void blitRenderTargets(RenderTarget* src, RenderTarget* dest) override;

	CubeDepthMap* createCubeDepthMap(int width, int height) override;

	virtual CubeRenderTarget* createCubeRenderTarget(int width, int height, const TextureData& data) override;

	RenderTarget* create2DRenderTarget(int width, int height, const TextureData& data, int samples) override;

	void clearFrameBuffer(GLuint frameBuffer, glm::vec4 color, float depthValue, int StencilValue);

	DepthMap* createDepthMap(int width, int height) override;

	RenderTarget* createRenderTarget(int samples) override;

	RenderTargetGL* createRenderTargetGL(int width, int height, const TextureData& data, GLuint samples,
		GLuint depthStencilType);

	RenderTarget* createVarianceShadowMap(int width, int height) override;

	void cullFaces(CullingMode mode) override;

	virtual void destroyCubeRenderTarget(CubeRenderTarget* target) override;

	void destroyRenderTarget(RenderTarget* target) override;

	void enableAlphaBlending(bool enable) override;

	void enableBackfaceDrawing(bool enable) override;
	
	void enableDepthWriting(bool enable) override;

	void endScene() override;

	GLint getCurrentRenderTarget() const;

	// Inherited via Renderer3D
	virtual EffectLibrary* getEffectLibrary() override;

	ModelDrawer* getModelDrawer() override;

	ModelManager* getModelManager() override;

	ShaderManager* getShaderManager() override;

	ShadingModelFactory& getShadingModelFactory() override;

	virtual SMAA* getSMAA() override;
	
	TextureManager* getTextureManager() override;
	
	RendererType getType() const override;
	
	void init() override;

	void present() override;
	
	void release() override;

	CubeRenderTarget* renderCubeMap(int width, int height, Texture* equirectangularMap) override;

	void setBackgroundColor(glm::vec3 color) override;

	void setMSAASamples(unsigned int samples) override;

	void setViewPort(int x, int y, int width, int height) override;

	void useCubeDepthMap(CubeDepthMap* cubeDepthMap) override;

	void useDepthMap(DepthMap* depthMap) override;

	virtual void useCubeRenderTarget(CubeRenderTarget* target, CubeMap::Side side, unsigned int mipLevel) override;

	void useRenderTarget(RenderTarget* target) override;

	void useBaseRenderTarget(BaseRenderTarget* target) override;

	void useScreenTarget() override;

	void useVarianceShadowMap(RenderTarget* map) override;

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
};