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

	void setViewPort(int x, int y, int width, int height) override;

	void useOffscreenBuffer() override;

	void useScreenBuffer() override;

protected:

	struct ScreenBuffer
	{
		GLuint frameBuffer;
		GLuint texColorBuffer;
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

	void createFrameRenderTargetBuffer(int width, int height);

	ScreenBuffer offscreen;

	ModelGL* screenSprite;
	glm::vec3 backgroundColor;
};