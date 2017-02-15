#pragma once
#include <renderer/Renderer3D.hpp>
#include <glad/glad.h>
#include <model/opengl/ModelGL.hpp>
#include <texture/opengl/TextureGL.hpp>
#include <drawing/opengl/ModelDrawerGL.hpp>

class SMAA_GL;


class RendererOpenGL : public Renderer3D
{
public:
	RendererOpenGL();
	
	virtual ~RendererOpenGL();
	
	void beginScene() override;

	void blitRenderTargets(RenderTarget* src, RenderTarget* dest) override;

	void clearFrameBuffer(GLuint frameBuffer, glm::vec4 color, float depthValue, int StencilValue);

	DepthMap* createDepthMap(int width, int height) override;

	RenderTarget* createRenderTarget(int samples) override;

	RenderTargetGL createRenderTarget(GLint textureChannel, int width, int height, GLuint samples = 1,
		GLuint depthStencilType = GL_DEPTH_COMPONENT) const;

	void destroyRenderTarget(RenderTarget* target) override;

	void enableAlphaBlending(bool enable) override;

	void enableBackfaceDrawing(bool enable) override;
	
	void enableDepthWriting(bool enable) override;

	void endScene() override;

	GLint getCurrentRenderTarget() const;

	ModelDrawer* getModelDrawer() override;

	ModelManager* getModelManager() override;

	ShaderManager* getShaderManager() override;

	virtual SMAA* getSMAA() override;
	
	TextureManager* getTextureManager() override;
	
	RendererType getType() const override;
	
	void init() override;

	void present() override;
	
	void release() override;

	void setBackgroundColor(glm::vec3 color) override;

	void setMSAASamples(unsigned int samples) override;

	void setViewPort(int x, int y, int width, int height) override;

	void useDepthMap(DepthMap* depthMap) override;

	void useRenderTarget(RenderTarget* target) override;

	void useScreenTarget() override;


	/**
	* A function for checking any opengl related errors.
	* Mainly intended for debug purposes
	* NOTE: Throws an OpenglException if any opengl related error occured
	* since the last call of glGetError()
	* @param errorPrefix: a prefix that will be put in front of the OpenglException.
	*/
	static void checkGLErrors(std::string errorPrefix);

protected:

	static void clearRenderTarget(RenderTargetGL* screenBuffer, bool releasedAllocatedMemory = true);

	void createFrameRenderTargetBuffer(int width, int height);

	std::unique_ptr<ModelGL> screenSprite;
	std::list<DepthMapGL> depthMaps;
	ModelDrawerGL modelDrawer;
	std::list<RenderTargetGL> renderTargets;
	glm::vec3 backgroundColor;
	unsigned int msaaSamples;

	SMAA_GL* smaa;
};