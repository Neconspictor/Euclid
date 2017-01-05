#pragma once
#include <renderer/Renderer3D.hpp>
#include <glad/glad.h>
#include <model/opengl/ModelGL.hpp>


class RendererOpenGL : public Renderer3D
{
public:
	RendererOpenGL();
	
	virtual ~RendererOpenGL();
	
	void beginScene() override;

	void drawOffscreenBuffer() override;

	void enableAlphaBlending(bool enable) override;

	void enableBackfaceDrawing(bool enable) override;
	
	void enableDepthWriting(bool enable) override;

	void endScene() override;

	ModelDrawer* getModelDrawer() override;

	ModelManager* getModelManager() override;

	ShaderManager* getShaderManager() override;
	
	TextureManager* getTextureManager() override;
	
	RendererType getType() const override;
	
	void init() override;

	void present() override;
	
	void release() override;

	void setBackgroundColor(glm::vec3 color) override;

	void setMSAASamples(unsigned samples) override;

	void setViewPort(int x, int y, int width, int height) override;

	void useOffscreenBuffer() override;

	void useScreenBuffer() override;

protected:

	struct RenderTargetGL : RenderTarget
	{
		GLuint frameBuffer;
		GLuint textureBuffer;
		GLuint renderBuffer;
	};

	/**
	 * A function for checking any opengl related errors.
	 * Mainly intended for debug purposes
	 * NOTE: Throws an OpenglException if any opengl related error occured
	 * since the last call of glGetError()
	 * @param errorPrefix: a prefix that will be put in front of the OpenglException.
	 */
	void checkGLErrors(std::string errorPrefix) const;

	static void clearRenderTarget(RenderTargetGL* screenBuffer, bool releasedAllocatedMemory = true);

	void createFrameRenderTargetBuffer(int width, int height);
	void createSingleSampledScreenBuffer(RenderTargetGL* screenBuffer) const;
	void createMultiSampledScreenBuffer(RenderTargetGL* screenBuffer, unsigned int samples) const;

	RenderTargetGL createRenderTarget(GLint textureChannel, int width, int height, GLuint samples = 1,
		GLuint depthStencilType = GL_DEPTH_COMPONENT);

	RenderTargetGL singleSampledScreenBuffer;
	RenderTargetGL multiSampledScreenBuffer;

	std::unique_ptr<ModelGL> screenSprite;
	glm::vec3 backgroundColor;
	unsigned int msaaSamples;
};